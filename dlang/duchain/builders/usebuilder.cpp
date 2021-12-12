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
		context = currentContext()->findContextIncluding(editorFindRange(ident, nullptr));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitTypeName: No context found for" << id;
		return;
	}
	DeclarationPointer decl = getTypeDeclaration(id, context);
    qCDebug(DUCHAIN) << "UseBuilder::visitTypeName " << id;
	if(decl) {
        qCDebug(DUCHAIN) << "UseBuilder::Decl found for typename: " << id;
		newUse(ident, decl);
    }
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
    qCDebug(DUCHAIN) << "UseBuilder::visitTemplateParam " << id;
	if(decl) {
		newUse(node, decl);
    }
}

void UseBuilder::visitPrimaryExpression(IPrimaryExpression *node)
{
	UseBuilderBase::visitPrimaryExpression(node);

	if(!node->getIdentifierOrTemplateInstance() || !currentContext())
		return;

    // Get the identifier either from the identifier or template
    IToken* ident = nullptr;
    if (auto ti = node->getIdentifierOrTemplateInstance()->getTemplateInstance()) {
        ident = ti->getIdentifier();
    }
    if (ident == nullptr) {
        ident = node->getIdentifierOrTemplateInstance()->getIdentifier();
    }

    QualifiedIdentifier identifier(identifierForNode(ident));

    m_identifier.push(identifier);

	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(ident, nullptr));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitPrimaryExpression: No context found for" << m_identifier;
		return;
	}

	DeclarationPointer decl = getDeclaration(m_identifier, context);
    // TODO: JG resolve types
//    IndexedType type = decl->indexedType();

    qCDebug(DUCHAIN) << "UseBuilder::visitPrimaryExpression " << m_identifier;
	if(decl) {
        // A declaration was found!
		newUse(ident, decl);
    }
}

void UseBuilder::visitExpression(IExpression* node)
{
    UseBuilderBase::visitExpression(node);
    m_identifier.clear();
}


void UseBuilder::visitUnaryExpression(IUnaryExpression *node)
{
    // If there are more unary expressions to visit, go there first to build the identifier chain
	UseBuilderBase::visitUnaryExpression(node);

    if(!node->getIdentifierOrTemplateInstance() || !currentContext()) {
		return;
    }

    // Get the identifier either from the identifier or template
    IToken* ident = nullptr;

    if (auto i = node->getIdentifierOrTemplateInstance()->getTemplateInstance()) {
        ident = i->getIdentifier();
    }
    if (ident == nullptr) {
        ident = node->getIdentifierOrTemplateInstance()->getIdentifier();
    }

	DUContext *context = nullptr;
	{
		DUChainReadLocker lock;
		context = currentContext()->findContextIncluding(editorFindRange(ident, nullptr));
	}
	if(!context)
	{
		qCDebug(DUCHAIN) << "visitUnaryExpression: No context found for" << ident->getText();
		return;
	}

	QualifiedIdentifier id(identifierForNode(ident));
    m_identifier.push(id);
    qCDebug(DUCHAIN) << "current idchain: " << m_identifier;


	DeclarationPointer decl = getDeclaration(m_identifier, context);
    qCDebug(DUCHAIN) << "UseBuilder::visitUnaryExpression " << id;
	if(decl) {
		newUse(node, decl);
    }
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
	if(decl) {
		newUse(node, decl);
    }
}


}
