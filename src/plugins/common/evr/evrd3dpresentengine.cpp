/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "evrd3dpresentengine.h"

#include "evrhelpers.h"

#include <qabstractvideobuffer.h>
#include <QAbstractVideoSurface>
#include <qvideoframe.h>
#include <QDebug>
#include <qthread.h>
#include <QOffscreenSurface>


static const int PRESENTER_BUFFER_COUNT = 3;

QT_BEGIN_NAMESPACE


class IMFSampleVideoBuffer: public QAbstractVideoBuffer
{
public:
    IMFSampleVideoBuffer(D3DPresentEngine *engine, IMFSample *sample, QAbstractVideoBuffer::HandleType handleType)
        : QAbstractVideoBuffer(handleType)
        , m_engine(engine)
        , m_sample(sample)
        , m_surface(0)
        , m_mapMode(NotMapped)
    {
        if (m_sample) {
            m_sample->AddRef();

            IMFMediaBuffer *buffer;
            if (SUCCEEDED(m_sample->GetBufferByIndex(0, &buffer))) {
                MFGetService(buffer,
                             mr_BUFFER_SERVICE,
                             iid_IDirect3DSurface9,
                             reinterpret_cast<void **>(&m_surface));
                buffer->Release();
            }
        }
    }

    ~IMFSampleVideoBuffer() override
    {
        if (m_surface) {
            if (m_mapMode != NotMapped)
                m_surface->UnlockRect();
            m_surface->Release();
        }
        if (m_sample)
            m_sample->Release();
    }

    QVariant handle() const override;

    MapMode mapMode() const override { return m_mapMode; }
    uchar *map(MapMode, int*, int*) override;
    void unmap() override;

private:
    mutable D3DPresentEngine *m_engine;
    IMFSample *m_sample;
    IDirect3DSurface9 *m_surface;
    MapMode m_mapMode;
    mutable unsigned int m_textureId = 0;
};

uchar *IMFSampleVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
    if (!m_surface || m_mapMode != NotMapped)
        return 0;

    D3DSURFACE_DESC desc;
    if (FAILED(m_surface->GetDesc(&desc)))
        return 0;

    D3DLOCKED_RECT rect;
    if (FAILED(m_surface->LockRect(&rect, NULL, mode == ReadOnly ? D3DLOCK_READONLY : 0)))
        return 0;

    m_mapMode = mode;

    if (numBytes)
        *numBytes = (int)(rect.Pitch * desc.Height);

    if (bytesPerLine)
        *bytesPerLine = (int)rect.Pitch;

    return reinterpret_cast<uchar *>(rect.pBits);
}

void IMFSampleVideoBuffer::unmap()
{
    if (m_mapMode == NotMapped)
        return;

    m_mapMode = NotMapped;
    m_surface->UnlockRect();
}

QVariant IMFSampleVideoBuffer::handle() const
{
    return m_textureId;
}


D3DPresentEngine::D3DPresentEngine()
    : m_deviceResetToken(0)
    , m_D3D9(0)
    , m_device(0)
    , m_deviceManager(0)
    , m_useTextureRendering(false)
{
    ZeroMemory(&m_displayMode, sizeof(m_displayMode));

    HRESULT hr = initializeD3D();

    if (SUCCEEDED(hr)) {
       hr = createD3DDevice();
       if (FAILED(hr))
           qWarning("Failed to create D3D device");
    } else {
        qWarning("Failed to initialize D3D");
    }
}

D3DPresentEngine::~D3DPresentEngine()
{
    releaseResources();

    qt_evr_safe_release(&m_device);
    qt_evr_safe_release(&m_deviceManager);
    qt_evr_safe_release(&m_D3D9);
}

HRESULT D3DPresentEngine::initializeD3D()
{
    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_D3D9);

    if (SUCCEEDED(hr))
        hr = DXVA2CreateDirect3DDeviceManager9(&m_deviceResetToken, &m_deviceManager);

    return hr;
}

HRESULT D3DPresentEngine::createD3DDevice()
{
    HRESULT hr = S_OK;
    HWND hwnd = NULL;
    UINT uAdapterID = D3DADAPTER_DEFAULT;
    DWORD vp = 0;

    D3DCAPS9 ddCaps;
    ZeroMemory(&ddCaps, sizeof(ddCaps));

    IDirect3DDevice9Ex* device = NULL;

    if (!m_D3D9 || !m_deviceManager)
        return MF_E_NOT_INITIALIZED;

    hwnd = ::GetShellWindow();

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

    pp.BackBufferWidth = 1;
    pp.BackBufferHeight = 1;
    pp.BackBufferFormat = D3DFMT_UNKNOWN;
    pp.BackBufferCount = 1;
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferFormat = D3DFMT_UNKNOWN;
    pp.hDeviceWindow = hwnd;
    pp.Flags = D3DPRESENTFLAG_VIDEO;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    hr = m_D3D9->GetDeviceCaps(uAdapterID, D3DDEVTYPE_HAL, &ddCaps);
    if (FAILED(hr))
        goto done;

    if (ddCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
        vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    hr = m_D3D9->CreateDeviceEx(
                uAdapterID,
                D3DDEVTYPE_HAL,
                pp.hDeviceWindow,
                vp | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
                &pp,
                NULL,
                &device
                );
    if (FAILED(hr))
        goto done;

    hr = m_D3D9->GetAdapterDisplayMode(uAdapterID, &m_displayMode);
    if (FAILED(hr))
        goto done;

    hr = m_deviceManager->ResetDevice(device, m_deviceResetToken);
    if (FAILED(hr))
        goto done;

    qt_evr_safe_release(&m_device);

    m_device = device;
    m_device->AddRef();

done:
    qt_evr_safe_release(&device);
    return hr;
}

bool D3DPresentEngine::isValid() const
{
    return m_device != NULL;
}

void D3DPresentEngine::releaseResources()
{
    m_surfaceFormat = QVideoSurfaceFormat();
}

HRESULT D3DPresentEngine::getService(REFGUID, REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (riid == __uuidof(IDirect3DDeviceManager9)) {
        if (m_deviceManager == NULL) {
            hr = MF_E_UNSUPPORTED_SERVICE;
        } else {
            *ppv = m_deviceManager;
            m_deviceManager->AddRef();
        }
    } else {
        hr = MF_E_UNSUPPORTED_SERVICE;
    }

    return hr;
}

HRESULT D3DPresentEngine::checkFormat(D3DFORMAT format)
{
    if (!m_D3D9 || !m_device)
        return E_FAIL;

    HRESULT hr = S_OK;

    D3DDISPLAYMODE mode;
    D3DDEVICE_CREATION_PARAMETERS params;

    hr = m_device->GetCreationParameters(&params);
    if (FAILED(hr))
        return hr;

    UINT uAdapter = params.AdapterOrdinal;
    D3DDEVTYPE type = params.DeviceType;

    hr = m_D3D9->GetAdapterDisplayMode(uAdapter, &mode);
    if (FAILED(hr))
        return hr;

    hr = m_D3D9->CheckDeviceFormat(uAdapter, type, mode.Format,
                                   D3DUSAGE_RENDERTARGET,
                                   D3DRTYPE_SURFACE,
                                   format);

    if (m_useTextureRendering && format != D3DFMT_X8R8G8B8 && format != D3DFMT_A8R8G8B8) {
        // The texture is always in RGB32 so the d3d driver must support conversion from the
        // requested format to RGB32.
        hr = m_D3D9->CheckDeviceFormatConversion(uAdapter, type, format, D3DFMT_X8R8G8B8);
    }

    return hr;
}

bool D3DPresentEngine::supportsTextureRendering() const
{
    return false;
}

void D3DPresentEngine::setHint(Hint hint, bool enable)
{
    if (hint == RenderToTexture)
        m_useTextureRendering = enable && supportsTextureRendering();
}

HRESULT D3DPresentEngine::createVideoSamples(IMFMediaType *format, QList<IMFSample*> &videoSampleQueue)
{
    if (!format)
        return MF_E_UNEXPECTED;

    HRESULT hr = S_OK;

    IDirect3DSurface9 *surface = NULL;
    IMFSample *videoSample = NULL;

    releaseResources();

    UINT32 width = 0, height = 0;
    hr = MFGetAttributeSize(format, MF_MT_FRAME_SIZE, &width, &height);
    if (FAILED(hr))
        return hr;

    DWORD d3dFormat = 0;
    hr = qt_evr_getFourCC(format, &d3dFormat);
    if (FAILED(hr))
        return hr;

    // Create the video samples.
    for (int i = 0; i < PRESENTER_BUFFER_COUNT; i++) {
        hr = m_device->CreateRenderTarget(width, height,
                                          (D3DFORMAT)d3dFormat,
                                          D3DMULTISAMPLE_NONE,
                                          0,
                                          TRUE,
                                          &surface, NULL);
        if (FAILED(hr))
            goto done;

        hr = MFCreateVideoSampleFromSurface(surface, &videoSample);
        if (FAILED(hr))
            goto done;

        videoSample->AddRef();
        videoSampleQueue.append(videoSample);

        qt_evr_safe_release(&videoSample);
        qt_evr_safe_release(&surface);
    }

done:
    if (SUCCEEDED(hr)) {
        m_surfaceFormat = QVideoSurfaceFormat(QSize(width, height),
                                              m_useTextureRendering ? QVideoFrame::Format_RGB32
                                                                    : qt_evr_pixelFormatFromD3DFormat(d3dFormat),
                                              m_useTextureRendering ? QAbstractVideoBuffer::GLTextureHandle
                                                                    : QAbstractVideoBuffer::NoHandle);
        UINT32 horizontal = 1, vertical = 1;
        hr = MFGetAttributeRatio(format, MF_MT_PIXEL_ASPECT_RATIO, &horizontal, &vertical);
        if (SUCCEEDED(hr))
            m_surfaceFormat.setPixelAspectRatio(horizontal, vertical);
    } else {
        releaseResources();
    }

    qt_evr_safe_release(&videoSample);
    qt_evr_safe_release(&surface);
    return hr;
}

QVideoFrame D3DPresentEngine::makeVideoFrame(IMFSample *sample)
{
    if (!sample)
        return QVideoFrame();

    QVideoFrame frame(new IMFSampleVideoBuffer(this, sample, m_surfaceFormat.handleType()),
                      m_surfaceFormat.frameSize(),
                      m_surfaceFormat.pixelFormat());

    // WMF uses 100-nanosecond units, Qt uses microseconds
    LONGLONG startTime = 0;
    auto hr = sample->GetSampleTime(&startTime);
     if (SUCCEEDED(hr)) {
        frame.setStartTime(startTime * 0.1);

        LONGLONG duration = -1;
        if (SUCCEEDED(sample->GetSampleDuration(&duration)))
            frame.setEndTime((startTime + duration) * 0.1);
    }

    return frame;
}

QT_END_NAMESPACE
