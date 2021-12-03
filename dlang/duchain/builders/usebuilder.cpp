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
	qCDebug(DUCHAIN) << "Uses builder startVisiting";
	UseBuilderBase::startVisiting(node);
}

void UseBuilder::visitTypeName(IType *node)
{
	if(!node || !currentContext())
		return;

    IToken* ident = nullptr;
    if (auto node2 = node->getType2()->getTypeIdentifierPart()) {

        // Get the identifier either from the identifier or template
        if (node2->getIdentifierOrTemplateInstance()->getTemplateInstance()) {
            ident = node2->getIdentifierOrTemplateInstance()->getTemplateInstance()->getIdentifier();
        }
        if (ident == nullptr) {
            ident = node2->getIdentifierOrTemplateInstance()->getIdentifier();
        }
    } else {
        // must be built in type
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
		qCDebug(DUCHAIN) << "visitTypeName: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getTypeDeclaration(id, context);
	if(decl)
		newUse(ident, decl);
}

void UseBuilder::visitTemplateParameter(ITemplateParameter* node)
{
    if(!node || !currentContext())
		return;

	QualifiedIdentifier id;
    if (auto n = node->getTemplateTypeParameter()) {
        id = identifierForNode(n->getIdentifier());
    }
    else {
        qCDebug(DUCHAIN) << "QualifiedIdentifier not found: " << id;
    }

    DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(node, 0));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitTemplateParameter: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}


void UseBuilder::visitPrimaryExpression(IPrimaryExpression *node)
{
	UseBuilderBase::visitPrimaryExpression(node);

	if(!node->getIdentifierOrTemplateInstance() || !currentContext())
		return;

    // TODO: JG instead of this can we skip this and let it descend to visitToken?

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
		context = currentContext()->findContextIncluding(editorFindRange(ident, nullptr));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitPrimaryExpression: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(ident, decl);
}

void UseBuilder::visitUnaryExpression(IUnaryExpression *node)
{
    // JG: can also be a new expression
	UseBuilderBase::visitUnaryExpression(node);
	if(!node->getIdentifierOrTemplateInstance() || !node->getIdentifierOrTemplateInstance()->getIdentifier() || !currentContext())
		return;

	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(node->getIdentifierOrTemplateInstance()->getIdentifier(), nullptr));
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
		{
            // TODO: JG here or outside the for loop?
            DUChainReadLocker lock;
            for(const QString &part : t->type<AbstractType>()->toString().split("::", Qt::SkipEmptyParts))
                id.push(Identifier(part));
        }
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
		context = currentContext()->findContextIncluding(editorFindRange(node, nullptr));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitToken: No context found for" << node->getText();
		return;
	}

	QualifiedIdentifier id = identifierForNode(node);
    qCDebug(DUCHAIN) << "UseBuilder::visitToken " << id;

	DeclarationPointer decl = getDeclaration(id, context);
	if(decl)
		newUse(node, decl);
}


}
