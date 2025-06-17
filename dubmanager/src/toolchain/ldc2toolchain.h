// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LDC2TOOLCHAIN_H
#define LDC2TOOLCHAIN_H

#include "toolchain.h"

/**
 * @todo write docs
 */
class LDC2Toolchain : public Toolchain
{
public:
    /**
     * Default constructor
     */
    LDC2Toolchain();

    /**
     * Destructor
     */
    ~LDC2Toolchain();

    QString name() override;

    /** returns true if this toolchain was found. False otherwise */
    bool probeInstallation() override;

    /** Returns a list of directories of import locations for this toolchain */
    KDevelop::Path::List includes() override;

};

#endif // LDC2TOOLCHAIN_H
