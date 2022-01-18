// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2022 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DCOMPILERFILTERSTRATEGY_H
#define DCOMPILERFILTERSTRATEGY_H

#include <kdevplatform/outputview/ifilterstrategy.h>
#include <QUrl>
#include <QScopedPointer>

class DCompilerFilterStrategyPrivate;

/**
 * @todo write docs
 */
class DCompilerFilterStrategy : public KDevelop::IFilterStrategy
{
public:
    /**
     * Default constructor
     */
    explicit DCompilerFilterStrategy(const QUrl& buildDir);

    /**
     * Destructor
     */
    ~DCompilerFilterStrategy() override;

    KDevelop::FilteredItem errorInLine(const QString & line) override;
    KDevelop::FilteredItem actionInLine(const QString & line) override;

private:
    const QScopedPointer<class DCompilerFilterStrategyPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DCompilerFilterStrategy)
};

#endif // DCOMPILERFILTERSTRATEGY_H
