// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <language/duchain/navigation/abstractnavigationcontext.h>

#include "navigationwidget.h"
#include "navigationcontext.h"

using namespace KDevelop;

DlangNavigationWidget::DlangNavigationWidget(const DDeclaration::Ptr& decl, const DocumentCursor& expansionLocation, AbstractNavigationWidget::DisplayHints hints)
{
    setDisplayHints(hints);
    initBrowser(400);

    setContext(NavigationContextPointer(new DlangNavigationContext(decl, expansionLocation)));
}

DlangNavigationWidget::~DlangNavigationWidget()
{
}

//#include "navigationwidget.moc"
