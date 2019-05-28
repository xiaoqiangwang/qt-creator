/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "librarywizard.h"
#include "librarywizarddialog.h"

#include <cpptools/abstracteditorsupport.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/qtsupportconstants.h>

#include <utils/fileutils.h>

#include <QTextStream>
#include <QCoreApplication>

static const char sharedHeaderPostfixC[] = "_global";

namespace QmakeProjectManager {
namespace Internal {

LibraryWizard::LibraryWizard()
{
    setId("H.Qt4Library");
    setCategory(QLatin1String(ProjectExplorer::Constants::LIBRARIES_WIZARD_CATEGORY));
    setDisplayCategory(QCoreApplication::translate("ProjectExplorer",
        ProjectExplorer::Constants::LIBRARIES_WIZARD_CATEGORY_DISPLAY));
    setDisplayName(tr("C++ Library"));
    setDescription(tr("Creates a C++ library based on qmake. This can be used to create:<ul>"
                "<li>a shared C++ library for use with <tt>QPluginLoader</tt> and runtime (Plugins)</li>"
                "<li>a shared or static C++ library for use with another project at linktime</li></ul>"));
    setIcon(QIcon(QLatin1String(":/wizards/images/lib.png")));
    setRequiredFeatures({QtSupport::Constants::FEATURE_QT_PREFIX});
}

Core::BaseFileWizard *LibraryWizard::create(QWidget *parent, const Core::WizardDialogParameters &parameters) const
{
    LibraryWizardDialog *dialog = new LibraryWizardDialog(this, displayName(), icon(), parent, parameters);
    dialog->setLowerCaseFiles(QtWizard::lowerCaseFiles());
    dialog->setProjectName(LibraryWizardDialog::uniqueProjectName(parameters.defaultPath()));
    dialog->setSuffixes(headerSuffix(), sourceSuffix(), formSuffix());
    return dialog;
}

static void writeLinuxProFile(QTextStream &str, const QtProjectParameters &params)
{
    str << "\n"
           "unix {\n";
    if (!params.targetDirectory.isEmpty())
        str << "    target.path = " << params.targetDirectory << '\n';
    else
        str << "    target.path = /usr/lib\n";
    str << "    INSTALLS += target\n"
           "}\n";
}

Core::GeneratedFiles LibraryWizard::generateFiles(const QWizard *w,
                                                 QString *errorMessage) const
{
    Q_UNUSED(errorMessage)
    const auto *dialog = qobject_cast<const LibraryWizardDialog *>(w);
    const QtProjectParameters projectParams = dialog->parameters();
    const QString projectPath = projectParams.projectPath();
    const LibraryParameters params = dialog->libraryParameters();
    const bool usePragmaOnce = CppTools::AbstractEditorSupport::usePragmaOnce();

    const QString sharedLibExportMacro = QtProjectParameters::exportMacro(projectParams.fileName);

    Core::GeneratedFiles rc;
    // Class header + source
    const QString sourceFileName = buildFileName(projectPath, params.sourceFileName, sourceSuffix());
    Core::GeneratedFile source(sourceFileName);
    source.setAttributes(Core::GeneratedFile::OpenEditorAttribute);

    const QString headerFileFullName = buildFileName(projectPath, params.headerFileName, headerSuffix());
    const QString headerFileName = Utils::FileName::fromString(headerFileFullName).fileName();
    QString pluginJsonFileFullName;
    QString pluginJsonFileName;
    if (projectParams.type == QtProjectParameters::QtPlugin) {
        pluginJsonFileFullName = buildFileName(projectPath, projectParams.fileName, QLatin1String("json"));
        pluginJsonFileName = Utils::FileName::fromString(pluginJsonFileFullName).fileName();
    }

    Core::GeneratedFile header(headerFileFullName);

    // Create files: global header for shared libs
    QString globalHeaderFileName;
    if (projectParams.type == QtProjectParameters::SharedLibrary) {
        const QString globalHeaderName = buildFileName(projectPath, projectParams.fileName.toLower() + QLatin1String(sharedHeaderPostfixC), headerSuffix());
        Core::GeneratedFile globalHeader(globalHeaderName);
        globalHeaderFileName = Utils::FileName::fromString(globalHeader.path()).fileName();
        globalHeader.setContents(CppTools::AbstractEditorSupport::licenseTemplate(globalHeaderFileName)
                                 + LibraryParameters::generateSharedHeader(globalHeaderFileName, projectParams.fileName, sharedLibExportMacro, usePragmaOnce));
        rc.push_back(globalHeader);
    }

    // Generate code
    QString headerContents, sourceContents;
    params.generateCode(projectParams.type, headerFileName,
                        globalHeaderFileName, sharedLibExportMacro, pluginJsonFileName,
                        /* indentation*/ 4, usePragmaOnce, &headerContents, &sourceContents);

    source.setContents(CppTools::AbstractEditorSupport::licenseTemplate(sourceFileName, params.className)
                       + sourceContents);
    header.setContents(CppTools::AbstractEditorSupport::licenseTemplate(headerFileFullName, params.className)
                       + headerContents);
    rc.push_back(source);
    rc.push_back(header);
    // Create files: profile
    const QString profileName = buildFileName(projectPath, projectParams.fileName, profileSuffix());
    Core::GeneratedFile profile(profileName);
    profile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);
    QString profileContents;
    {
        QTextStream proStr(&profileContents);
        QtProjectParameters::writeProFileHeader(proStr);
        projectParams.writeProFile(proStr);
        proStr << "\nSOURCES +="
               << " \\\n        " << Utils::FileName::fromString(source.path()).fileName()
               << "\n\nHEADERS +="
               << " \\\n        " << headerFileName;
        if (!globalHeaderFileName.isEmpty())
            proStr << " \\\n        " << globalHeaderFileName << " \n";
        if (!pluginJsonFileName.isEmpty())
            proStr << "\nDISTFILES += " << pluginJsonFileName << " \n";
        writeLinuxProFile(proStr, projectParams);
    }
    profile.setContents(profileContents);
    rc.push_back(profile);

    if (!pluginJsonFileName.isEmpty()) {
        Core::GeneratedFile jsonFile(pluginJsonFileFullName);
        jsonFile.setContents(QLatin1String("{\n    \"Keys\" : [ ]\n}\n"));
        rc.push_back(jsonFile);
    }
    return rc;
}

} // namespace Internal
} // namespace QmakeProjectManager