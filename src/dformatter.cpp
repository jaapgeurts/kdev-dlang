/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2021  Jaap Geurts <jaapg@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dformatter.h"

DFormatter::DFormatter()
{

}

DFormatter::~DFormatter()
{

}

QString DFormatter::formatSource(const QString& text, const QString& leftContext, const QString& rightContext)
{
    return QStringLiteral("DFormatter::formatSource()");
}

QVariant DFormatter::option(const QString &name) const
{
    return 0;
}

bool DFormatter::predefinedStyle(const QString &name)
{
    return false;
}

void DFormatter::loadStyle(const QString &content)
{
}

QString DFormatter::saveStyle() const
{
    return QStringLiteral("DFormatter::saveStyle()");
}
