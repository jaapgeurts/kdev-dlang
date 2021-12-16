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
    m_ui->lblGlobalName->setText(m_dubSettings->name);
    m_ui->lblGlobalDescription->document()->setPlainText(m_dubSettings->description);
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
