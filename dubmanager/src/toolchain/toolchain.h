// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H

#include <QString>

#include <util/path.h>

/**
 * @todo Abstract super class for compiler implementations.
 * This class probes the install location of a D compiler toolchain
 */
class Toolchain
{
public:

    virtual ~Toolchain() = default;

    virtual QString name() = 0;
    /** returns true if this toolchain was found. False otherwise */
    virtual bool probeInstallation() = 0;

    /** Returns a list of directories of import locations for this toolchain */
    virtual KDevelop::Path::List includes() = 0;


};

#endif // TOOLCHAIN_H
