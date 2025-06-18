// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMDTOOLCHAIN_H
#define DMDTOOLCHAIN_H

#include "toolchain.h"

/**
 * @todo write docs
 */
class DMDToolchain : public Toolchain
{
public:
    /**
     * Default constructor
     */
    DMDToolchain();

    /**
     * Destructor
     */
    ~DMDToolchain();

    QString name() override;

    /** returns true if this toolchain was found. False otherwise */
    bool probeInstallation() override;

    /** Returns a list of directories of import locations for this toolchain */
    KDevelop::Path::List includePaths() override;

};

#endif // DMDTOOLCHAIN_H
