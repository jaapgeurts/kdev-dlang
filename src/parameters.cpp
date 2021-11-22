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

#include <QStringList>

#include "parameters.h"
#include "globalsettings.h"
#include "projectsettings.h"

#include <interfaces/iproject.h>


using namespace KDevelop;

namespace dscannercheck {

DScannerParameters::DScannerParameters(IProject* project) :
    m_project(project)
{
    executablePath = "/home/jaapg/bin/dscanner";
    hideOutputView = GlobalSettings::hideOutputView();

    m_projectRootPath    = m_project->path();
}

DScannerParameters::~DScannerParameters()
{

}

QStringList DScannerParameters::commandLine() const
{
    QStringList result;

    result << executablePath;

    result << "-S"; // do style check. Individual settings are controlled via a settings file

    // set reporting format
    result << "-f";
    result << "##{filepath}:{line}:{column}:{type}:{message}";

    // append all the checks here, depending on the settings.
    // if (boolean this or that)
    // result << "--option=value

    // get a list of all project files when we're scanning the project

    // finally add the path of the file to checks
    result << checkPath;

    return result;
}

Path DScannerParameters::projectRootPath() const
{
    return m_projectRootPath;
}

}
