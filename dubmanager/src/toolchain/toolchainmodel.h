// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLCHAINMODEL_H
#define TOOLCHAINMODEL_H

#include <qabstractitemmodel.h>

#include "toolchain.h"

/**
 * @todo write docs
 */
class ToolchainModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum ToolchainRole {
        ToolchainData = Qt::UserRole
    };

    /**
     * Default constructor
     */
    ToolchainModel(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~ToolchainModel();

    /**
     * @todo write docs
     *
     * @param index TODO
     * @param role TODO
     * @return TODO
     */
    virtual QVariant data(const QModelIndex& index, int role) const override;

    /**
     * @todo write docs
     *
     * @param parent TODO
     * @return TODO
     */
    virtual int columnCount(const QModelIndex& parent) const override;

    /**
     * @todo write docs
     *
     * @param parent TODO
     * @return TODO
     */
    virtual int rowCount(const QModelIndex& parent) const override;

    /**
     * @todo write docs
     *
     * @param child TODO
     * @return TODO
     */
    virtual QModelIndex parent(const QModelIndex& child) const override;

    /**
     * @todo write docs
     *
     * @param row TODO
     * @param column TODO
     * @param parent TODO
     * @return TODO
     */
    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    void setToolChains(const QList<QSharedPointer<Toolchain>>& toolchains);

    int selectedToolChain();

public Q_SLOTS:
    void setSelectedToolChain(int index);

private:

    QList<QSharedPointer<Toolchain>> m_Toolchains;

    int m_SelectedToolChain;
};

#endif // TOOLCHAINMODEL_H
