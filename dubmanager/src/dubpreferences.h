// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUBPREFERENCES_H
#define DUBPREFERENCES_H

#include <interfaces/configpage.h>
#include <interfaces/iproject.h>

#include <project/projectconfigpage.h>

#include "dubparser.h"


namespace Ui
{
class DubPreferences;
}

/**
 * @todo write docs
 */
class DubPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:

    enum class DubType {
        Sdlang,
        Json
    };

    /**
     * Default constructor
     */
    DubPreferences(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent = nullptr);

    /**
     * Destructor
     */
    ~DubPreferences();

    void init();

    void updateWidgets();

    /**
     * @todo write docs
     *
     * @return TODO
     */
    void apply() override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    void defaults() override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    void reset() override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    virtual QString name() const override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    virtual QString fullName() const override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
//     virtual QIcon icon() const override;

private:
    QScopedPointer<Ui::DubPreferences> m_ui;


    DubParser m_Parser;
    QSharedPointer<DubSettings> m_dubSettings;
    DubType m_dubType;

};

#endif // DUBPREFERENCES_H
