/*************************************************************************************
 *  Copyright (C) 2015 by Thomas Brix Larsen <brix@brix-verden.dk>                   *
 *  Copyright (C) 2014 by Pavel Petrushkov <onehundredof@gmail.com>                  *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "completionitem.h"

#include <language/duchain/declaration.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainlock.h>

#include "context.h"

CompletionItem::CompletionItem(KDevelop::DeclarationPointer decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext> context, int inheritanceDepth) :
    NormalDeclarationCompletionItem(decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), 0),
    m_prefix("")
{
    Q_UNUSED(context);
    Q_UNUSED(inheritanceDepth);
	DUChainReadLocker lock;
	if(!decl)
		return;

	if(decl->kind() == KDevelop::Declaration::Import || decl->kind() == KDevelop::Declaration::NamespaceAlias)
		m_prefix = "module";
}

QVariant CompletionItem::data(const QModelIndex &index, int role, const KDevelop::CodeCompletionModel *model) const
{
	return NormalDeclarationCompletionItem::data(index, role, model);
}
