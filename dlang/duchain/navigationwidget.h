// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <language/duchain/navigation/abstractnavigationwidget.h>

#include "builders/ddeclaration.h"

namespace KDevelop
{
class DocumentCursor;
class IncludeItem;
}

/**
 * @todo write docs
 */
class DlangNavigationWidget : public KDevelop::AbstractNavigationWidget
{
    Q_OBJECT

public:

DlangNavigationWidget(const DDeclaration::Ptr & decl, const KDevelop::DocumentCursor& expansionLocation, KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);


~DlangNavigationWidget();

};

#endif // NAVIGATIONWIDGET_H
