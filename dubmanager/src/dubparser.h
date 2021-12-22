// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUBPARSER_H
#define DUBPARSER_H

#include <QString>
#include <QFile>
#include <QSharedPointer>
#include <QList>
#include <QList>
#include <QVariant>
#include <QStack>

#include "../libsdlang/sdlang.h"
#include "dubsettings.h"

class SDLNode {

public:
    SDLNode();
    SDLNode(const QString& name);

    /** Adds a value to this member */
    void addValue(const QVariant& value);
    /** Will delete all values and replace with this one */
    void setValue(const QVariant& value);

    /** Replace all values wih the new ones */
    void replaceValues(const QList<QVariant>& values);

    void addAtrrib(const QString& name);
    SDLNode* addNode(const QString& name);

    const QString& name();
    const QList<QVariant>& values();
    const QVariant& valueAt(int i);
    const QHash<QString, QVariant> attribs();
    const QList<QSharedPointer<SDLNode>>& nodes();


private:
    QString m_name;
    QList<QVariant> m_values;
    QHash<QString,QVariant> m_attribs;
    QList<QSharedPointer<SDLNode>> m_nodes;

    bool m_isAttrib;
    QString m_attribName;

};

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

    QSharedPointer<SDLNode> m_root;
    SDLNode* m_currentNode;
    SDLNode* m_lastNode;

    QStack<SDLNode*> m_nodeStack;

    // for the sdlang parsing process
    bool is_node = false;
    int depth = 0;

};

#endif // DUBPARSER_H
