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

#include "dformatter.h"

#include <interfaces/isourceformatter.h>

#include <QProcess>
#include <QTemporaryFile>

#include "debug.h"

using namespace KDevelop;

DFormatter::DFormatter()
{
}

DFormatter::~DFormatter()
{
}

QString DFormatter::formatSource(const QString& text, const QString& leftContext, const QString& rightContext)
{
    QProcess proc;

    QString program = "/home/jaapg/bin/dfmt";
    QStringList args;

    // make sure settings are saved
    saveSettings();

    // supply a temporary config file
    args << "--config";
    args << "/tmp";

    qCDebug(DFMT) << "Running: " << program << args.join(QLatin1Char(' '));

    proc.start(program,args);
    if (!proc.waitForStarted())
        return "Error";

    QString all = leftContext + text + rightContext;
    proc.write(all.toUtf8());
    proc.closeWriteChannel();

    if (!proc.waitForFinished())
        return "error";

    return QString::fromUtf8(proc.readAll());
}


QVariant DFormatter::option(const QString &key) const
{
    if (!m_options.contains(key))
        qCDebug(DFMT) << "Missing option name " << key;

    return m_options[key];
}

bool DFormatter::predefinedStyle(const QString &name)
{
    if (name == QLatin1String("Default")) {
        resetStyle();
        return true;
    }

    return false;
}

void DFormatter::loadStyle(const QString &content)
{
    m_options = ISourceFormatter::stringToOptionMap(content);
    // TODO: consider saving the settings to the .editorconfig file
    // this also implies that we must read it.
    saveSettings();
}

QString DFormatter::saveStyle() const
{
    return ISourceFormatter::optionMapToString(m_options);
}

void DFormatter::resetStyle()
{
    setIndentStyle(IndentStyle::Spaces);
    setIndentSize(4);
    setTabWidth(4);
    setSingleIndent(false);
    setBraceStyle(BraceStyle::Allman);
    setAlignSwitchStatements(true);
    setOutdentAttributes(true);
    setSplitOperatorAtLineEnd(false);
    setSpaceAfterCast(true);
    setSpaceAfterKeywords(true);
    setSpaceBeforeFunctionParameters(false);
    setSelectiveImportSpace(true);
    setCompactLabeledStatements(true);
    setTemplateContraintStyle(TemplateContraintStyle::ConditionalNewlineIndent);
    setSingleTemplateContraintIndent(false);
    setSpaceBeforeAssocArrayColon(false);
    setKeepLineBreaks(false);
}

// indent_style | `tab`, **`space`** | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_style)
void DFormatter::setIndentStyle(IndentStyle style)
{
    if (style == IndentStyle::Tabs)
        m_options[QStringLiteral("indent_style")] = "tab";
    else if (style == IndentStyle::Spaces)
        m_options[QStringLiteral("indent_style")] = "space";
}

// indent_size | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_size)
void DFormatter::setIndentSize(int length)
{
    m_options[QStringLiteral("indent_size")] = length;
}

// tab_width | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#tab_width)
void DFormatter::setTabWidth(int length)
{
    m_options[QStringLiteral("tab_width")] = length;
}

// dfmt_single_indent | `true`, **`false`** | Set if the code in parens is indented by a single tab instead of two.
void DFormatter::setSingleIndent(bool value)
{
    m_options[QStringLiteral("dfmt_single_indent")] = value;
}

// dfmt_brace_style | **`allman`**, `otbs`, or `stroustrup` | [See Wikipedia](https://en.wikipedia.org/wiki/Brace_style)
void DFormatter::setBraceStyle(BraceStyle braceStyle)
{
    switch(braceStyle) {
        case BraceStyle::Allman:
            m_options[QStringLiteral("dfmt_brace_style")] = "allman";
            break;
        case BraceStyle::Otbs:
            m_options[QStringLiteral("dfmt_brace_style")] = "otbs";
            break;
        case BraceStyle::Stroustrup:
            m_options[QStringLiteral("dfmt_brace_style")] = "stroustrup";
            break;
    }
}

// DON'T USE: dfmt_soft_max_line_length | positive integers (**`80`**) | The formatting process will usually keep lines below this length, but they may be up to *max_line_length* columns long.

// dfmt_align_switch_statements | **`true`**, `false` | Align labels, cases, and defaults with their enclosing switch.
void DFormatter::setAlignSwitchStatements(bool value)
{
    m_options[QStringLiteral("dfmt_align_switch_statements")] = value;
}

// dfmt_outdent_attributes (Not yet implemented) | **`true`**, `false`| Decrease the indentation level of attributes.
void DFormatter::setOutdentAttributes(bool value)
{
    m_options[QStringLiteral("dfmt_outdent_attributes")] = value;
}

// dfmt_split_operator_at_line_end | `true`, **`false`** | Place operators on the end of the previous line when splitting lines.
void DFormatter::setSplitOperatorAtLineEnd(bool value)
{
    m_options[QStringLiteral("dfmt_split_operator_at_line_end")] = value;
}

// dfmt_space_after_cast | **`true`**, `false` | Insert space after the closing paren of a `cast` expression.
void DFormatter::setSpaceAfterCast(bool value)
{
    m_options[QStringLiteral("dfmt_space_after_cast")] = value;
}

// dfmt_space_after_keywords (Not yet implemented) | **`true`**, `false` | Insert space after `if`, `while`, `foreach`, etc, and before the `(`.
void DFormatter::setSpaceAfterKeywords(bool value)
{
    m_options[QStringLiteral("dfmt_space_after_keywords")] = value;
}

// dfmt_space_before_function_parameters | `true`, **`false`** | Insert space before the opening paren of a function parameter list.
void DFormatter::setSpaceBeforeFunctionParameters(bool value)
{
    m_options[QStringLiteral("dfmt_space_before_function_parameters")] = value;
}

// dfmt_selective_import_space | **`true`**, `false` | Insert space after the module name and before the `:` for selective imports.
void DFormatter::setSelectiveImportSpace(bool value)
{
    m_options[QStringLiteral("dfmt_selective_import_space")] = value;
}

// dfmt_compact_labeled_statements | **`true`**, `false` | Place labels on the same line as the labeled `switch`, `for`, `foreach`, or `while` statement.
void DFormatter::setCompactLabeledStatements(bool value)
{
    m_options[QStringLiteral("dfmt_compact_labeled_statements")] = value;
}

// dfmt_template_constraint_style | **`conditional_newline_indent`** `conditional_newline` `always_newline` `always_newline_indent` | Control the formatting of template constraints.
void DFormatter::setTemplateContraintStyle(TemplateContraintStyle style)
{
    switch(style) {
        case TemplateContraintStyle::AlwaysNewline:
            m_options[QStringLiteral("dfmt_template_constraint_style")] = "always_newline";
            break;
        case TemplateContraintStyle::AlwaysNewlineIndent:
            m_options[QStringLiteral("dfmt_template_constraint_style")] = "always_newline_indent";
            break;
        case TemplateContraintStyle::ConditionaleNewline:
            m_options[QStringLiteral("dfmt_template_constraint_style")] = "conditional_newline";
            break;
        case TemplateContraintStyle::ConditionalNewlineIndent:
            m_options[QStringLiteral("dfmt_template_constraint_style")] = "conditional_newline_indent";
            break;
    }
}

// dfmt_single_template_constraint_indent | `true`, **`false`** | Set if the constraints are indented by a single tab instead of two. Has only an effect if the style set to `always_newline_indent` or `conditional_newline_indent`.
void DFormatter::setSingleTemplateContraintIndent(bool value)
{
    m_options[QStringLiteral("dfmt_single_template_constraint_indent")] = value;
}

// dfmt_space_before_aa_colon | `true`, **`false`** | Adds a space after an associative array key before the `:` like in older dfmt versions.
void DFormatter::setSpaceBeforeAssocArrayColon(bool value)
{
    m_options[QStringLiteral("dfmt_space_before_aa_colon")] = value;
}

// dfmt_keep_line_breaks | `true`, **`false`** | Keep existing line breaks if these don't violate other formatting rules.
void DFormatter::setKeepLineBreaks(bool value)
{
    m_options[QStringLiteral("dfmt_keep_line_breaks")] = value;
}

void DFormatter::saveSettings() {

    // TODO: only save this when there are changes
    // save them to the project root
    QFile settings("/tmp/.editorconfig");
    if (settings.open(QIODevice::WriteOnly | QIODevice::Text)) {

        qCDebug(DFMT) << "Saving settings to: " << settings.fileName();

        QTextStream out(&settings);
        out << "[*.d]" << Qt::endl;
        QMapIterator<QString,QVariant> i(m_options);
        while (i.hasNext()) {
            i.next();
            out << i.key() << "=" <<i.value().toString() << Qt::endl;
        }
    }
    // settings goes out of scope and closes the file
}



