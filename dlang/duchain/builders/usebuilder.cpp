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

#include "usebuilder.h"

#include "helper.h"
#include "duchaindebug.h"

using namespace KDevelop;

namespace dlang
{

UseBuilder::UseBuilder(ParseSession *session)
{
	setParseSession(session);
}

ReferencedTopDUContext UseBuilder::build(const IndexedString &url, INode *node,const ReferencedTopDUContext& updateContext)
{
	qCDebug(DUCHAIN) << "Uses builder run";
	return UseBuilderBase::build(url, node, updateContext);
}

void UseBuilder::startVisiting(INode *node)
{
	UseBuilderBase::startVisiting(node);
}

void UseBuilder::visitTypeName(IType *node)
{
	if(!node || !currentContext())
		return;

    // TODO: fixed here
	QualifiedIdentifier id;
    if (node->getType2()->getTypeIdentifierPart()) {
        if (node->getType2()->getTypeIdentifierPart()->getIdentifierOrTemplateInstance()) {
            id = identifierForNode(node->getType2()->getTypeIdentifierPart()->getIdentifierOrTemplateInstance()->getIdentifier());
        }
    }
	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(node, 0));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitTypeName: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getTypeDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}

void UseBuilder::visitPrimaryExpression(IPrimaryExpression *node)
{
	UseBuilderBase::visitPrimaryExpression(node);

	if(!node->getIdentifierOrTemplateInstance() || !currentContext())
		return;

    // Get the identifier either from the identifier or template
    IToken* ident = nullptr;
    if (node->getIdentifierOrTemplateInstance()->getTemplateInstance()) {
        ident = node->getIdentifierOrTemplateInstance()->getTemplateInstance()->getIdentifier();
    }
    if (ident == nullptr) {
        ident = node->getIdentifierOrTemplateInstance()->getIdentifier();
    }
    if (ident == nullptr) {
        return;
    }

	QualifiedIdentifier id(identifierForNode(ident));
	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(ident, 0));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitPrimaryExpression: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}

void UseBuilder::visitUnaryExpression(IUnaryExpression *node)
{
	UseBuilderBase::visitUnaryExpression(node);
	if(!node->getIdentifierOrTemplateInstance() || !node->getIdentifierOrTemplateInstance()->getIdentifier() || !currentContext())
		return;

	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(node->getIdentifierOrTemplateInstance()->getIdentifier(), 0));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitUnaryExpression: No context found for" << node->getIdentifierOrTemplateInstance()->getIdentifier()->getText();
		return;
	}

	QualifiedIdentifier id;
	for(const QString &str : identifierChain)
	{
		auto t = getTypeOrVarDeclaration(QualifiedIdentifier(str), context);
		if(!t)
			continue;
		if(!t->type<AbstractType>())
		{
			id.push(Identifier(str));
			continue;
		}
		for(const QString &part : t->type<AbstractType>()->toString().split("::", Qt::SkipEmptyParts))
			id.push(Identifier(part));
	}
	id.push(identifierForNode(node->getIdentifierOrTemplateInstance()->getIdentifier()));
	identifierChain.clear();

	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}

void UseBuilder::visitToken(IToken *node)
{
	UseBuilderBase::visitToken(node);
	if(!node || !currentContext())
		return;

	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(node, 0));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitToken: No context found for" << node->getText();
		return;
	}

	QualifiedIdentifier id = identifierForNode(node);

	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}


}
