/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TOOLCHAINWIDGET_H
#define TOOLCHAINWIDGET_H

#include <QVector>
#include <QWidget>

#include <interfaces/configpage.h>

#include "toolchainmodel.h"

namespace Ui
{
class ToolChainWidget;
}

class QMenu;

class ToolChainWidget : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit ToolChainWidget(QWidget* parent = nullptr);
    ~ToolChainWidget() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    void apply() override;
    void reset() override;
    void defaults() override;


    void setToolChains(const QList<QSharedPointer<Toolchain>>& toolchains);

private Q_SLOTS:
    void deleteCompiler();
    void addCompiler(const QString& factoryName);
    void compilerSelected(const QModelIndex& index);
    void compilerEdited();

Q_SIGNALS:
    void compilerChanged();

private:
    void enableItems(bool enable);

    QScopedPointer<Ui::ToolChainWidget> m_ui;
    QMenu *m_addMenu;
    ToolchainModel* m_ToolChainModel;
};

#endif
