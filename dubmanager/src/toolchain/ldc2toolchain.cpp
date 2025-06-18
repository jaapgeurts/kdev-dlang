// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

// QT
#include <QProcess>
#include <QDir>
#include <QTemporaryFile>
#include <QStandardPaths>

// KDEVELOPE
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>


#include "ldc2toolchain.h"

#include "debug.h"

using namespace KDevelop;


LDC2Toolchain::LDC2Toolchain()
{

}

LDC2Toolchain::~LDC2Toolchain()
{

}

QString LDC2Toolchain::name() {
    return QStringLiteral("LDC2");
}


/** returns true if this toolchain was found. False otherwise */
bool LDC2Toolchain::probeInstallation() {

    QStringList compiler = { QStringLiteral("ldc2"),QStringLiteral("-v"), QStringLiteral("-c"), QStringLiteral("empty.d") };

    QString p = QStandardPaths::findExecutable(compiler[0]);
    qCDebug(DUB) << "Probing compiler: " << compiler[0];
    if (!p.isEmpty()) {
        qCDebug(DUB) << "Compiler found: " << p;
        const auto rt = ICore::self()->runtimeController()->currentRuntime();

        QProcess proc;
        proc.setProcessChannelMode( QProcess::MergedChannels );

        proc.setStandardInputFile(QProcess::nullDevice());
        proc.setProgram(p);
        proc.setArguments(compiler.mid(1));
        rt->startProcess(&proc);

        if ( !proc.waitForStarted( 2000 ) || !proc.waitForFinished( 2000 ) ) {
            qCDebug(DUB) <<  "Unable to read standard include paths from " << p;
            return false;
        }

        // we run the compiler with an error condition so that it produced the output
        // if (proc.exitCode() != 0) {
        //     qCWarning(DUB) <<  "error while fetching includes for the compiler:" << p;
        //     return false;
        // }
        // parse the import path.
        QString output = QString::fromUtf8(proc.readAll());
        QString line;
        QTextStream stream(&output);
        while (stream.readLineInto(&line)) {
            qCDebug(DUB) << "OUT: " << line;

        }
        return true;
    }

    return false;

}

/** Returns a list of directories of import locations for this toolchain */
KDevelop::Path::List LDC2Toolchain::includePaths() {
    return Path::List();
}
