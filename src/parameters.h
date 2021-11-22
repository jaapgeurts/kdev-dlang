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

#ifndef DSCANNERPARAMETERS_H
#define DSCANNERPARAMETERS_H

#include <util/path.h>


namespace KDevelop
{
class IProject;
}

namespace dscannercheck
{

namespace defaults
{

// global settings
static const bool hideOutputView = true;
static const bool checkStyle = true;
}

/**
 * @todo write docs
 */
class DScannerParameters
{
public:
    /**
     * Default constructor
     */
    explicit DScannerParameters(KDevelop::IProject* project = nullptr);

    /**
     * Destructor
     */
    ~DScannerParameters();

    QStringList commandLine() const;

    // TODO: reconsider this. Make these private!!
    // global settings
    QString executablePath;
    bool hideOutputView;

    // runtime settings
    QString checkPath;

    KDevelop::Path projectRootPath() const;

private:

    KDevelop::IProject* m_project;

    KDevelop::Path m_projectRootPath;

};

}
#endif // DSCANNERPARAMETERS_H
