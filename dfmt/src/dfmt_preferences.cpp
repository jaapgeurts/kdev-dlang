/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dfmt_preferences.h"

#include "dformatter.h"
#include "dfmt_plugin.h"

using namespace KDevelop;

// these exist to map the UI comboboxes to enums

static const int INDENT_TABS = 0;
static const int INDENT_SPACES = 1;

static const int BRACESTYLE_ALLMAN = 0;
static const int BRACESTYLE_OTBS = 1; // one true brace style
static const int BRACESTYLE_STROUSTRUP = 2;

static const int TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE = 0;
static const int TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE_INDENT = 1;
static const int TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE = 2;
static const int TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE_INDENT = 3;


DFMTPreferences::DFMTPreferences(QWidget *parent)
    : SettingsWidget(parent)
    , m_formatter(new DFormatter)
{
    setupUi(this);
    m_enableWidgetSignals = true;
    init();
}

DFMTPreferences::~DFMTPreferences( )
{
}

void DFMTPreferences::init()
{

    connect(tabWidget, &QTabWidget::currentChanged, this, &DFMTPreferences::currentTabChanged);

    // Tabs
    connect(cbIndentType, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::indentChanged);
    connect(sbNumberSpaces, QOverload<int>::of(&QSpinBox::valueChanged), this, &DFMTPreferences::indentChanged);
    connect(chkSingleIndent, &QCheckBox::stateChanged, this, &DFMTPreferences::singleIndentChanged);

    // Spaces
    connect(chkSpaceAfterCast, &QCheckBox::stateChanged, this, &DFMTPreferences::spaceAfterCastChanged);
    connect(chkSpaceBeforeFunctionParameters, &QCheckBox::stateChanged, this, &DFMTPreferences::spaceBeforeFunctionParametersChanged);
    connect(chkSelectiveImportSpace, &QCheckBox::stateChanged, this, &DFMTPreferences::selectiveImportSpaceChanged);
    connect(chkSpaceBeforeAssocArrayColon, &QCheckBox::stateChanged, this, &DFMTPreferences::spaceBeforeAssocArrayColonChanged);

    // brace style
    connect(cbBraceStyle, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::braceStyleChanged);


    // Alignment
    connect(chkKeepLineBreaks, &QCheckBox::stateChanged, this, &DFMTPreferences::keepLineBreaksChanged);
    connect(chkAlignSwitchStatements, &QCheckBox::stateChanged, this, &DFMTPreferences::alignSwitchStatementsChanged);
    connect(chkSplitOperatorAtLineEnd, &QCheckBox::stateChanged, this, &DFMTPreferences::splitOperatorAtLineEndChanged);
    connect(chkCompactLabeledStatements, &QCheckBox::stateChanged, this, &DFMTPreferences::compactLabeledStatementsChanged);
    //connect(chkOutdentAttributes, &QCheckBox::stateChanged, this, &DFMTPreferences::outdentAttributesChanged);

    // Templates
    connect(cbTemplateConstraintStyle, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::templateConstraintStyleChanged);
    connect(chkSingleTemplateConstraintIndent, &QCheckBox::stateChanged, this, &DFMTPreferences::singleTemplateConstraintIndentChanged);
}

void DFMTPreferences::load(const SourceFormatterStyle &style)
{
    if(!style.content().isEmpty())
        m_formatter->loadStyle(style.content());
    else
        m_formatter->predefinedStyle(style.name());

    updateWidgets();
    updatePreviewText();
}

QString DFMTPreferences::save()
{
    return m_formatter->saveStyle();
}

void DFMTPreferences::updateWidgets()
{
    // block signals to avoid writing stuff to m_formatter
    m_enableWidgetSignals = false;
    //indent
    if (m_formatter->option("indent_style").toString() == QLatin1String("tab")) {
        cbIndentType->setCurrentIndex(INDENT_TABS);
        sbNumberSpaces->setValue(m_formatter->option(QStringLiteral("tab_width")).toInt());
        chkSingleIndent->setEnabled(true);
    } else {
        cbIndentType->setCurrentIndex(INDENT_SPACES);
        sbNumberSpaces->setValue(m_formatter->option(QStringLiteral("indent_size")).toInt());
        chkSingleIndent->setEnabled(false);
    }
    chkSingleIndent->setChecked(m_formatter->option(QStringLiteral("dfmt_single_indent")).toBool());

    //spaces
    chkSpaceAfterCast->setChecked(m_formatter->option(QStringLiteral("dfmt_space_after_cast")).toBool());
    chkSpaceBeforeFunctionParameters->setChecked(m_formatter->option(QStringLiteral("dfmt_space_before_function_parameters")).toBool());
    chkSelectiveImportSpace->setChecked(m_formatter->option(QStringLiteral("dfmt_selective_import_space")).toBool());
    chkSpaceBeforeAssocArrayColon->setChecked(m_formatter->option(QStringLiteral("dfmt_space_before_aa_colon")).toBool());
    chkSpaceAfterKeywords->setChecked(m_formatter->option(QStringLiteral("dfmt_space_after_keywords")).toBool());

    // brace style
    QString option = m_formatter->option("dfmt_brace_style").toString();
    if (option == QLatin1String("allman")) {
        cbBraceStyle->setCurrentIndex(BRACESTYLE_ALLMAN);
    } else if (option == QLatin1String("otbs")) {
        cbBraceStyle->setCurrentIndex(BRACESTYLE_OTBS);
    } else if (option == QLatin1String("stroustrup")) {
        cbBraceStyle->setCurrentIndex(BRACESTYLE_STROUSTRUP);
    }

    // Alignment
    chkKeepLineBreaks->setChecked(m_formatter->option(QStringLiteral("dfmt_keep_line_breaks")).toBool());
    chkAlignSwitchStatements->setChecked(m_formatter->option(QStringLiteral("dfmt_align_switch_statements")).toBool());
    chkSplitOperatorAtLineEnd->setChecked(m_formatter->option(QStringLiteral("dfmt_split_operator_at_line_end")).toBool());
    chkCompactLabeledStatements->setChecked(m_formatter->option(QStringLiteral("dfmt_compact_labeled_statements")).toBool());
    chkOutdentAttributes->setChecked(m_formatter->option(QStringLiteral("dfmt_outdent_attributes")).toBool());

    // templates
    option = m_formatter->option("dfmt_template_constraint_style").toString();
    if (option == QLatin1String("always_newline")) {
        cbTemplateConstraintStyle->setCurrentIndex(TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE);
    } else if (option == QLatin1String("always_newline_indent")) {
        cbTemplateConstraintStyle->setCurrentIndex(TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE_INDENT);
    } else if (option == QLatin1String("conditional_newline")) {
        cbTemplateConstraintStyle->setCurrentIndex(TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE);
    } else if (option == QLatin1String("conditional_newline_indent")) {
        cbTemplateConstraintStyle->setCurrentIndex(TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE_INDENT);
    }
    chkSingleTemplateConstraintIndent->setChecked(m_formatter->option(QStringLiteral("dfmt_single_template_constraint_indent")).toBool());

    m_enableWidgetSignals = true; // re enable signals
}

void DFMTPreferences::updatePreviewText(bool emitChangedSignal)
{
    Q_UNUSED(emitChangedSignal);
    if(tabWidget->currentIndex() == 0)
        emit previewTextChanged(DFormatPlugin::indentingSample());
    else
        emit previewTextChanged(DFormatPlugin::formattingSample());
}

void DFMTPreferences::currentTabChanged()
{
    updatePreviewText(false);
}

void DFMTPreferences::indentChanged()
{
    if(!m_enableWidgetSignals)
        return;

    switch(cbIndentType->currentIndex()) {
        case INDENT_TABS:
            m_formatter->setTabWidth(sbNumberSpaces->value());
            m_formatter->setIndentStyle(DFormatter::IndentStyle::Tabs);
            chkSingleIndent->setEnabled(false);
            break;
        case INDENT_SPACES:
            m_formatter->setIndentSize(sbNumberSpaces->value());
            m_formatter->setIndentStyle(DFormatter::IndentStyle::Spaces);
            chkSingleIndent->setEnabled(true);
            break;
    }


    updatePreviewText();
}

void DFMTPreferences::singleIndentChanged()
{
     if(!m_enableWidgetSignals)
        return;

     m_formatter->setSingleIndent(chkSingleIndent->isChecked());

     updatePreviewText();
}


void DFMTPreferences::spaceAfterCastChanged()
{
     if(!m_enableWidgetSignals)
        return;

     m_formatter->setSpaceAfterCast(chkSpaceAfterCast->isChecked());

     updatePreviewText();
}

void DFMTPreferences::spaceBeforeFunctionParametersChanged()
{
     if(!m_enableWidgetSignals)
        return;

     m_formatter->setSpaceBeforeFunctionParameters(chkSpaceBeforeFunctionParameters->isChecked());

     updatePreviewText();
}

void DFMTPreferences::selectiveImportSpaceChanged()
{
     if(!m_enableWidgetSignals)
        return;

     m_formatter->setSelectiveImportSpace(chkSelectiveImportSpace->isChecked());

     updatePreviewText();
}

void DFMTPreferences::spaceBeforeAssocArrayColonChanged()
{
     if(!m_enableWidgetSignals)
        return;

     m_formatter->setSpaceBeforeAssocArrayColon(chkSpaceBeforeAssocArrayColon->isChecked());


     updatePreviewText();
}


void DFMTPreferences::braceStyleChanged()
{
    if(!m_enableWidgetSignals)
        return;

    switch(cbBraceStyle->currentIndex()) {
        case BRACESTYLE_ALLMAN:
            m_formatter->setBraceStyle(DFormatter::BraceStyle::Allman);
            break;
        case BRACESTYLE_OTBS:
            m_formatter->setBraceStyle(DFormatter::BraceStyle::Otbs);
            break;
        case BRACESTYLE_STROUSTRUP:
            m_formatter->setBraceStyle(DFormatter::BraceStyle::Stroustrup);
            break;

    }

    updatePreviewText();
}

void DFMTPreferences::keepLineBreaksChanged()
{
    if(!m_enableWidgetSignals)
        return;

    m_formatter->setKeepLineBreaks(chkKeepLineBreaks->isChecked());

    updatePreviewText();
}

void DFMTPreferences::alignSwitchStatementsChanged()
{
    if(!m_enableWidgetSignals)
        return;

    m_formatter->setAlignSwitchStatements(chkAlignSwitchStatements->isChecked());

    updatePreviewText();
}

void DFMTPreferences::splitOperatorAtLineEndChanged()
{
    if(!m_enableWidgetSignals)
        return;

    m_formatter->setSplitOperatorAtLineEnd(chkSplitOperatorAtLineEnd->isChecked());

    updatePreviewText();
}

void DFMTPreferences::compactLabeledStatementsChanged()
{
    if(!m_enableWidgetSignals)
        return;

    m_formatter->setCompactLabeledStatements(chkCompactLabeledStatements->isChecked());

    updatePreviewText();
}

//void DFMTPreferences::outdentAttributesChanged()
// {
//     if(!m_enableWidgetSignals)
//         return;
//
//     m_formatter->setOutdentAttributes(chkOutdentAttributes->isChecked());
//
//     updatePreviewText();
// }

// templates
void DFMTPreferences::templateConstraintStyleChanged()
{
    if(!m_enableWidgetSignals)
        return;

    switch (cbTemplateConstraintStyle->currentIndex()) {
        case TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE:
            m_formatter->setTemplateContraintStyle(DFormatter::TemplateContraintStyle::AlwaysNewline);
            break;
        case TEMPLATE_CONSTRAINT_ALWAYS_NEWLINE_INDENT:
            m_formatter->setTemplateContraintStyle(DFormatter::TemplateContraintStyle::AlwaysNewlineIndent);
            break;
        case TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE:
            m_formatter->setTemplateContraintStyle(DFormatter::TemplateContraintStyle::ConditionaleNewline);
            break;
        case TEMPLATE_CONSTRAINT_CONDITIONAL_NEWLINE_INDENT:
            m_formatter->setTemplateContraintStyle(DFormatter::TemplateContraintStyle::ConditionalNewlineIndent);
            break;
    }

    updatePreviewText();
}

void DFMTPreferences::singleTemplateConstraintIndentChanged()
{
    if(!m_enableWidgetSignals)
        return;

    m_formatter->setSingleTemplateContraintIndent(chkSingleTemplateConstraintIndent->isChecked());

    updatePreviewText();
}
