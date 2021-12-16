// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUBPARSER_H
#define DUBPARSER_H

#include <QString>
#include <QFile>
#include <QSharedPointer>

#include "../libsdlang/sdlang.h"
#include "dubsettings.h"

/**
 * @todo write docs
 */
class DubParser
{
public:
    /**
     * Default constructor
     */
    DubParser();

    /**
     * Destructor
     */
    ~DubParser();

    QString setValue(const QString& key, const QString& value);

    void sdlReportError(enum sdlang_error_t error, int line);
    void sdlEmitToken(const struct sdlang_token_t* token);
    size_t sdlReadNextByte(void* ptr, size_t size);

    QSharedPointer<DubSettings> parseProjectFileSdl(const QString& path);
    void parseProjectFileJson(const QString& path);


private:

    QFile m_dubFile;
    QString m_fileName;
    QString m_currentNode;

    QSharedPointer<DubSettings> m_dubSettings;

    // for the sdlang parsing process
    bool is_node = false;
    bool is_attribute = false;
    int depth = 0;

};

#endif // DUBPARSER_H
