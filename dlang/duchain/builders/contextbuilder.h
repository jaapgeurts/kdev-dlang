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

#include <language/duchain/builders/abstractcontextbuilder.h>

#include "parser/parsesession.h"
#include "duchain/dduchainexport.h"

typedef KDevelop::AbstractContextBuilder<INode, IToken> ContextBuilderBase;

class Editor
{
public:
	Editor(ParseSession **session) : m_session(session)
	{

	}

	ParseSession *parseSession() const
	{
		return *m_session;
	}

private:
	ParseSession **m_session;
};


class KDEVDDUCHAIN_EXPORT ContextBuilder : public ContextBuilderBase
{
public:
	ContextBuilder();
	virtual ~ContextBuilder();

	virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString &url, INode *node,const KDevelop::ReferencedTopDUContext& updateContext = KDevelop::ReferencedTopDUContext()) override;

	virtual void startVisiting(INode *node) override;

    virtual void visitAddExpression(IAddExpression *node);
    virtual void visitAliasDeclaration(IAliasDeclaration* node);
    virtual void visitAliasInitializer(IAliasInitializer* node, const QString& comment);
    virtual void visitArguments(IArguments *node);
    virtual void visitAsmStatement(IAsmStatement *node);
    virtual void visitAssertArguments(IAssertArguments *node); // TODO: added
    virtual void visitAssertExpression(IAssertExpression *node);
    virtual void visitAssignExpression(IAssignExpression *node);
    virtual void visitBaseClass(IBaseClass *node);
    virtual void visitBaseClassList(IBaseClassList *node);
    virtual void visitBlockStatement(IBlockStatement *node, bool openContext);
    virtual void visitBody(IFunctionBody *node, bool openContext);
    virtual void visitBreakStatement(IBreakStatement *node);
    virtual void visitCaseRangeStatement(ICaseRangeStatement *node);
    virtual void visitCaseStatement(ICaseStatement *node);
    virtual void visitCatch(ICatch *node);
    virtual void visitClassDeclaration(IClassDeclaration *node);
    virtual void visitCmpExpression(ICmpExpression *node);
    virtual void visitCompileCondition(ICompileCondition *node);
    virtual void visitConditionalStatement(IConditionalStatement *node);
    virtual void visitConstructor(IConstructor *node);
    virtual void visitContinueStatement(IContinueStatement *node);
    virtual void visitDebugCondition(IDebugCondition *node);
    virtual void visitDebugSpecification(IDebugSpecification *node);
    virtual void visitDeclaration(IDeclaration *node);
    virtual void visitDeclarationOrStatement(IDeclarationOrStatement *node);
    virtual void visitDeclarationsAndStatements(IDeclarationsAndStatements *node);
    virtual void visitDeclarator(IDeclarator *node);
    virtual void visitDefaultStatement(IDefaultStatement *node);
    virtual void visitDestructor(IDestructor *node);
    virtual void visitDoStatement(IDoStatement *node);
    virtual void visitEnumBody(IEnumBody *node);
    virtual void visitEnumDeclaration(IEnumDeclaration *node);
    virtual void visitEnumMember(IEnumMember *node);
    virtual void visitEqualExpression(IEqualExpression *node);
    virtual void visitExpression(IExpression *node);
    virtual void visitExpressionNode(IExpressionNode *node);
    virtual void visitExpressionStatement(IExpressionStatement *node);
    virtual void visitFinally(IFinally *node);
    virtual void visitFinalSwitchStatement(IFinalSwitchStatement *node);
    virtual void visitForeachStatement(IForeachStatement *node);
    virtual void visitForeachType(IForeachType *node);
    virtual void visitForStatement(IForStatement *node);
    virtual void visitFuncDeclaration(IFunctionDeclaration *node);
    virtual void visitFunctionCallExpression(IFunctionCallExpression *node);
    virtual void visitGotoStatement(IGotoStatement *node);
    // TODO: JG remove
//     virtual void visitIdentifier(IToken* node);
    virtual void visitIdentifierOrTemplateInstance(IIdentifierOrTemplateInstance *node);
    virtual void visitIdentityExpression(IIdentityExpression *node);
    virtual void visitIfStatement(IIfStatement *node);
    virtual void visitImportBind(IImportBind* node);
    virtual void visitImportDeclaration(IImportDeclaration *node);
    virtual void visitIndexExpression(IIndexExpression *node);
    virtual void visitIndex(IIndex *node);
    virtual void visitInExpression(IInExpression *node);
    virtual void visitInitializer(IInitializer *node);
    virtual void visitInterfaceDeclaration(IInterfaceDeclaration *node);
    virtual void visitLabeledStatement(ILabeledStatement *node);
    virtual void visitLastCatch(ILastCatch *node);
    virtual void visitModule(IModule *node);
    virtual void visitMulExpression(IMulExpression* node);
    virtual void visitParameter(IParameter *node);
    virtual void visitPrimaryExpression(IPrimaryExpression *node);
    virtual void visitRelExpression(IRelExpression *node);
    virtual void visitReturnStatement(IReturnStatement *node);
    virtual void visitScopeGuardStatement(IScopeGuardStatement *node);
    virtual void visitShiftExpression(IShiftExpression *node);
    virtual void visitSingleImport(ISingleImport *node);
    virtual void visitStatement(IStatement *node);
    virtual void visitStatementNoCaseNoDefault(IStatementNoCaseNoDefault *node);
    virtual void visitStaticAssertStatement(IStaticAssertStatement *node);
    virtual void visitStaticIfCondition(IStaticIfCondition *node);
    virtual void visitStructBody(IStructBody *node);
    virtual void visitStructDeclaration(IStructDeclaration *node);
    virtual void visitSwitchStatement(ISwitchStatement *node);
    virtual void visitSynchronizedStatement(ISynchronizedStatement *node);
    virtual void visitTemplateDeclaration(ITemplateDeclaration* node);
    virtual void visitTemplateInstance(ITemplateInstance* node);
    virtual void visitTemplateParameter(ITemplateParameter* node);
    virtual void visitTernaryExpression(ITernaryExpression* node);
    virtual void visitThrowStatement(IThrowStatement *node);
    virtual void visitToken(IToken *node);
    virtual void visitTryStatement(ITryStatement *node);
    virtual void visitTypeName(IType *node) = 0;
    virtual void visitUnaryExpression(IUnaryExpression *node);
    virtual void visitVarDeclaration(IVariableDeclaration *node);
    virtual void visitVersionCondition(IVersionCondition *node);
    virtual void visitVersionSpecification(IVersionSpecification *node);
    virtual void visitWhileStatement(IWhileStatement *node);
    virtual void visitWithStatement(IWithStatement *node);

    /** Find the most appropriate context from current to global */
    virtual KDevelop::DUContext* findContextRecursive(const KDevelop::RangeInRevision& range) const;
	virtual KDevelop::DUContext *contextFromNode(INode *node) override;
	virtual void setContextOnNode(INode *node, KDevelop::DUContext *context) override;

	virtual KDevelop::RangeInRevision editorFindRange(INode *fromNode, INode *toNode) override;

	virtual KDevelop::QualifiedIdentifier identifierForNode(IToken *node) override;
	virtual KDevelop::QualifiedIdentifier identifierForNode(IIdentifierChain *node);
	virtual KDevelop::QualifiedIdentifier identifierForNode(IIdentifierOrTemplateChain *node);
    virtual KDevelop::QualifiedIdentifier identifierForNode(IIdentifierOrTemplateInstance *node);
	KDevelop::QualifiedIdentifier identifierForIndex(qint64 index);

	void setParseSession(ParseSession *session);

	virtual KDevelop::TopDUContext *newTopContext(const KDevelop::RangeInRevision &range, KDevelop::ParsingEnvironmentFile *file=0) override;

	virtual KDevelop::DUContext *newContext(const KDevelop::RangeInRevision &range) override;

	KDevelop::QualifiedIdentifier createFullName(IToken *package, IToken *typeName);

	ParseSession *parseSession();

	Editor *editor() const
	{
		return m_editor.data();
	}

protected:
	ParseSession *m_session;

	bool m_mapAst; //Make KDevelop::AbstractContextBuilder happy.
	QScopedPointer<Editor> m_editor; //Make KDevelop::AbstractUseBuilder happy.
	KDevelop::QualifiedIdentifier m_identifier; // Keeps track of the current identifier

};
