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
    if (token->type == SDLANG_TOKEN_NODE_END)
    {
        is_node = false;
        is_attribute = false;
        return;
    }

    if (token->type == SDLANG_TOKEN_BLOCK)
    {
        ++depth;
        return;
    }
    else if (token->type == SDLANG_TOKEN_BLOCK_END)
    {
        --depth;
        return;
    }


    const int len = (const int)(token->string.to - token->string.from);
    QString currentText = QString::fromUtf8(token->string.from,len);

    switch (token->type)
    {
    case SDLANG_TOKEN_NODE:
        m_currentNode = currentText;
        is_node = true;
        break;
    case SDLANG_TOKEN_BLOCK:
        ++depth;
        break;
    case SDLANG_TOKEN_BLOCK_END:
        --depth;
        break;
    case SDLANG_TOKEN_ATTRIBUTE:
        //fprintf(stdout, "- attr: %.*s", len, text);
        is_attribute = true;
        break;
    case SDLANG_TOKEN_INT32:
        //fprintf(stdout, "- value(i32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_INT64:
        //fprintf(stdout, "- value(i64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_INT128:
        //fprintf(stdout, "- value(i128): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FLOAT32:
        //fprintf(stdout, "- value(f32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FLOAT64:
        //fprintf(stdout, "- value(f64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_STRING:
        m_dubSettings->setValue(m_currentNode,currentText);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_BASE64:
//         fprintf(stdout, "- value(base64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_UINT32:
//         fprintf(stdout, "- value(hex32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_UINT64:
//         fprintf(stdout, "- value(hex64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_TRUE:
        fprintf(stdout, "- value: true");
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FALSE:
        fprintf(stdout, "- value: false");
        is_attribute = false;
        break;
    case SDLANG_TOKEN_NULL:
        fprintf(stdout, "- value: null");
        is_attribute = false;
        break;

    case SDLANG_TOKEN_NODE_END:
    default:
        break;
    }

    fprintf(stdout, "\n");
}


size_t DubParser::sdlReadNextByte(void* ptr, size_t size)
{
    return m_dubFile.read((char*)ptr,size);
}


QSharedPointer<DubSettings> DubParser::parseProjectFileSdl(const QString& path)
{
    m_fileName = path;
    m_dubFile.setFileName(path);
    m_dubFile.open(QIODevice::ReadOnly);

    if (!m_dubSettings)
        m_dubSettings = QSharedPointer<DubSettings>(new DubSettings);

    const int result = sdlang_parse(sdl_read,this);

    m_dubFile.close();

    return m_dubSettings;

}

void DubParser::parseProjectFileJson(const QString& path)
{
}
