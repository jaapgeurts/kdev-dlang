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
#include "formatjob.h"

#include <QProcess>

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

QVariant DFormatter::option(const QString &name) const
{
    return 0;
}

bool DFormatter::predefinedStyle(const QString &name)
{
    return false;
}

void DFormatter::loadStyle(const QString &content)
{
}

QString DFormatter::saveStyle() const
{
    return QStringLiteral("DFormatter::saveStyle()");
}

// indent_style | `tab`, **`space`** | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_style)
void DFormatter::setIndentStyle(bool mode);

// indent_size | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_size)
void DFormatter::setIndentSize(int length)
{
}

// tab_width | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#tab_width)
void DFormatter::setTabWidth(int length)
{
}

// dfmt_single_indent | `true`, **`false`** | Set if the code in parens is indented by a single tab instead of two.
void DFormatter::setSingleIndent(bool vaue)
{
}

// dfmt_brace_style | **`allman`**, `otbs`, or `stroustrup` | [See Wikipedia](https://en.wikipedia.org/wiki/Brace_style)
void DFormatter::setBraceStyle(BraceStyle braceStyle)
{
}

// DON'T USE: dfmt_soft_max_line_length | positive integers (**`80`**) | The formatting process will usually keep lines below this length, but they may be up to *max_line_length* columns long.

// dfmt_align_switch_statements | **`true`**, `false` | Align labels, cases, and defaults with their enclosing switch.
void DFormatter::setAlignSwitchStatements(bool value)
{
}

// dfmt_outdent_attributes (Not yet implemented) | **`true`**, `false`| Decrease the indentation level of attributes.
void DFormatter::setOutdentAttributes(bool value)
{
}

// dfmt_split_operator_at_line_end | `true`, **`false`** | Place operators on the end of the previous line when splitting lines.
void DFormatter::setSplitOperatorAtLineEnd(bool value)
{
}

// dfmt_space_after_cast | **`true`**, `false` | Insert space after the closing paren of a `cast` expression.
void DFormatter::setSpaceAfterCast(bool value)
{
}

// dfmt_space_after_keywords (Not yet implemented) | **`true`**, `false` | Insert space after `if`, `while`, `foreach`, etc, and before the `(`.
void DFormatter::setSpaceAfterKeywords(bool value)
{
}

// dfmt_space_before_function_parameters | `true`, **`false`** | Insert space before the opening paren of a function parameter list.
void DFormatter::setSpaceBeforeFunctionParameters(bool value)
{
}

// dfmt_selective_import_space | **`true`**, `false` | Insert space after the module name and before the `:` for selective imports.
void DFormatter::setSelectiveImportSpace(bool value)
{
}

// dfmt_compact_labeled_statements | **`true`**, `false` | Place labels on the same line as the labeled `switch`, `for`, `foreach`, or `while` statement.
void DFormatter::setCompactLabeledStatements(bool value)
{
}

// dfmt_template_constraint_style | **`conditional_newline_indent`** `conditional_newline` `always_newline` `always_newline_indent` | Control the formatting of template constraints.
void DFormatter::setTemplateContraintStyle(TemplateContraintStyle style)
{
}

// dfmt_single_template_constraint_indent | `true`, **`false`** | Set if the constraints are indented by a single tab instead of two. Has only an effect if the style set to `always_newline_indent` or `conditional_newline_indent`.
void DFormatter::setSingleTemplateContraintIndent(bool value)
{
}

// dfmt_space_before_aa_colon | `true`, **`false`** | Adds a space after an associative array key before the `:` like in older dfmt versions.
void DFormatter::setSpaceBeforeAssocArrayColon(bool value)
{
}

// dfmt_keep_line_breaks | `true`, **`false`** | Keep existing line breaks if these don't violate other formatting rules.
void DFormatter::setKeepLineBreaks(bool value)
{
}



