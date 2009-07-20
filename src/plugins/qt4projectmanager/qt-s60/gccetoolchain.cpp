/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
**
**************************************************************************/

#include "gccetoolchain.h"
#include "qt4project.h"

#include <coreplugin/icore.h>

#include <QtCore/QDir>
#include <QtDebug>

using namespace ProjectExplorer;
using namespace Qt4ProjectManager::Internal;

namespace {
    const char *GCCE_COMMAND = "arm-none-symbianelf-gcc.exe";
}

GCCEToolChain::GCCEToolChain(S60Devices::Device device)
    : GccToolChain(QLatin1String(GCCE_COMMAND)),
    m_deviceId(device.id),
    m_deviceName(device.name),
    m_deviceRoot(device.epocRoot)
{

}

ToolChain::ToolChainType GCCEToolChain::type() const
{
    return ToolChain::GCCE;
}

QList<HeaderPath> GCCEToolChain::systemHeaderPaths()
{
    GccToolChain::systemHeaderPaths();
    m_systemHeaderPaths.append(HeaderPath(QString("%1\\epoc32\\include").arg(m_deviceRoot), HeaderPath::GlobalHeaderPath));
    m_systemHeaderPaths.append(HeaderPath(QString("%1\\epoc32\\include\\stdapis").arg(m_deviceRoot), HeaderPath::GlobalHeaderPath));
    m_systemHeaderPaths.append(HeaderPath(QString("%1\\epoc32\\include\\stdapis\\sys").arg(m_deviceRoot), HeaderPath::GlobalHeaderPath));
    m_systemHeaderPaths.append(HeaderPath(QString("%1\\epoc32\\include\\variant").arg(m_deviceRoot), HeaderPath::GlobalHeaderPath));
    return m_systemHeaderPaths;
}

void GCCEToolChain::addToEnvironment(ProjectExplorer::Environment &env)
{
    // TODO: do we need to set path to gcce?
    env.prependOrSetPath(QString("%1\\epoc32\\tools").arg(m_deviceRoot)); // e.g. make.exe
    env.prependOrSetPath(QString("%1\\epoc32\\gcc\\bin").arg(m_deviceRoot)); // e.g. gcc.exe
    env.set("EPOCDEVICE", QString("%1:%2").arg(m_deviceId, m_deviceName));
    env.set("EPOCROOT", S60Devices::cleanedRootPath(m_deviceRoot));
}

QString GCCEToolChain::makeCommand() const
{
    return "make";
}

QString GCCEToolChain::defaultMakeTarget() const
{
    const Qt4Project *qt4project = qobject_cast<const Qt4Project *>(m_project);
    if (qt4project) {
        if (!(QtVersion::QmakeBuildConfig(qt4project->qmakeStep()->value(
                qt4project->activeBuildConfiguration(),
                "buildConfiguration").toInt()) & QtVersion::DebugBuild)) {
            return "release-gcce";
        }
    }
    return "debug-gcce";
}

bool GCCEToolChain::equals(ToolChain *other) const
{
    return (other->type() == type()
            && m_deviceId == static_cast<GCCEToolChain *>(other)->m_deviceId
            && m_deviceName == static_cast<GCCEToolChain *>(other)->m_deviceName);
}

void GCCEToolChain::setProject(const ProjectExplorer::Project *project)
{
    m_project = project;
}
