// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QIcon>

#include "toolchainmodel.h"

#include "debug.h"

ToolchainModel::ToolchainModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_SelectedToolChain(0)
{
}

ToolchainModel::~ToolchainModel()
{
}

QVariant ToolchainModel::data(const QModelIndex& index, int role) const
{
    // qCDebug(DUB) << "ToolchainModel::data(const QModelIndex& index, int role)";

    if (!index.isValid())
        return QVariant();

    switch(role) {
        case Qt::DisplayRole: {
            return m_Toolchains[index.row()]->name();
        }
        case Qt::DecorationRole: {
            int idx = index.row();
            return idx == m_SelectedToolChain ? QIcon::fromTheme(QStringLiteral("checkmark")) : QIcon();
        }
    }

    return QVariant();
}

int ToolchainModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

int ToolchainModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    // qCDebug(DUB) << "ToolchainModel::rowCount(const QModelIndex& parent)";

    return m_Toolchains.size();
}

QModelIndex ToolchainModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QModelIndex ToolchainModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    // qCDebug(DUB) << "ToolchainModel::index(int row, int column, const QModelIndex& parent)";
    return createIndex(row,column,nullptr);
}

void ToolchainModel::setToolChains(const QList<QSharedPointer<Toolchain>>& toolchains)
{
    // clear all items first.
    beginResetModel();
    m_Toolchains = toolchains;
    endResetModel();
}

int ToolchainModel::selectedToolChain() {
    return m_SelectedToolChain;
}

void ToolchainModel::setSelectedToolChain(int index) {
    beginResetModel();
    m_SelectedToolChain = index;
    endResetModel();
}

