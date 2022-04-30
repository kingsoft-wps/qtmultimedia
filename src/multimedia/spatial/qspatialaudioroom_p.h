/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Spatial Audio module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL-NOGPL2$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QSPATIALAUDIOROOM_P_H
#define QSPATIALAUDIOROOM_P_H

#include <qspatialaudioroom.h>
#include <qspatialaudioengine_p.h>
#include <QtGui/qquaternion.h>

#include <resonance_audio_api_extensions.h>
#include "platforms/common/room_effects_utils.h"
#include "platforms/common/room_properties.h"

QT_BEGIN_NAMESPACE

class QSpatialAudioRoomPrivate
{
public:
    static QSpatialAudioRoomPrivate *get(const QSpatialAudioRoom *r) { return r->d; }

    QSpatialAudioEngine *engine = nullptr;
    vraudio::RoomProperties roomProperties;
    bool dirty = true;

    vraudio::ReverbProperties reverb;
    vraudio::ReflectionProperties reflections;

    void update();
};

QT_END_NAMESPACE

#endif
