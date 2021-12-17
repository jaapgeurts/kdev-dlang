// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dubsettings.h"

#include "dubpreferences.h"
#include "ui_dubpreferences.h"

using namespace KDevelop;

DubPreferences::DubPreferences(IPlugin* plugin, IProject* project , QWidget* parent) :
    ConfigPage(plugin, nullptr, parent),
    m_ui(new Ui::DubPreferences())
{
    m_ui->setupUi(this);

        // check standard locations for project file.
    QString sdlFileName = project->path().toLocalFile() + QStringLiteral("/dub.sdl");
    QString jsonFileName = project->path().toLocalFile() + QStringLiteral("/dub.json");
    if (QFile::exists(sdlFileName)) {
         m_dubSettings = m_Parser.parseProjectFileSdl(sdlFileName);
    }
    else if (QFile::exists(jsonFileName)) {
        //m_Parser.parseProjectFileJson(jsonFileName);
    }

    init();

    updateWidgets();
}

DubPreferences::~DubPreferences()
{
}

void DubPreferences::init()
{

}

void DubPreferences::updateWidgets()
{
    // Global tab
    m_ui->leGlobalName->setText(m_dubSettings->getValue<QString>("name"));
    m_ui->leGlobalDescription->document()->setPlainText(m_dubSettings->getValue<QString>("description"));

    m_ui->cmbGlobalLicense->setCurrentText(m_dubSettings->getValue<QString>("license"));
    m_ui->leGlobalCopyright->setText(m_dubSettings->getValue<QString>("copyright"));
    m_ui->leGlobalHomepage->setText(m_dubSettings->getValue<QString>("homepage"));

    m_ui->lstGlobalAuthors->clear();
    for(const QVariant& author : m_dubSettings->getValues("authors")) {
        m_ui->lstGlobalAuthors->addItem(author.toString());
    }
    // set all items editable
    for(int i =0 ;i < m_ui->lstGlobalAuthors->count(); i++) {
        QListWidgetItem* item = m_ui->lstGlobalAuthors->item(i);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }

    // Build tab
    m_ui->leBuildTargetName->setText(m_dubSettings->getValue<QString>("targetName"));
    m_ui->cmbBuildTargetType->setCurrentText(m_dubSettings->getValue<QString>("targetType"));

    const QList<QVariant>& values = m_dubSettings->getValues("buildOptions");
    if (values.contains("debugMode"))
        m_ui->rbBuildDebug->setChecked(true);
    else if (values.contains("releaseMode"))
        m_ui->rbBuildRelease->setChecked(true);

    // dependencies
    m_dubSettings->getValue<QString>("dependency"); // this can return a list


}



void DubPreferences::apply()
{
}

void DubPreferences::defaults()
{
}

void DubPreferences::reset()
{
}

QString DubPreferences::name() const
{
    return QStringLiteral("DUB Settings");
}

QString DubPreferences::fullName() const
{
    return QStringLiteral("DUB Project Management Settings");
}

// QIcon DubPreferences::icon() const
// {
// }
