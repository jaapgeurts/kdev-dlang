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

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <KLocalizedString>
#include <util/path.h>

#include "dcompilerfilterstrategy.h"

using namespace KDevelop;

#include "dubjob.h"

DUBJob::DUBJob( QObject* parent, const QUrl& buildDir,
             CommandType command) :
             OutputExecuteJob(parent),
             m_Command(command)
{
    setCapabilities(Killable);
    setStandardToolView(IOutputView::BuildView);
    setFilteringStrategy(new DCompilerFilterStrategy(buildDir));
    setWorkingDirectory(buildDir);
    setJobName(QStringLiteral("DUB Job"));
    setToolTitle(i18nc("@title:window", "DUB"));
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    // enable to process errors
    setProperties(
        OutputExecuteJob::NeedWorkingDirectory |
        OutputExecuteJob::DisplayStderr |
        OutputExecuteJob::DisplayStdout |
        OutputExecuteJob::IsBuilderHint );

}

/**
    * Destructor
    */
DUBJob::~DUBJob()
{
}

QStringList DUBJob::commandLine() const
{
    // TODO: JG add all build options
    return { "dub","build" };
}

