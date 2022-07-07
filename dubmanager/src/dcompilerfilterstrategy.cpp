// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2022 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later


#include <kdevplatform/util/path.h>
#include <kdevplatform/outputview/filtereditem.h>

#include <QHash>
#include <QRegularExpression>

#include "dcompilerfilterstrategy.h"

using namespace KDevelop;


/// Impl. of DCompilerFilterStrategy.
class DCompilerFilterStrategyPrivate
{
public:
    explicit DCompilerFilterStrategyPrivate(const QUrl& buildDir);

    Path m_buildDir;

};


DCompilerFilterStrategy::DCompilerFilterStrategy(const QUrl& buildDir )
    : d_ptr(new DCompilerFilterStrategyPrivate(buildDir))
{
}

DCompilerFilterStrategy::~DCompilerFilterStrategy() = default;

FilteredItem DCompilerFilterStrategy::errorInLine(const QString & line)
{
    Q_D(DCompilerFilterStrategy);

    FilteredItem item(line);
    // GDC
    //const QRegularExpression filter(QStringLiteral("(.+\\.d):(\\d+):(\\d+):\\serror:\\s(.*)"));
    // LDC2
    const QRegularExpression filter(QStringLiteral("(.+\\.d)\\((\\d+),?(\\d+)?\\):\\s(Error|Deprecation|Warning):\\s(.*)"));
    const auto match = filter.match(line);
    if (match.hasMatch()) {
        Path path(d->m_buildDir, match.captured(1));
        item.url = path.toUrl();
        item.lineNo = match.capturedRef(2).toInt()-1;
        if (!match.capturedRef().isNull())
            item.columnNo = match.capturedRef(3).toInt()-1;
        if (match.capturedRef(4) == QStringLiteral("Error"))
            item.type = FilteredItem::ErrorItem;
        else if (match.capturedRef(4) == QStringLiteral("Warning"))
            item.type = FilteredItem::WarningItem;
        else if (match.capturedRef(4) == QStringLiteral("Deprecation"))
            item.type = FilteredItem::InformationItem;
        else
            item.type = FilteredItem::InformationItem;
        item.isActivatable = true;
    }
    return item;
}

FilteredItem DCompilerFilterStrategy::actionInLine(const QString & line)
{
    Q_D(DCompilerFilterStrategy);

    FilteredItem item(line);

    return item;
}


/// Private

DCompilerFilterStrategyPrivate::DCompilerFilterStrategyPrivate(const QUrl& buildDir)
    : m_buildDir(buildDir)
{
}
