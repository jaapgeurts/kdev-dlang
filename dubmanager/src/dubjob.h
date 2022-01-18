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

#ifndef DUBJOB_H
#define DUBJOB_H

#include <QString>

#include <outputview/outputexecutejob.h>

#include "debug.h"

class KDevelop::IFilterStrategy;

/**
 * @todo write docs
 */
class DUBJob : public KDevelop::OutputExecuteJob
{
public:
    enum CommandType
    {
        BuildCommand,
        CleanCommand,
        CustomTargetCommand,
        InstallCommand
    };

    enum ErrorTypes
    {
        IncorrectItemError = UserDefinedError,
        ItemNoLongerValidError,
        BuildCommandError,
        FailedError = FailedShownError
    };
    /**
     * Default constructor
     */
    DUBJob( QObject* parent, const QUrl& buildDir ,CommandType command );
    /**
     * Destructor
     */
    ~DUBJob() override;

     // This returns the "make" command line.
    QStringList commandLine() const override;

private:
    CommandType m_Command;
};

#endif // DUBJOB_H
