// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TEMPLATENAVIGATIONCONTEXT_H
#define TEMPLATENAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/declaration.h>
#include <language/editor/documentcursor.h>

#include "builders/ddeclaration.h"


/**
 * @todo write docs
 */
class DlangNavigationContext : public KDevelop::AbstractNavigationContext
{
public:

    explicit DlangNavigationContext(const DDeclaration::Ptr& decl,
                           const KDevelop::DocumentCursor& expansionLocation = KDevelop::DocumentCursor::invalid());
    ~DlangNavigationContext() override;

    QString html(bool shorten) override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    virtual QString name() const override;

    // Called to add an additional widget into the popup
//     virtual QWidget* widget() const override;

private:

    const DDeclaration::Ptr m_decl;
    KDevelop::DocumentCursor m_location;

};

#endif // TEMPLATENAVIGATIONCONTEXT_H
