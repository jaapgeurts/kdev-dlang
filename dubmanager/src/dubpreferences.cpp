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

    m_ui->lwGlobalAuthors->clear();
    for(const QVariant& author : m_dubSettings->getValues("authors")) {
        m_ui->lwGlobalAuthors->addItem(author.toString());
    }
    // set all items editable
    for(int i =0 ;i < m_ui->lwGlobalAuthors->count(); i++) {
        QListWidgetItem* item = m_ui->lwGlobalAuthors->item(i);
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

    int index = m_ui->cmbBuildTargetType->findText(m_dubSettings->getValue<QString>("targetType"),Qt::MatchFixedString);
    m_ui->cmbBuildTargetType->setCurrentIndex(index);

    // dependencies
    int count = m_dubSettings->numNodes("dependency");
    for (int i=0;i<count;i++ ) {
        QString dep = m_dubSettings->getValue<QString>("dependency", i);
        QString item = dep + " " + m_dubSettings->getAttribute<QString>("dependency", "version", i);
        m_ui->lwBuildDependencies->addItem(item);
    }

    // excluded files
    count = m_dubSettings->numValues("excludedSourceFiles",0);
    for(int i=0; i<count;i++) {
        QString fileName = m_dubSettings->getValue<QString>("excludedSourceFiles", 0, i);
        m_ui->lwBuildExcludedFiles->addItem(fileName);
    }
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
