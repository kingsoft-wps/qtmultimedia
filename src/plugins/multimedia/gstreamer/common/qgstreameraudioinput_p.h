// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGSTREAMERAUDIOINPUT_P_H
#define QGSTREAMERAUDIOINPUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtMultimedia/private/qmultimediautils_p.h>
#include <QtMultimedia/private/qplatformaudioinput_p.h>
#include <QtMultimedia/private/qtmultimediaglobal_p.h>
#include <QtMultimedia/qaudiodevice.h>

#include <qgst_p.h>
#include <qgstpipeline_p.h>

QT_BEGIN_NAMESPACE

class QAudioDevice;

class Q_MULTIMEDIA_EXPORT QGstreamerAudioInput : public QObject, public QPlatformAudioInput
{
public:
    static QMaybe<QPlatformAudioInput *> create(QAudioInput *parent);
    ~QGstreamerAudioInput();

    int volume() const;
    bool isMuted() const;

    bool setAudioInput(const QAudioDevice &);
    QAudioDevice audioInput() const;

    void setAudioDevice(const QAudioDevice &) override;
    void setVolume(float volume) override;
    void setMuted(bool muted) override;

    QGstElement gstElement() const { return gstAudioInput; }

private:
    QGstreamerAudioInput(QGstElement autoaudiosrc, QGstElement volume, QAudioInput *parent);

    float m_volume = 1.;
    bool m_muted = false;

    QAudioDevice m_audioDevice;

    // Gst elements
    QGstBin gstAudioInput;

    QGstElement audioSrc;
    QGstElement audioVolume;
};

QT_END_NAMESPACE

#endif
