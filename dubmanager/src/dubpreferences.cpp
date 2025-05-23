// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dubsettings.h"

#include "dubpreferences.h"
#include "ui_dubpreferences.h"

#include "debug.h"

using namespace KDevelop;

DubPreferences::DubPreferences(IPlugin* plugin, DubSettings::Ptr settings, QWidget* parent) :
    ConfigPage(plugin, nullptr, parent),
    m_ui(new Ui::DubPreferences()),
    m_dubSettings(settings)
{
    m_ui->setupUi(this);



}

DubPreferences::~DubPreferences()
{
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
