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

using namespace KDevelop;

#include "dubjob.h"

DUBJob::DUBJob( QObject* parent, ProjectBaseItem* item,
             CommandType command, const QStringList& overrideTargets ) :
             m_Idx(item->index()),
             m_Command(command)
{

}

    /**
     * Destructor
     */
DUBJob::~DUBJob()
{

}

void DUBJob::start()
{
    ProjectBaseItem* it = item();
    qCDebug(PLUGIN_KDEVDUBBUILDER) << "Building with dub" << m_Command;
    if (!it)
    {
        setError(ItemNoLongerValidError);
        setErrorText(i18n("Build item no longer available"));
        emitResult();
        return;
    }

    if( it->type() == ProjectBaseItem::File ) {
        setError(IncorrectItemError);
        setErrorText(i18n("Internal error: cannot build a file item"));
        emitResult();
        return;
    }

    setStandardToolView(IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    OutputExecuteJob::setWorkingDirectory(it->path().toUrl());

    OutputExecuteJob::start();
}

QStringList DUBJob::commandLine() const
{
    QStringList cmdLine;
    cmdLine << "dub";
    cmdLine << "build";
    return cmdLine;
}

KDevelop::ProjectBaseItem* DUBJob::item() const
{
    return ICore::self()->projectController()->projectModel()->itemFromIndex(m_Idx);
}
