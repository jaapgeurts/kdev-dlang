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

#pragma once

#include <QStack>

#include <language/duchain/builders/abstractusebuilder.h>
#include <language/duchain/identifier.h>

#include "contextbuilder.h"


typedef KDevelop::AbstractUseBuilder<INode, IToken, ContextBuilder> UseBuilderBase;

class KDEVDDUCHAIN_EXPORT UseBuilder : public UseBuilderBase
{
public:
	UseBuilder(ParseSession *session);

	virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString &url, INode *node, const KDevelop::ReferencedTopDUContext& updateContext = KDevelop::ReferencedTopDUContext()) override;
	virtual void startVisiting(INode *node) override;
	virtual void visitPrimaryExpression(IPrimaryExpression *node) override;
    virtual void visitTemplateParameter(ITemplateParameter* node) override;
	virtual void visitToken(IToken *node) override;
	virtual void visitTypeName(IType *node) override;
	virtual void visitUnaryExpression(IUnaryExpression *node) override;

private:
	QStack<KDevelop::AbstractType::Ptr> m_types;

};

