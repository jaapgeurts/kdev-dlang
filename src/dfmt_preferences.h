/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DMFT_PREFERENCES_H
#define DMFT_PREFERENCES_H

#include <interfaces/isourceformatter.h>
#include "ui_dfmt_preferences.h"

class DFormatter;

class DFMTPreferences : public KDevelop::SettingsWidget, public Ui::DFMTPreferences
{
        Q_OBJECT

public:

    explicit DFMTPreferences(QWidget *parent=nullptr);
    ~DFMTPreferences() override;

    void load(const KDevelop::SourceFormatterStyle &style) override;
    QString save() override;

protected:
    void init();
    void updatePreviewText(bool emitChangedSignal = true);
    void setItemChecked(int idx, bool checked);
    void updateWidgets();

private Q_SLOTS:
    void currentTabChanged();
    void indentChanged();
    void indentObjectsChanged(QListWidgetItem *item);
    void minMaxValuesChanged();
    void bracketsChanged();
    void blocksChanged();
    void paddingChanged();
    void onelinersChanged();
    void pointerAlignChanged();
    void afterParensChanged();

private:
    QScopedPointer<DFormatter> m_formatter;
    bool m_enableWidgetSignals;
};

#endif // ASTYLEPREFERENCES_H
