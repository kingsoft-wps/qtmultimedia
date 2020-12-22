/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Ruslan Baratov
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

#ifndef QANDROIDCAPTURESERVICE_H
#define QANDROIDCAPTURESERVICE_H

#include <qmediaservice.h>
#include <qmediacontrol.h>

QT_BEGIN_NAMESPACE

class QAndroidMediaRecorderControl;
class QAndroidCaptureSession;
class QAndroidCameraControl;
class QAndroidVideoDeviceSelectorControl;
class QAndroidAudioInputSelectorControl;
class QAndroidCameraSession;
class QAndroidCameraVideoRendererControl;
class QAndroidCameraExposureControl;
class QAndroidCameraFocusControl;
class QAndroidViewfinderSettingsControl2;
class QAndroidCameraImageProcessingControl;
class QAndroidImageEncoderControl;
class QAndroidCameraImageCaptureControl;
class QAndroidCameraCaptureBufferFormatControl;
class QAndroidAudioEncoderSettingsControl;
class QAndroidVideoEncoderSettingsControl;
class QAndroidMediaContainerControl;

class QAndroidCaptureService : public QMediaService
{
    Q_OBJECT

public:
    explicit QAndroidCaptureService(const QString &service, QObject *parent = 0);
    virtual ~QAndroidCaptureService();

    QMediaControl *requestControl(const char *name);
    void releaseControl(QMediaControl *);

private:
    QString m_service;

    QAndroidMediaRecorderControl *m_recorderControl;
    QAndroidCaptureSession *m_captureSession;
    QAndroidCameraControl *m_cameraControl;
    QAndroidVideoDeviceSelectorControl *m_videoInputControl;
    QAndroidAudioInputSelectorControl *m_audioInputControl;
    QAndroidCameraSession *m_cameraSession;
    QAndroidCameraVideoRendererControl *m_videoRendererControl;
    QAndroidCameraExposureControl *m_cameraExposureControl;
    QAndroidCameraFocusControl *m_cameraFocusControl;
    QAndroidViewfinderSettingsControl2 *m_viewfinderSettingsControl2;
    QAndroidCameraImageProcessingControl *m_cameraImageProcessingControl;
    QAndroidImageEncoderControl *m_imageEncoderControl;
    QAndroidCameraImageCaptureControl *m_imageCaptureControl;
    QAndroidCameraCaptureBufferFormatControl *m_captureBufferFormatControl;
    QAndroidAudioEncoderSettingsControl *m_audioEncoderSettingsControl;
    QAndroidVideoEncoderSettingsControl *m_videoEncoderSettingsControl;
    QAndroidMediaContainerControl *m_mediaContainerControl;
};

QT_END_NAMESPACE

#endif // QANDROIDCAPTURESERVICE_H
