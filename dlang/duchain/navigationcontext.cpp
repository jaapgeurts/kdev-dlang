// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "navigationcontext.h"
#include "builders/ddeclaration.h"

#include "duchaindebug.h"

#include <QPushButton>

using namespace KDevelop;

DlangNavigationContext::DlangNavigationContext(const DDeclaration::Ptr& decl, const DocumentCursor& expansionLocation) :
            m_decl(decl),
            m_location(expansionLocation)
{
}


DlangNavigationContext::~DlangNavigationContext()
{
}

QString DlangNavigationContext::name() const
{
    return m_decl->identifier().toString();
}

QString DlangNavigationContext::html(bool shorten)
{
    Q_UNUSED(shorten);
    clear();

    modifyHtml() += QStringLiteral("<html><body><p>");

    if (m_decl->dKind() == DDeclaration::Kind::Template)
        modifyHtml() += "template ";
    else if (m_decl->dKind() == DDeclaration::Kind::Import)
        modifyHtml() += "import ";

    modifyHtml() += name();

    modifyHtml() += QStringLiteral("<br/>Container: ");

    QString fname = m_location.document.toUrl().fileName();

    modifyHtml() += QStringLiteral("<br/>Decl: ") + fname + QStringLiteral(":") + QString::number(m_location.line()+1);

    modifyHtml() += QStringLiteral("</p><p>DLang</p></body></html>");

    return currentHtml();
}

// QWidget* DlangNavigationContext::widget() const {
//
// //    Called to add an additional widget to the Use popup tooltip
//     return nullptr;
// }
