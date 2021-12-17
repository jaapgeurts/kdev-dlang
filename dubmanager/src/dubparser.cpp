// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dubparser.h"


static void sdl_emit_token(const struct sdlang_token_t* token, void* user) {
  static_cast<DubParser*>(user)->sdlEmitToken(token);
}

static size_t sdl_read(void* ptr, size_t size, void* user)
{
  return static_cast<DubParser*>(user)->sdlReadNextByte(ptr,size);
}

static void sdl_report_error(enum sdlang_error_t error, int line, void* user)
{
    static_cast<DubParser*>(user)->sdlReportError(error,line);
}


DubParser::DubParser()
{
    sdlang_set_emit_token(sdl_emit_token);
    sdlang_set_report_error(sdl_report_error);
}

DubParser::~DubParser()
{

}



void DubParser::sdlReportError(enum sdlang_error_t error, int line)
{
    switch (error)
    {
    case SDLANG_PARSE_ERROR:
        fprintf(stderr, "parse error at line %d\n", line);
        break;
    case SDLANG_PARSE_ERROR_STACK_OVERFLOW:
        fprintf(stderr, "parse stack overflow at line %d\n", line);
        break;
    case SDLANG_PARSE_ERROR_BUFFER_TOO_SMALL:
        fprintf(stderr, "out of buffer memory at line %d\n", line);
        break;
    default:
        fprintf(stderr, "unknown error [%d] at line %d\n", error, line);
        break;
    }
}

void DubParser::sdlEmitToken(const struct sdlang_token_t* token)
{
//     if (token->type == SDLANG_TOKEN_NODE_END)
//     {
//         is_node = false;
//         return;
//     }
//
//     if (token->type == SDLANG_TOKEN_BLOCK)
//     {
//         ++depth;
//         return;
//     }
//     else if (token->type == SDLANG_TOKEN_BLOCK_END)
//     {
//         --depth;
//         return;
//     }


    const int len = (const int)(token->string.to - token->string.from);
    QString currentText = QString::fromUtf8(token->string.from,len);

    switch (token->type)
    {
    case SDLANG_TOKEN_NODE:
        m_lastNode = m_currentNode->addNode(currentText);
        is_node = true;
        break;
    case SDLANG_TOKEN_BLOCK:
        m_nodeStack.push(m_currentNode);
        m_currentNode = m_currentNode->addNode(currentText);
        m_lastNode = m_currentNode;
        ++depth;
        break;
    case SDLANG_TOKEN_BLOCK_END:
        m_currentNode = m_nodeStack.pop();
        m_lastNode = m_currentNode;
        --depth;
        break;
    case SDLANG_TOKEN_ATTRIBUTE:
        //fprintf(stdout, "- attr: %.*s", len, text);
        m_lastNode->addAtrrib(currentText);
        break;
    case SDLANG_TOKEN_INT32:
        m_lastNode->addValue(QString(currentText).toInt());
        //fprintf(stdout, "- value(i32): %.*s", len, text);
        break;
    case SDLANG_TOKEN_INT64:
        //fprintf(stdout, "- value(i64): %.*s", len, text);
        m_lastNode->addValue(QString(currentText).toLongLong());
        break;
    case SDLANG_TOKEN_INT128:
        m_lastNode->addValue(QString(currentText).toLongLong());
        //fprintf(stdout, "- value(i128): %.*s", len, text);
        break;
    case SDLANG_TOKEN_FLOAT32:
        m_lastNode->addValue(QString(currentText).toFloat());
        //fprintf(stdout, "- value(f32): %.*s", len, text);
        break;
    case SDLANG_TOKEN_FLOAT64:
        m_lastNode->addValue(QString(currentText).toDouble());
        //fprintf(stdout, "- value(f64): %.*s", len, text);
        break;
    case SDLANG_TOKEN_STRING:
        m_lastNode->addValue(currentText);
        break;
    case SDLANG_TOKEN_BASE64:
//         fprintf(stdout, "- value(base64): %.*s", len, text);
        break;
    case SDLANG_TOKEN_UINT32:
        m_lastNode->addValue(QString(currentText).toUInt());
//         fprintf(stdout, "- value(hex32): %.*s", len, text);
        break;
    case SDLANG_TOKEN_UINT64:
        m_lastNode->addValue(QString(currentText).toULongLong());
//         fprintf(stdout, "- value(hex64): %.*s", len, text);
        break;
    case SDLANG_TOKEN_TRUE:
        m_lastNode->addValue(true);
//        fprintf(stdout, "- value: true");
        break;
    case SDLANG_TOKEN_FALSE:
        m_lastNode->addValue(false);
        //        fprintf(stdout, "- value: false");
        break;
    case SDLANG_TOKEN_NULL:
        // insert nothing. means no value or null
//        m_currentNode->addValue(false);
//         fprintf(stdout, "- value: null");
        break;

    case SDLANG_TOKEN_NODE_END:
    default:
        break;
    }

//      fprintf(stdout, "\n");
}


size_t DubParser::sdlReadNextByte(void* ptr, size_t size)
{
    return m_dubFile.read((char*)ptr,size);
}

void dumpTree(const QSharedPointer<SDLNode>& root) {
    printf("%s ",root->name().toLocal8Bit().data());
    for(const QVariant& v : root->values()) {
        printf("%s ",v.toString().toLocal8Bit().data());
    }
    for(const QString& s : root->attribs().keys()) {
        printf("%s ",root->attribs().value(s).toString().toLocal8Bit().data());
    }
    printf("\n");

    for(const QSharedPointer<SDLNode>& node : root->nodes()) {
        dumpTree(node);
    }

}

QSharedPointer<DubSettings> DubParser::parseProjectFileSdl(const QString& path)
{
    m_fileName = path;
    m_dubFile.setFileName(path);
    m_dubFile.open(QIODevice::ReadOnly);

    m_root = QSharedPointer<SDLNode>(new SDLNode);
    m_currentNode = m_root.data();

    const int result = sdlang_parse(sdl_read,this);

    m_dubFile.close();

    // dump it:
    //dumpTree(m_root);

    QSharedPointer<DubSettings> settings = QSharedPointer<DubSettings>(new DubSettings(m_root));
    return settings;

}

void DubParser::parseProjectFileJson(const QString& path)
{
}

SDLNode::SDLNode() :
m_name("<root>"),
m_isAttrib(false)
{
}


SDLNode::SDLNode(const QString& name) :
m_name(name),
m_isAttrib(false)
{
}

void SDLNode::addValue(const QVariant& value)
{
    if (m_isAttrib) {
        m_attribs.insert(m_attribName,value);
        m_isAttrib = false;
    } else {
        // TODO: don't allow two values.
        m_values.append(value);
    }
}

void SDLNode::setValue(const QVariant& value) {
    m_values.clear();
    m_values.append(value);
}


SDLNode * SDLNode::addNode(const QString& name)
{
    SDLNode* node = new SDLNode(name);

    // TODO: JG a node with the same name can appear more than once
    m_nodes.insert(name,QSharedPointer<SDLNode>(node));

    return node;
}

void SDLNode::addAtrrib(const QString& name)
{
    m_isAttrib = true;
    m_attribName = name;
}

const QString& SDLNode::name()
{
    return m_name;
}

const QList<QVariant>& SDLNode::values()
{
    return m_values;
}

void SDLNode::replaceValues(const QList<QVariant>& values) {
    m_values.clear();
    m_values.append(values);
}

const QVariant& SDLNode::valueAt(int i)
{
    return m_values.at(i);
}

const QHash<QString, QVariant> SDLNode::attribs()
{
    return m_attribs;
}

const QHash<QString, QSharedPointer<SDLNode>>& SDLNode::nodes()
{
    return m_nodes;
}

