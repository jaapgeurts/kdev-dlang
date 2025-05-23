/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "toolchainwidget.h"

#include <QAction>
#include <KLocalizedString>
#include <QKeySequence>
#include <QMenu>

#include "ui_toolchainwidget.h"
#include "debug.h"

using namespace KDevelop;

ToolChainWidget::ToolChainWidget(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
    , m_ui(new Ui::ToolChainWidget)
{
    m_ui->setupUi(this);
    m_ui->compilers->header()->setSectionResizeMode(QHeaderView::Stretch);

    m_addMenu = new QMenu(m_ui->addButton);

    m_addMenu->clear();

    connect(m_ui->removeButton, &QPushButton::clicked, this, &ToolChainWidget::deleteCompiler);

    auto delAction = new QAction(i18nc("@action", "Delete Compiler"), this);
    delAction->setShortcut( QKeySequence( QStringLiteral("Del") ) );
    delAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    m_ui->compilers->addAction( delAction );
    connect( delAction, &QAction::triggered, this, &ToolChainWidget::deleteCompiler );

    connect(m_ui->compilers->selectionModel(), &QItemSelectionModel::currentChanged, this, &ToolChainWidget::compilerSelected);

    connect(m_ui->compilerName, &QLineEdit::textEdited, this, &ToolChainWidget::compilerEdited);

    connect(m_ui->compilerPath, &KUrlRequester::textEdited, this, &ToolChainWidget::compilerEdited);


    enableItems(false);
}

ToolChainWidget::~ToolChainWidget()
{
}



void ToolChainWidget::deleteCompiler()
{
    qCDebug(DUB) << "Deleting compiler";

    Q_EMIT changed();
}

void ToolChainWidget::addCompiler(const QString& factoryName)
{
    Q_EMIT changed();
}

void ToolChainWidget::compilerSelected(const QModelIndex& index)
{
}

void ToolChainWidget::compilerEdited()
{
}

void ToolChainWidget::enableItems(bool enable)
{
    m_ui->compilerName->setEnabled(enable);
    m_ui->compilerPath->setEnabled(enable);

    if(!enable) {
        m_ui->compilerName->clear();
        m_ui->compilerPath->clear();
    }
}

void ToolChainWidget::reset()
{
}

void ToolChainWidget::apply()
{
}

void ToolChainWidget::defaults()
{
}

QString ToolChainWidget::name() const
{
    return i18nc("@title:tab", "D ToolChain");
}

QString ToolChainWidget::fullName() const
{
    return i18nc("@title:tab", "Configure D ToolChain");
}

QIcon ToolChainWidget::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dlang-rocket"));
}

KDevelop::ConfigPage::ConfigPageType ToolChainWidget::configPageType() const
{
    return ConfigPage::LanguageConfigPage;
}
