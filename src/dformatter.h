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

#ifndef DFORMATTER_H
#define DFORMATTER_H

#include <QVariant>
#include <QString>

/**
 * @todo write docs
 */
class DFormatter
{
public:
    /**
     * Default constructor
     */
    DFormatter();

    /**
     * Destructor
     */
    ~DFormatter();

    QString formatSource(const QString& text, const QString& leftContext = QString(), const QString& rightContext = QString());

    QVariant option(const QString &name) const;

    bool predefinedStyle(const QString &name);
    void loadStyle(const QString &content);
    QString saveStyle() const;

    // indent
    /*
    void setTabIndentation(int length, bool forceTabs);
    void setSpaceIndentation(int length);
    void setTabSpaceConversionMode(bool mode);
    void setFillEmptyLines(bool on);
    void setBlockIndent(bool on);
    void setBracketIndent(bool on);
    void setCaseIndent(bool on);
    void setClassIndent(bool on);
    void setLabelIndent(bool on);
    void setNamespaceIndent(bool on);
    void setPreprocessorIndent(bool on);
    void setSwitchIndent(bool on);
    void setMaxInStatementIndentLength(int max);
    void setMinConditionalIndentLength(int min);
    void setAfterParens(bool on);
    void setContinuation(int n);
    */
    //brackets
//    void setBracketFormatMode(astyle::BraceMode mode);
//     void setBreakClosingHeaderBracketsMode(bool state);
    //blocks
//     void setBreakBlocksMode(bool state);
//     void setBreakElseIfsMode(bool state);
//     void setBreakClosingHeaderBlocksMode(bool state);
    //padding
//     void setOperatorPaddingMode(bool mode);
//     void setParensOutsidePaddingMode(bool mode);
//     void setParensInsidePaddingMode(bool mode);
//     void setParensHeaderPaddingMode(bool mode);
//     void setParensUnPaddingMode(bool state);
    //oneliners
//     void setBreakOneLineBlocksMode(bool state);
//     void setBreakOneLineStatementsMode(bool state);
    //     pointer
//     void setPointerAlignment(astyle::PointerAlign alignment);

private:
    void updateFormatter();
    void resetStyle();

    QVariantMap m_options;

};

#endif // DFORMATTER_H
