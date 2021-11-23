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

class FormatJob;

/**
 * @todo write docs
 */
class DFormatter {
public:

    enum class BraceStyle {
        Allman,
        Otbs,
        Stroustrup
    };

    enum class TemplateContraintStyle {
        ConditionalNewlineIndent,
        ConditionaleNewline,
        AlwaysNewline,
        AlwaysNewlineIndent
    };

    /**
     * Default constructor
     */
    DFormatter();

    /**
     * Destructor
     */
    virtual ~DFormatter();

    QString formatSource(const QString& text, const QString& leftContext = QString(), const QString& rightContext = QString());

    QVariant option(const QString &name) const;

    bool predefinedStyle(const QString &name);
    void loadStyle(const QString &content);
    QString saveStyle() const;

protected Q_SLOTS:
    // indent

    // DON'T USE: end_of_line | `cr`, `crlf` and **`lf`** | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#end_of_line)

    // DON'T USE: insert_final_newline | **`true`** | Not supported. `dfmt` always inserts a final newline.

    // DON'T USE: charset | **`UTF-8`** | Not supported. `dfmt` only works correctly on UTF-8.

    // indent_style | `tab`, **`space`** | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_style)
    void setIndentStyle(bool mode);

    // indent_size | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#indent_size)
    void setIndentSize(int length);

    // tab_width | positive integers (**`4`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#tab_width)
    void setTabWidth(int length);

    // dfmt_single_indent | `true`, **`false`** | Set if the code in parens is indented by a single tab instead of two.
    void setSingleIndent(bool vaue);

    // DON'T USE: trim_trailing_whitespace | **`true`** | Not supported. `dfmt` does not emit trailing whitespace.

    // DON'T USE: max_line_length | positive integers (**`120`**) | [See EditorConfig documentation.](https://github.com/editorconfig/editorconfig/wiki/EditorConfig-Properties#max_line_length)

    // dfmt_brace_style | **`allman`**, `otbs`, or `stroustrup` | [See Wikipedia](https://en.wikipedia.org/wiki/Brace_style)
    void setBraceStyle(BraceStyle braceStyle);

    // DON'T USE: dfmt_soft_max_line_length | positive integers (**`80`**) | The formatting process will usually keep lines below this length, but they may be up to *max_line_length* columns long.

    // dfmt_align_switch_statements | **`true`**, `false` | Align labels, cases, and defaults with their enclosing switch.
    void setAlignSwitchStatements(bool value);

    // dfmt_outdent_attributes (Not yet implemented) | **`true`**, `false`| Decrease the indentation level of attributes.
    void setOutdentAttributes(bool value);

    // dfmt_split_operator_at_line_end | `true`, **`false`** | Place operators on the end of the previous line when splitting lines.
    void setSplitOperatorAtLineEnd(bool value);

    // dfmt_space_after_cast | **`true`**, `false` | Insert space after the closing paren of a `cast` expression.
    void setSpaceAfterCast(bool value);

    // dfmt_space_after_keywords (Not yet implemented) | **`true`**, `false` | Insert space after `if`, `while`, `foreach`, etc, and before the `(`.
    void setSpaceAfterKeywords(bool value);

    // dfmt_space_before_function_parameters | `true`, **`false`** | Insert space before the opening paren of a function parameter list.
    void setSpaceBeforeFunctionParameters(bool value);

    // dfmt_selective_import_space | **`true`**, `false` | Insert space after the module name and before the `:` for selective imports.
    void setSelectiveImportSpace(bool value);

    // dfmt_compact_labeled_statements | **`true`**, `false` | Place labels on the same line as the labeled `switch`, `for`, `foreach`, or `while` statement.
    void setCompactLabeledStatements(bool value);

    // dfmt_template_constraint_style | **`conditional_newline_indent`** `conditional_newline` `always_newline` `always_newline_indent` | Control the formatting of template constraints.
    void setTemplateContraintStyle(TemplateContraintStyle style);

    // dfmt_single_template_constraint_indent | `true`, **`false`** | Set if the constraints are indented by a single tab instead of two. Has only an effect if the style set to `always_newline_indent` or `conditional_newline_indent`.
    void setSingleTemplateContraintIndent(bool value);

    // dfmt_space_before_aa_colon | `true`, **`false`** | Adds a space after an associative array key before the `:` like in older dfmt versions.
    void setSpaceBeforeAssocArrayColon(bool value);

    // dfmt_keep_line_breaks | `true`, **`false`** | Keep existing line breaks if these don't violate other formatting rules.
    void setKeepLineBreaks(bool value);

private:

    FormatJob* m_job;

    void updateFormatter();
    void resetStyle();

    QVariantMap m_options;

};

#endif // DFORMATTER_H
