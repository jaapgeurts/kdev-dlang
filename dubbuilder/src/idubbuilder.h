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

#ifndef IDUBBUILDER_H
#define IDUBBUILDER_H

#include <project/interfaces/iprojectbuilder.h>

/**
 * @todo write docs
 */
class IDUBBuilder : public KDevelop::IProjectBuilder
{
public:
    /**
     * @todo write docs
     */
    ~IDUBBuilder() override = default;

};

Q_DECLARE_INTERFACE( IDUBBuilder, "org.kdevelop.IDUBBuilder" )

#endif // IDUBBUILDER_H