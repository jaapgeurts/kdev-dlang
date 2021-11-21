/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dfmt_preferences.h"

#include "dformatter.h"
#include "dfmt_plugin.h"

using namespace KDevelop;

namespace {

const int INDENT_BLOCK = 0;
const int INDENT_BRACKET = 1;
const int INDENT_CASE = 2;
const int INDENT_CLASS = 3;
const int INDENT_LABEL = 4;
const int INDENT_NAMESPACE = 5;
const int INDENT_PREPROCESSOR = 6;
const int INDENT_SWITCH = 7;

const int PADDING_NOCHANGE = 0;
const int PADDING_NO = 1;
const int PADDING_IN = 2;
const int PADDING_OUT = 3;
const int PADDING_INOUT = 4;

const int INDENT_TABS = 0;
const int INDENT_TABSFORCE = 1;
const int INDENT_SPACES = 2;

const int BRACKET_NOCHANGE = 0;
const int BRACKET_ATTACH = 1;
const int BRACKET_BREAK = 2;
const int BRACKET_LINUX = 3;
const int BRACKET_RUNINMODE = 4;

const int POINTERALIGN_NOCHANGE = 0;
const int POINTERALIGN_NAME = 1;
const int POINTERALIGN_MIDDLE = 2;
const int POINTERALIGN_TYPE = 3;

}

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
    // setup list widget to have checked items
    for(int i = 0; i < listIdentObjects->count(); i++) {
        QListWidgetItem *item = listIdentObjects->item(i);
        item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        item->setCheckState(Qt::Checked);
    }

    connect(tabWidget, &QTabWidget::currentChanged, this, &DFMTPreferences::currentTabChanged);

    connect(cbIndentType, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::indentChanged);
    connect(inpNuberSpaces, QOverload<int>::of(&QSpinBox::valueChanged), this, &DFMTPreferences::indentChanged);
    connect(chkConvertTabs, &QCheckBox::stateChanged, this, &DFMTPreferences::indentChanged);
    connect(chkFillEmptyLines, &QCheckBox::stateChanged, this, &DFMTPreferences::indentChanged);

    connect(listIdentObjects, &QListWidget::itemChanged,
             this, &DFMTPreferences::indentObjectsChanged);

    connect(inpMaxStatement, QOverload<int>::of(&QSpinBox::valueChanged), this, &DFMTPreferences::minMaxValuesChanged);
    connect(inpMinConditional, QOverload<int>::of(&QSpinBox::valueChanged), this, &DFMTPreferences::minMaxValuesChanged);

    connect(cbBrackets, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::bracketsChanged);
    connect(chkBracketsCloseHeaders, &QCheckBox::stateChanged, this, &DFMTPreferences::bracketsChanged);

    connect(chkBlockBreak, &QCheckBox::stateChanged, this, &DFMTPreferences::blocksChanged);
    connect(chkBlockBreakAll, &QCheckBox::stateChanged, this, &DFMTPreferences::blocksChanged);
    connect(chkBlockIfElse, &QCheckBox::stateChanged, this, &DFMTPreferences::blocksChanged);

    connect(cbParenthesisPadding, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &DFMTPreferences::paddingChanged);
    connect(chkPadParenthesisHeader, &QCheckBox::stateChanged, this, &DFMTPreferences::paddingChanged);
    connect(chkPadOperators, &QCheckBox::stateChanged, this, &DFMTPreferences::paddingChanged);

    connect(chkKeepStatements, &QCheckBox::stateChanged, this, &DFMTPreferences::onelinersChanged);
    connect(chkKeepBlocks, &QCheckBox::stateChanged, this, &DFMTPreferences::onelinersChanged);

    connect(cbPointerAlign, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DFMTPreferences::pointerAlignChanged);

    connect(chkAfterParens, &QCheckBox::stateChanged, this, &DFMTPreferences::afterParensChanged);
    connect(inpContinuation, QOverload<int>::of(&QSpinBox::valueChanged), this, &DFMTPreferences::afterParensChanged);
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
    if(m_formatter->option(QStringLiteral("Fill")).toString() == QLatin1String("Tabs")) {
        chkConvertTabs->setEnabled(false);
        chkConvertTabs->setChecked(false);
        if(m_formatter->option(QStringLiteral("FillForce")).toBool()) {
            cbIndentType->setCurrentIndex(INDENT_TABSFORCE);
        } else {
            cbIndentType->setCurrentIndex(INDENT_TABS);
        }
    } else {
        cbIndentType->setCurrentIndex(INDENT_SPACES);
        chkConvertTabs->setEnabled(true);
        chkConvertTabs->setChecked(m_formatter->option(QStringLiteral("FillForce")).toBool());
    }
    inpNuberSpaces->setValue(m_formatter->option(QStringLiteral("FillCount")).toInt());
    chkFillEmptyLines->setChecked(m_formatter->option(QStringLiteral("FillEmptyLines")).toBool());

    // indent objects
    setItemChecked(INDENT_BLOCK, m_formatter->option(QStringLiteral("IndentBlocks")).toBool());
    setItemChecked(INDENT_BRACKET, m_formatter->option(QStringLiteral("IndentBrackets")).toBool());
    setItemChecked(INDENT_CASE, m_formatter->option(QStringLiteral("IndentCases")).toBool());
    setItemChecked(INDENT_CLASS, m_formatter->option(QStringLiteral("IndentClasses")).toBool());
    setItemChecked(INDENT_LABEL, m_formatter->option(QStringLiteral("IndentLabels")).toBool());
    setItemChecked(INDENT_NAMESPACE, m_formatter->option(QStringLiteral("IndentNamespaces")).toBool());
    setItemChecked(INDENT_PREPROCESSOR, m_formatter->option(QStringLiteral("IndentPreprocessors")).toBool());
    setItemChecked(INDENT_SWITCH, m_formatter->option(QStringLiteral("IndentSwitches")).toBool());

    inpMaxStatement->setValue(m_formatter->option(QStringLiteral("MaxStatement")).toInt());
    inpMinConditional->setValue(m_formatter->option(QStringLiteral("MinConditional")).toInt());

    chkAfterParens->setChecked(m_formatter->option(QStringLiteral("AfterParens")).toBool());
    inpContinuation->setValue(m_formatter->option(QStringLiteral("Continuation")).toInt());
    inpContinuation->setEnabled(chkAfterParens->isChecked());

    // brackets
    QString s = m_formatter->option(QStringLiteral("Brackets")).toString();
    if(s == QLatin1String("Attach"))
        cbBrackets->setCurrentIndex(BRACKET_ATTACH);
    else if(s == QLatin1String("Break"))
        cbBrackets->setCurrentIndex(BRACKET_BREAK);
    else if(s == QLatin1String("Linux"))
        cbBrackets->setCurrentIndex(BRACKET_LINUX);
    else
        cbBrackets->setCurrentIndex(BRACKET_NOCHANGE);
    chkBracketsCloseHeaders->setChecked(
        m_formatter->option(QStringLiteral("BracketsCloseHeaders")).toBool());

    // blocks
    chkBlockBreak->setChecked(m_formatter->option(QStringLiteral("BlockBreak")).toBool());
    chkBlockBreakAll->setChecked(m_formatter->option(QStringLiteral("BlockBreakAll")).toBool());
    chkBlockIfElse->setChecked(m_formatter->option(QStringLiteral("BlockIfElse")).toBool());
    // enable or not chkBlockBreakAll
    chkBlockBreakAll->setEnabled(chkBlockBreak->isChecked());

    // padding
    bool padin = m_formatter->option(QStringLiteral("PadParenthesesIn")).toBool();
    bool padout = m_formatter->option(QStringLiteral("PadParenthesesOut")).toBool();
    bool unpad = m_formatter->option(QStringLiteral("PadParenthesesUn")).toBool();
    if(unpad) {
        if(padin) {
            if(padout)
                cbParenthesisPadding->setCurrentIndex(PADDING_INOUT);
            else
                cbParenthesisPadding->setCurrentIndex(PADDING_IN);
        } else if(padout)
            cbParenthesisPadding->setCurrentIndex(PADDING_OUT);
        else
            cbParenthesisPadding->setCurrentIndex(PADDING_NO);
    } else
        cbParenthesisPadding->setCurrentIndex(PADDING_NOCHANGE);

    // padding header has no influence with padding out
    if (padout)
        chkPadParenthesisHeader->setDisabled(true);

    chkPadParenthesisHeader->setChecked(m_formatter->option(QStringLiteral("PadParenthesesHeader")).toBool());
    chkPadOperators->setChecked(m_formatter->option(QStringLiteral("PadOperators")).toBool());
    // oneliner
    chkKeepStatements->setChecked(m_formatter->option(QStringLiteral("KeepStatements")).toBool());
    chkKeepBlocks->setChecked(m_formatter->option(QStringLiteral("KeepBlocks")).toBool());

    // pointer align
    s = m_formatter->option(QStringLiteral("PointerAlign")).toString();
    if (s == QLatin1String("Name"))
        cbPointerAlign->setCurrentIndex(POINTERALIGN_NAME);
    else if (s == QLatin1String("Type"))
        cbPointerAlign->setCurrentIndex(POINTERALIGN_TYPE);
    else if (s == QLatin1String("Middle"))
        cbPointerAlign->setCurrentIndex(POINTERALIGN_MIDDLE);
    else
        cbPointerAlign->setCurrentIndex(POINTERALIGN_NOCHANGE);

    m_enableWidgetSignals = true; // re enable signals
}

void DFMTPreferences::setItemChecked(int idx, bool checked)
{
    QListWidgetItem *item = listIdentObjects->item(idx);
    if(!item)
        return;

    if(checked)
        item->setCheckState(Qt::Checked);
    else
        item->setCheckState(Qt::Unchecked);
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


//     switch(cbIndentType->currentIndex()) {
//         case INDENT_TABS:
//             m_formatter->setTabSpaceConversionMode( false );
//             m_formatter->setTabIndentation(inpNuberSpaces->value(), false);
//             chkConvertTabs->setEnabled(false);
//             break;
//         case INDENT_TABSFORCE:
//             m_formatter->setTabSpaceConversionMode( false );
//             m_formatter->setTabIndentation(inpNuberSpaces->value(), true);
//             chkConvertTabs->setEnabled(false);
//             break;
//         case INDENT_SPACES:
//             m_formatter->setSpaceIndentation(inpNuberSpaces->value());
//             chkConvertTabs->setEnabled(true);
//             m_formatter->setTabSpaceConversionMode( chkConvertTabs->isEnabled() & chkConvertTabs->isChecked() );
//             break;
//     }
//
//     m_formatter->setFillEmptyLines( chkFillEmptyLines->isChecked() );

    updatePreviewText();
}

void DFMTPreferences::indentObjectsChanged(QListWidgetItem *item)
{
    if(!m_enableWidgetSignals)
        return;
    if(!item)
        return;

    bool checked = (item->checkState() == Qt::Checked);
//     switch(listIdentObjects->row(item)) {
//         case INDENT_BLOCK: m_formatter->setBlockIndent(checked); break;
//         case INDENT_BRACKET: m_formatter->setBracketIndent(checked); break;
//         case INDENT_CASE: m_formatter->setCaseIndent(checked); break;
//         case INDENT_CLASS: m_formatter->setClassIndent(checked); break;
//         case INDENT_LABEL: m_formatter->setLabelIndent(checked); break;
//         case INDENT_NAMESPACE: m_formatter->setNamespaceIndent(checked); break;
//         case INDENT_PREPROCESSOR: m_formatter->setPreprocessorIndent(checked); break;
//         case INDENT_SWITCH: m_formatter->setSwitchIndent(checked); break;
//     }

    updatePreviewText();
}

void DFMTPreferences::minMaxValuesChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     m_formatter->setMaxInStatementIndentLength(inpMaxStatement->value());
//     m_formatter->setMinConditionalIndentLength(inpMinConditional->value());

    updatePreviewText();
}

void DFMTPreferences::bracketsChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     switch(cbBrackets->currentIndex()) {
//         case BRACKET_NOCHANGE: m_formatter->setBracketFormatMode(astyle::NONE_MODE); break;
//         case BRACKET_ATTACH: m_formatter->setBracketFormatMode(astyle::ATTACH_MODE); break;
//         case BRACKET_BREAK: m_formatter->setBracketFormatMode(astyle::BREAK_MODE); break;
//         case BRACKET_LINUX: m_formatter->setBracketFormatMode(astyle::LINUX_MODE); break;
//         case BRACKET_RUNINMODE: m_formatter->setBracketFormatMode(astyle::RUN_IN_MODE); break;
//     }
//
//     m_formatter->setBreakClosingHeaderBracketsMode(chkBracketsCloseHeaders->isChecked());

    updatePreviewText();
}

void DFMTPreferences::blocksChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     m_formatter->setBreakBlocksMode(chkBlockBreak->isChecked());
//     m_formatter->setBreakClosingHeaderBlocksMode(chkBlockBreakAll->isChecked());
//     m_formatter->setBreakElseIfsMode(chkBlockIfElse->isChecked());

    chkBlockBreakAll->setEnabled(chkBlockBreak->isChecked());

    updatePreviewText();
}

void DFMTPreferences::paddingChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     switch(cbParenthesisPadding->currentIndex()) {
//         case PADDING_NOCHANGE:
//             m_formatter->setParensUnPaddingMode(false);
//             m_formatter->setParensInsidePaddingMode(false);
//             m_formatter->setParensOutsidePaddingMode(false);
//             chkPadParenthesisHeader->setDisabled(false);
//             break;
//         case PADDING_NO:
//             m_formatter->setParensUnPaddingMode(true);
//             m_formatter->setParensInsidePaddingMode(false);
//             m_formatter->setParensOutsidePaddingMode(false);
//             chkPadParenthesisHeader->setDisabled(false);
//             break;
//         case PADDING_IN:
//             m_formatter->setParensUnPaddingMode(true);
//             m_formatter->setParensInsidePaddingMode(true);
//             m_formatter->setParensOutsidePaddingMode(false);
//             chkPadParenthesisHeader->setDisabled(false);
//             break;
//         case PADDING_OUT:
//             m_formatter->setParensUnPaddingMode(true);
//             m_formatter->setParensInsidePaddingMode(false);
//             m_formatter->setParensOutsidePaddingMode(true);
//             padding header has no influence with padding out
//             chkPadParenthesisHeader->setDisabled(true);
//             break;
//         case PADDING_INOUT:
//             m_formatter->setParensUnPaddingMode(true);
//             m_formatter->setParensInsidePaddingMode(true);
//             m_formatter->setParensOutsidePaddingMode(true);
//             padding header has no influence with padding out
//             chkPadParenthesisHeader->setDisabled(true);
//             break;
//     }
//
//     m_formatter->setParensHeaderPaddingMode(chkPadParenthesisHeader->isChecked());
//     m_formatter->setOperatorPaddingMode(chkPadOperators->isChecked());

    updatePreviewText();
}

void DFMTPreferences::onelinersChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     m_formatter->setBreakOneLineStatementsMode(!chkKeepStatements->isChecked());
//     m_formatter->setBreakOneLineBlocksMode(!chkKeepBlocks->isChecked());

    updatePreviewText();
}

void DFMTPreferences::pointerAlignChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     switch (cbPointerAlign->currentIndex()) {
//         case POINTERALIGN_NAME:
//             m_formatter->setPointerAlignment(astyle::PTR_ALIGN_NAME);
//             break;
//         case POINTERALIGN_TYPE:
//             m_formatter->setPointerAlignment(astyle::PTR_ALIGN_TYPE);
//             break;
//         case POINTERALIGN_MIDDLE:
//             m_formatter->setPointerAlignment(astyle::PTR_ALIGN_MIDDLE);
//             break;
//         default:
//         case POINTERALIGN_NOCHANGE:
//             m_formatter->setPointerAlignment(astyle::PTR_ALIGN_NONE);
//             break;
//     }

    updatePreviewText();
}

void DFMTPreferences::afterParensChanged()
{
    if(!m_enableWidgetSignals)
        return;
//     m_formatter->setAfterParens(chkAfterParens->isChecked());
//     inpContinuation->setEnabled(chkAfterParens->isChecked());
//     m_formatter->setContinuation(inpContinuation->value());

    updatePreviewText();
}
