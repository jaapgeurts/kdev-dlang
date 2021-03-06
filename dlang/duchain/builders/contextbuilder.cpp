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

#include <language/duchain/types/delayedtype.h>

#include "contextbuilder.h"
#include "duchaindebug.h"


using namespace KDevelop;

ContextBuilder::ContextBuilder()
{
	m_mapAst = false;
}

ContextBuilder::~ContextBuilder()
{

}

KDevelop::ReferencedTopDUContext ContextBuilder::build(const KDevelop::IndexedString &url, INode *node,const KDevelop::ReferencedTopDUContext& updateContext)
{
	return KDevelop::AbstractContextBuilder<INode, IToken>::build(url, node, updateContext);
}

void ContextBuilder::startVisiting(INode *node)
{
    qCDebug(DUCHAIN) << "ContextBuilder StartVisiting";
	if(!node || node == (INode *)0x1)
		return;

	if(node->getKind() == Kind::module_)
	{
		auto module = (IModule *)node;
		visitModule(module);
	}
}

void ContextBuilder::visitModule(IModule *node)
{
	for(size_t i=0; i<node->numDeclarations(); i++)
	{
		if(auto n = node->getDeclaration(i))
			visitDeclaration(n);
	}
}

void ContextBuilder::visitMulExpression(IMulExpression* node)
{
	if(auto n = node->getLeft())
		visitExpressionNode(n);
	if(auto n = node->getRight())
		visitExpressionNode(n);
}


KDevelop::DUContext *ContextBuilder::contextFromNode(INode *node)
{
	return (KDevelop::DUContext *)node->getContext();
}

KDevelop::RangeInRevision ContextBuilder::editorFindRange(INode *fromNode, INode *toNode)
{
	if(!fromNode)
		return KDevelop::RangeInRevision();
	return m_session->findRange(fromNode, toNode ? toNode : fromNode);
}

// TODO: JG Should return an identifier; not QualifiedIdentifier
KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(IToken *node)
{
	if(!node || node == (IToken *)0x1)
		return QualifiedIdentifier();
	return QualifiedIdentifier(node->getText());
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(IIdentifierChain *node)
{
	if(!node)
		return QualifiedIdentifier();
	QualifiedIdentifier ident;
	for(size_t i=0; i<node->numIdentifiers(); i++)
		ident.push(Identifier(node->getIdentifier(i)->getText()));
	return ident;
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(IIdentifierOrTemplateChain *node)
{
	if(!node)
		return QualifiedIdentifier();
	QualifiedIdentifier ident;
	for(size_t i=0; i<node->numIdentifiersOrTemplateInstances(); i++)
		ident.push(Identifier(node->getIdentifiersOrTemplateInstance(i)->getIdentifier()->getText()));
	return ident;
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode ( IIdentifierOrTemplateInstance* node )
{
    if (!node)
        return QualifiedIdentifier();
    QualifiedIdentifier ident;
    if (node->getTemplateInstance())
        ident.push(Identifier(node->getTemplateInstance()->getIdentifier()->getText()));
    else
        ident.push(Identifier(node->getIdentifier()->getText()));

    return ident;
}



KDevelop::QualifiedIdentifier ContextBuilder::identifierForIndex(qint64 index)
{
	Q_UNUSED(index)
    // TODO: implement
	qCDebug(DUCHAIN) << "TODO: Implement " << __FUNCTION__ << " (" << __FILE__ << ":" <<__LINE__<< ")";
	return QualifiedIdentifier();
}

void ContextBuilder::setContextOnNode(INode *node, KDevelop::DUContext *context)
{
    node->setContext(context);
}

void ContextBuilder::setParseSession(ParseSession *session)
{
	m_session = session;
}

TopDUContext *ContextBuilder::newTopContext(const RangeInRevision &range, ParsingEnvironmentFile *file)
{
	if(!file)
	{
		file = new ParsingEnvironmentFile(m_session->currentDocument());
		file->setLanguage(ParseSession::languageString());
	}
	return new KDevelop::TopDUContext(m_session->currentDocument(), range, file);
}

DUContext *ContextBuilder::newContext(const RangeInRevision &range)
{
	return new KDevelop::DUContext(range, currentContext());
}

QualifiedIdentifier ContextBuilder::createFullName(IToken *package, IToken *typeName)
{
	QualifiedIdentifier id(QString::fromLocal8Bit(package->getText()) + "." + QString::fromLocal8Bit(typeName->getText()));
	return id;
}

ParseSession *ContextBuilder::parseSession()
{
	return m_session;
}

void ContextBuilder::visitSingleImport(ISingleImport *node)
{
	DUChainWriteLocker lock;
    // TODO: JG import bindings are ignored
    // Lives in importdeclaration.
    IIdentifierChain* chain = node->getIdentifierChain();
//     qCDebug(DUCHAIN) << "Import chain: " << identifierForNode(chain).toString();
	QList<ReferencedTopDUContext> contexts = m_session->contextForImport(identifierForNode(chain));
	if(contexts.length() > 0 && chain->numIdentifiers() > 0) {
		currentContext()->addImportedParentContext(
             contexts[0],
             CursorInRevision(chain->getIdentifier(0)->getLine(),
                              chain->getIdentifier(0)->getColumn()));
    }

	topContext()->updateImportsCache();
}

void ContextBuilder::visitFuncDeclaration(IFunctionDeclaration *node)
{

    // Visit the typename before opening any context
    visitTypeName(node->getReturnType());

    // NOTE: Context must be opened here(not near the block statement) because
    // parameter variables are part of the function context, not of the containing context.

    // TODO: JG: deal with missingFunctionBody
    bool mustOpenContext = node->getFunctionBody()->getSpecifiedFunctionBody() != nullptr;
    if(mustOpenContext) {
        openContext(node, editorFindRange(node->getFunctionBody(), nullptr),DUContext::ContextType::Function, node->getName());
    }

	if(node->getParameters())
	{
		for(size_t i=0; i<node->getParameters()->numParameters(); i++)
		{
			if(auto n = node->getParameters()->getParameter(i))
				visitParameter(n);
		}
	}

	if(auto n = node->getFunctionBody())
		visitBody(n, false);

    if (mustOpenContext)
        closeContext();
}

void ContextBuilder::visitConstructor(IConstructor *node)
{
	openContext(node, editorFindRange(node, node->getFunctionBody()), DUContext::Function, QualifiedIdentifier("this"));

	if(node->getParameters())
	{
		for(size_t i=0; i<node->getParameters()->numParameters(); i++)
		{
			if(auto n = node->getParameters()->getParameter(i))
				visitParameter(n);
		}
	}

	if(auto n = node->getFunctionBody()) {
		visitBody(n, false);
    }
	closeContext();
}

void ContextBuilder::visitDestructor(IDestructor *node)
{
	openContext(node, editorFindRange(node, node->getFunctionBody()), DUContext::Function, QualifiedIdentifier("~this"));
	if(auto n = node->getFunctionBody())
		visitBody(n, false);
	closeContext();
}

void ContextBuilder::visitBody(IFunctionBody *node, bool openContext)
{
    if (node->getSpecifiedFunctionBody()) {
        if(auto n = node->getSpecifiedFunctionBody()->getBlockStatement())
            visitBlockStatement(n, openContext);
    }
}

void ContextBuilder::visitBlockStatement(IBlockStatement *node, bool openContext)
{
    if (openContext) {
        ContextBuilder::openContext(node, DUContext::Other);
    }
	if(node->getDeclarationsAndStatements()) {
		visitDeclarationsAndStatements(node->getDeclarationsAndStatements());
    }
    if (openContext) {
        closeContext();
    }
}

void ContextBuilder::visitDeclarationsAndStatements(IDeclarationsAndStatements *node)
{
	for(size_t i=0; i<node->numDeclarationsAndStatements(); i++)
	{
		if(node->getDeclarationsAndStatement(i))
		{
			if(node->getDeclarationsAndStatement(i)->getDeclaration())
				visitDeclaration(node->getDeclarationsAndStatement(i)->getDeclaration());
			if(node->getDeclarationsAndStatement(i)->getStatement())
				visitStatement(node->getDeclarationsAndStatement(i)->getStatement());
		}
	}
}

void ContextBuilder::visitDeclaration(IDeclaration *node)
{

    if (auto n= node->getAliasDeclaration())
        visitAliasDeclaration(n);
	else if(auto n = node->getClassDeclaration())
		visitClassDeclaration(n);
	else if(auto n = node->getFunctionDeclaration())
		visitFuncDeclaration(n);
	else if(auto n = node->getConstructor())
		visitConstructor(n);
	else if(auto n = node->getDestructor())
		visitDestructor(n);
	else if(auto n = node->getImportDeclaration())
		visitImportDeclaration(n);
	else if(auto n = node->getStructDeclaration())
		visitStructDeclaration(n);
	else if(auto n = node->getVariableDeclaration())
		visitVarDeclaration(n);
	else if(auto n = node->getDebugSpecification())
		visitDebugSpecification(n);
	else if(auto n = node->getVersionSpecification())
		visitVersionSpecification(n);
	else if(auto n = node->getInterfaceDeclaration())
		visitInterfaceDeclaration(n);
	else if(auto n = node->getEnumDeclaration())
		visitEnumDeclaration(n);
    else if (auto n = node->getTemplateDeclaration())
        visitTemplateDeclaration(n);

    for(size_t i=0; i<node->numDeclarations(); i++)
		visitDeclaration(node->getDeclaration(i));
}

void ContextBuilder::visitAliasDeclaration(IAliasDeclaration* node)
{
    for (size_t i=0;i<node->numInitializers();i++) {
        visitAliasInitializer(node->getInitializer(i), QString::fromUtf8(node->getComment()));
    }
    // TODO: JG also deal with declaratorIdentifier list
    // TODO: JG also deal with type identifier

}

void ContextBuilder::visitAliasInitializer(IAliasInitializer* node, const QString& comment)
{
    Q_UNUSED(node);
    Q_UNUSED(comment);
}


void ContextBuilder::visitClassDeclaration(IClassDeclaration *node)
{
	if(auto n = node->getBaseClassList())
		visitBaseClassList(n);

    // open the context here
    DUChainWriteLocker lock;
    openContext(node, editorFindRange(node->getStructBody(), nullptr), DUContext::ContextType::Class, node->getName());

	if(auto n = node->getStructBody())
		visitStructBody(n);

    closeContext();
}

void ContextBuilder::visitStructDeclaration(IStructDeclaration *node)
{
    // open the context here
    DUChainWriteLocker lock;
    openContext(node, editorFindRange(node->getStructBody(), nullptr), DUContext::ContextType::Class, node->getName());

    // Contexts are opened when block statements are encountered
	if(auto n = node->getStructBody())
		visitStructBody(n);

    closeContext();
}

void ContextBuilder::visitInterfaceDeclaration(IInterfaceDeclaration *node)
{
    // open the context here
    DUChainWriteLocker lock;
    openContext(node, editorFindRange(node->getStructBody(), nullptr), DUContext::ContextType::Class, node->getName());

	if(auto n = node->getStructBody())
		visitStructBody(n);

    closeContext();
}

void ContextBuilder::visitBaseClassList(IBaseClassList *node)
{
	for(size_t i=0; i<node->numItems(); i++)
		visitBaseClass(node->getItem(i));
}

void ContextBuilder::visitBaseClass(IBaseClass *node)
{
    // TODO: JG: deal with template here too
	if(auto n = node->getType2()->getTypeIdentifierPart()->getIdentifierOrTemplateInstance())
	{
        // TODO: JG changed from visitSymbol to visitToken
        if (auto ti = n->getTemplateInstance())
            visitTemplateInstance(ti);
        else
			visitToken(n->getIdentifier());
	}
}

void ContextBuilder::visitStructBody(IStructBody *node)
{
	for(size_t i=0; i<node->numDeclarations(); i++)
	{
		if(auto n = node->getDeclaration(i))
			visitDeclaration(n);
	}
}

void ContextBuilder::visitVarDeclaration(IVariableDeclaration *node)
{
	if(node->getType())
		visitTypeName(node->getType());
	for(size_t i=0; i<node->numDeclarators(); i++)
		visitDeclarator(node->getDeclarator(i));
}

void ContextBuilder::visitParameter(IParameter *node)
{
	if(auto n = node->getType())
		visitTypeName(n);
}

void ContextBuilder::visitEnumDeclaration(IEnumDeclaration *node)
{
	openContext(node, editorFindRange(node->getEnumBody(), 0), DUContext::Enum, node->getName());
	if(auto n = node->getEnumBody())
		visitEnumBody(n);
	closeContext();
}

void ContextBuilder::visitEnumBody(IEnumBody *node)
{
	for(size_t i=0; i<node->numEnumMembers(); i++)
		visitEnumMember(node->getEnumMember(i));
}

void ContextBuilder::visitEnumMember(IEnumMember *node)
{
	if(auto n = node->getAssignExpression())
		visitExpressionNode(n);
}

void ContextBuilder::visitStatement(IStatement *node)
{
	if(auto n = node->getStatementNoCaseNoDefault())
		visitStatementNoCaseNoDefault(n);
	if(auto n = node->getCaseStatement())
		visitCaseStatement(n);
	if(auto n = node->getCaseRangeStatement())
		visitCaseRangeStatement(n);
	if(auto n = node->getDefaultStatement())
		visitDefaultStatement(n);
}

void ContextBuilder::visitStatementNoCaseNoDefault(IStatementNoCaseNoDefault *node)
{
	if(auto n = node->getExpressionStatement())
		visitExpressionStatement(n);
	if(auto n = node->getIfStatement())
		visitIfStatement(n);
	if(auto n = node->getConditionalStatement())
		visitConditionalStatement(n);
	if(auto n = node->getDebugSpecification())
		visitDebugSpecification(n);
	if(auto n = node->getVersionSpecification())
		visitVersionSpecification(n);
	if(auto n = node->getBlockStatement())
		visitBlockStatement(n, true);
	if(auto n = node->getReturnStatement())
		visitReturnStatement(n);
	if(auto n = node->getWhileStatement())
		visitWhileStatement(n);
	if(auto n = node->getForStatement())
		visitForStatement(n);
	if(auto n = node->getForeachStatement())
		visitForeachStatement(n);
	if(auto n = node->getDoStatement())
		visitDoStatement(n);
	if(auto n = node->getSwitchStatement())
		visitSwitchStatement(n);
	if(auto n = node->getFinalSwitchStatement())
		visitFinalSwitchStatement(n);
	if(auto n = node->getLabeledStatement())
		visitLabeledStatement(n);
	if(auto n = node->getBreakStatement())
		visitBreakStatement(n);
	if(auto n = node->getContinueStatement())
		visitContinueStatement(n);
	if(auto n = node->getGotoStatement())
		visitGotoStatement(n);
	if(auto n = node->getTryStatement())
		visitTryStatement(n);
	if(auto n = node->getThrowStatement())
		visitThrowStatement(n);
	if(auto n = node->getScopeGuardStatement())
		visitScopeGuardStatement(n);
	if(auto n = node->getWithStatement())
		visitWithStatement(n);
	if(auto n = node->getSynchronizedStatement())
		visitSynchronizedStatement(n);
	if(auto n = node->getStaticAssertStatement())
		visitStaticAssertStatement(n);
	if(auto n = node->getAsmStatement())
		visitAsmStatement(n);
}

void ContextBuilder::visitIfStatement(IIfStatement *node)
{
	if(node->getThenStatement())
	{
		visitExpression(node->getExpression());
		if(auto n = node->getThenStatement()->getDeclaration())
			visitDeclaration(n);
		if(auto n = node->getThenStatement()->getStatement())
			visitStatement(n);
	}
	if(node->getElseStatement())
	{
		if(auto n = node->getElseStatement()->getDeclaration())
			visitDeclaration(n);
		if(auto n = node->getElseStatement()->getStatement())
			visitStatement(n);
	}
}

void ContextBuilder::visitConditionalStatement(IConditionalStatement *node)
{
	if(auto n = node->getCompileCondition())
		visitCompileCondition(n);
	//TODO: Open context.
	if(auto n = node->getTrueStatement())
		visitDeclarationOrStatement(n);
	//TODO: Open context.
	if(auto n = node->getFalseStatement())
		visitDeclarationOrStatement(n);
}

void ContextBuilder::visitCompileCondition(ICompileCondition *node)
{
	if(auto n = node->getDebugCondition())
		visitDebugCondition(n);
	if(auto n = node->getStaticIfCondition())
		visitStaticIfCondition(n);
	if(auto n = node->getVersionCondition())
		visitVersionCondition(n);
}

void ContextBuilder::visitDebugSpecification(IDebugSpecification *node)
{
	Q_UNUSED(node)
}

void ContextBuilder::visitDebugCondition(IDebugCondition *node)
{
	if(auto n = node->getIdentifierOrInteger())
		visitToken(n);
}

void ContextBuilder::visitStaticIfCondition(IStaticIfCondition *node)
{
	if(auto n = node->getAssignExpression())
		visitExpressionNode(n);
}

void ContextBuilder::visitVersionSpecification(IVersionSpecification *node)
{
	Q_UNUSED(node)
}

void ContextBuilder::visitVersionCondition(IVersionCondition *node)
{
	if(auto n = node->getToken())
		visitToken(n);
}

void ContextBuilder::visitExpressionStatement(IExpressionStatement *node)
{
	visitExpression(node->getExpression());
}

void ContextBuilder::visitExpressionNode(IExpressionNode *node)
{
	if(!node)
		return;
	if(auto n = node->getPrimaryExpression())
		visitPrimaryExpression(n);
	else if(auto n = node->getAddExpression())
		visitAddExpression(n);
    else if (auto n = node->getMulExpression())
        visitMulExpression(n);
	else if(auto n = node->getAssignExpression())
		visitAssignExpression(n);
	else if(auto n = node->getFunctionCallExpression())
		visitFunctionCallExpression(n);
	else if(auto n = node->getUnaryExpression())
		visitUnaryExpression(n);
	else if(auto n = node->getExpression())
		visitExpression(n);
	else if(auto n = node->getCmpExpression())
		visitCmpExpression(n);
	else if(auto n = node->getRelExpression())
		visitRelExpression(n);
	else if(auto n = node->getEqualExpression())
		visitEqualExpression(n);
	else if(auto n = node->getShiftExpression())
		visitShiftExpression(n);
	else if(auto n = node->getIdentityExpression())
		visitIdentityExpression(n);
	else if(auto n = node->getInExpression())
		visitInExpression(n);
    else if (auto n = node->getTernaryExpression())
        visitTernaryExpression(n);

    //qCDebug(DUCHAIN) << "Clearing identifier chain: " << m_identifier;
    m_identifier.clear();
}

void ContextBuilder::visitExpression(IExpression *node)
{
	for(size_t i=0; i<node->numItems(); i++) {
		visitExpressionNode(node->getItem(i));
    }
}

void ContextBuilder::visitInExpression(IInExpression *node)
{
	visitExpressionNode(node->getLeft());
	visitExpressionNode(node->getRight());
}

void ContextBuilder::visitIdentityExpression(IIdentityExpression *node)
{
	visitExpressionNode(node->getLeft());
	visitExpressionNode(node->getRight());
}

void ContextBuilder::visitShiftExpression(IShiftExpression *node)
{
	visitExpressionNode(node->getLeft());
	visitExpressionNode(node->getRight());
}

void ContextBuilder::visitEqualExpression(IEqualExpression *node)
{
	visitExpressionNode(node->getLeft());
	visitExpressionNode(node->getRight());
}

void ContextBuilder::visitRelExpression(IRelExpression *node)
{
	visitExpressionNode(node->getLeft());
	visitExpressionNode(node->getRight());
}

void ContextBuilder::visitCmpExpression(ICmpExpression *node)
{
	visitExpressionNode(node->getShiftExpression());
	visitExpressionNode(node->getEqualExpression());
	visitExpressionNode(node->getIdentityExpression());
	visitExpressionNode(node->getRelExpression());
	visitExpressionNode(node->getInExpression());
}

void ContextBuilder::visitPrimaryExpression(IPrimaryExpression *node)
{
    // TODO: JG primary expressions have many more children
    if (auto n = node->getIdentifierOrTemplateInstance())
        visitIdentifierOrTemplateInstance(n);
}

void ContextBuilder::visitIdentifierOrTemplateInstance(IIdentifierOrTemplateInstance* node )
{
    if (auto n = node->getTemplateInstance())
        visitTemplateInstance(n);
    // TODO: JG fixe
//    if (auto n = node->getIdentifier())
//        visitIdentifier(n);
}

void ContextBuilder::visitTemplateDeclaration ( ITemplateDeclaration* node )
{
    // NOTE: Context must be opened here, because parameter variables
    // are part of the template context, not of the containing context.
    openContext(node, editorFindRange(node, nullptr),DUContext::Template, node->getName());

    // template parameters
    if (auto tp = node->getTemplateParameters()) {
        if (auto tpl = tp->getTemplateParameterList()) {
            for(size_t i =0 ;i<tpl->numItems();i++) {
                visitTemplateParameter(tpl->getItem(i));
            }
        }
    }
    for(size_t i=0; i<node->numDeclarations();i++) {
        visitDeclaration(node->getDeclaration(i));
    }

	closeContext();
}

void ContextBuilder::visitTemplateParameter(ITemplateParameter* node)
{
    Q_UNUSED(node);
}

void ContextBuilder::visitTemplateInstance ( ITemplateInstance* node )
{
    // TODO: JG fix this
//    visitIdentifier(node->getIdentifier());

    // TODO: JG implement template arguments
//     if (auto n = node->getTemplateArguments())
//         visitTemplateArguments(n);

}

// TODO: JG Remove
// void ContextBuilder::visitIdentifier(IToken* node )
// {
//     identifierChain.append(QString::fromUtf8(node->getText()));
// }

void ContextBuilder::visitTernaryExpression(ITernaryExpression* node)
{

    // TODO: JG Implement this
   Q_UNUSED(node);
}


void ContextBuilder::visitAddExpression(IAddExpression *node)
{
	if(auto n = node->getLeft())
		visitExpressionNode(n);

	if(auto n = node->getRight())
		visitExpressionNode(n);
}

void ContextBuilder::visitUnaryExpression(IUnaryExpression *node)
{
	if(auto n = node->getPrimaryExpression())
		visitPrimaryExpression(n);
	else if(auto n = node->getFunctionCallExpression())
		visitFunctionCallExpression(n);
	else if(auto n = node->getUnaryExpression())
		visitUnaryExpression(n);
	else if(auto n = node->getAssertExpression())
		visitAssertExpression(n);
    else if (auto n = node->getIndexExpression())
        visitIndexExpression(n);
    else if (auto n= node->getNewExpression())
        visitTypeName(n->getType());
}

void ContextBuilder::visitAssignExpression(IAssignExpression *node)
{
	if(auto n = node->getExpression())
		visitExpressionNode(n);
	if(auto n = node->getTernaryExpression())
		visitExpressionNode(n);
}

void ContextBuilder::visitIndexExpression ( IIndexExpression* node )
{
    if (auto n = node->getUnaryExpression())
        visitUnaryExpression(n);
    for(size_t i=0; i<node->numIndexes(); i++)
        visitIndex(node->getIndex(i));
}

void ContextBuilder::visitIndex ( IIndex* node )
{
    if (auto n = node->getLow())
        visitExpressionNode(n);
    if (auto n = node->getHigh())
        visitExpressionNode(n);

}

void ContextBuilder::visitDeclarator(IDeclarator *node)
{
	if(auto n = node->getInitializer())
		visitInitializer(n);
}

void ContextBuilder::visitInitializer(IInitializer *node)
{
	if(!node->getNonVoidInitializer())
		return;
	if(auto n = node->getNonVoidInitializer()->getAssignExpression())
		visitExpressionNode(n);
}

void ContextBuilder::visitImportDeclaration(IImportDeclaration *node)
{
	for(size_t i=0; i<node->numSingleImports(); i++)
	{
		if(auto n = node->getSingleImport(i))
			visitSingleImport(n);
	}
	if (auto n = node->getImportBindings())
    {
        if (auto n2 = n->getSingleImport())
            visitSingleImport(n2);
        for(size_t i=0;i<n->numImportBinds();i++)
            visitImportBind(n->getImportBind(i));
    }
}

void ContextBuilder::visitImportBind ( IImportBind* node )
{
    Q_UNUSED(node);
//     qCDebug(DUCHAIN) << "visitImportBind() not implemented";
}


void ContextBuilder::visitFunctionCallExpression(IFunctionCallExpression *node)
{
	if(auto n = node->getUnaryExpression())
		visitUnaryExpression(n);
    m_identifier.clear();
	if(auto n = node->getType())
		visitTypeName(n);
	if(auto n = node->getArguments())
		visitArguments(n);
}

void ContextBuilder::visitArguments(IArguments *node)
{
	auto list = node->getArgumentList();
	if(!list)
		return;
	for(size_t i=0; i<list->numItems(); i++) {
		visitExpressionNode(list->getItem(i));
    }
}

void ContextBuilder::visitReturnStatement(IReturnStatement *node)
{
	if(auto n = node->getExpression())
		visitExpression(n);
}

void ContextBuilder::visitWhileStatement(IWhileStatement *node)
{
	ContextBuilder::openContext(node, DUContext::Other);
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
	if(auto n = node->getExpression())
		visitExpression(n);
	closeContext();
}

void ContextBuilder::visitForStatement(IForStatement *node)
{
	ContextBuilder::openContext(node, DUContext::Other);
	if(auto n = node->getInitialization())
		visitDeclarationOrStatement(n);
	if(auto n = node->getTest())
		visitExpression(n);
	if(auto n = node->getIncrement())
		visitExpression(n);
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
	closeContext();
}

void ContextBuilder::visitForeachStatement(IForeachStatement *node)
{
    // TODO: JG adjust to new foreach style with indexer
    // also add foreach_reverse by accessing the token!
	ContextBuilder::openContext(node, DUContext::Other);
	if(auto n = node->getForeachType())
		visitForeachType(n);
	if(auto n = node->getForeachTypeList())
	{
		for(size_t i=0; i<n->numItems(); i++)
			visitForeachType(n->getItem(i));
	}
	if(auto n = node->getLow()) // the expression, or the low side of range
		visitExpression(n);
	if(auto n = node->getHigh()) // high side of range
		visitExpression(n);
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
	closeContext();
}

void ContextBuilder::visitDeclarationOrStatement(IDeclarationOrStatement *node)
{
	if(auto n = node->getDeclaration())
		visitDeclaration(n);
	if(auto n = node->getStatement())
		visitStatement(n);
}

void ContextBuilder::visitDoStatement(IDoStatement *node)
{
	//Do.
	ContextBuilder::openContext(node, DUContext::Other);
	if(auto n = node->getStatementNoCaseNoDefault())
		visitStatementNoCaseNoDefault(n);
	closeContext();
	//While.
	if(auto n = node->getExpression())
		visitExpression(n);
}

void ContextBuilder::visitSwitchStatement(ISwitchStatement *node)
{
	if(auto n = node->getExpression())
		visitExpression(n);
	if(auto n = node->getStatement())
		visitStatement(n);
}

void ContextBuilder::visitFinalSwitchStatement(IFinalSwitchStatement *node)
{
	if(auto n = node->getSwitchStatement())
		visitSwitchStatement(n);
}

void ContextBuilder::visitCaseStatement(ICaseStatement *node)
{
	for(size_t i=0; i<node->getArgumentList()->numItems(); i++)
		visitExpressionNode(node->getArgumentList()->getItem(i));
	if(auto n = node->getDeclarationsAndStatements())
		visitDeclarationsAndStatements(n);
}

void ContextBuilder::visitCaseRangeStatement(ICaseRangeStatement *node)
{
	if(auto n = node->getLow())
		visitExpressionNode(n);
	if(auto n = node->getHigh())
		visitExpressionNode(n);
	if(auto n = node->getDeclarationsAndStatements())
		visitDeclarationsAndStatements(n);
}

void ContextBuilder::visitDefaultStatement(IDefaultStatement *node)
{
	if(auto n = node->getDeclarationsAndStatements())
		visitDeclarationsAndStatements(n);
}

void ContextBuilder::visitLabeledStatement(ILabeledStatement *node)
{
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
}

void ContextBuilder::visitBreakStatement(IBreakStatement *node)
{
	if(auto n = node->getLabel())
		visitToken(n);
}

void ContextBuilder::visitContinueStatement(IContinueStatement *node)
{
	if(auto n = node->getLabel())
		visitToken(n);
}

void ContextBuilder::visitGotoStatement(IGotoStatement *node)
{
	if(auto n = node->getLabel())
		visitToken(n);
	if(auto n = node->getExpression())
		visitExpression(n);
}

void ContextBuilder::visitTryStatement(ITryStatement *node)
{
	//TODO: Open context.
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
	if(auto n = node->getCatches())
	{
		for(size_t i=0; i<n->numCatches(); i++)
			visitCatch(n->getCatche(i));
		if(auto c = n->getLastCatch())
			visitLastCatch(c);
	}
	if(auto n = node->getFinally())
		visitFinally(n);
}

void ContextBuilder::visitCatch(ICatch *node)
{
	//TODO: JG Open context.
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
	if(auto n = node->getType())
		visitTypeName(n);
}

void ContextBuilder::visitLastCatch(ILastCatch *node)
{
	//TODO: JG Open context.
	if(auto n = node->getStatementNoCaseNoDefault())
		visitStatementNoCaseNoDefault(n);
}

void ContextBuilder::visitFinally(IFinally *node)
{
	//TODO: Open context.
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
}

void ContextBuilder::visitThrowStatement(IThrowStatement *node)
{
	if(auto n = node->getExpression())
		visitExpression(n);
}

void ContextBuilder::visitScopeGuardStatement(IScopeGuardStatement *node)
{
	//TODO: Open context.
	if(auto n = node->getStatementNoCaseNoDefault())
		visitStatementNoCaseNoDefault(n);
}

void ContextBuilder::visitWithStatement(IWithStatement *node)
{
	//TODO: Use expression for namespace.
	if(auto n = node->getExpression())
		visitExpression(n);
	//TODO: Open context.
	if(auto n = node->getDeclarationOrStatement())
		visitDeclarationOrStatement(n);
}

void ContextBuilder::visitSynchronizedStatement(ISynchronizedStatement *node)
{
	if(auto n = node->getExpression())
		visitExpression(n);
	//TODO: Open context.
	if(auto n = node->getStatementNoCaseNoDefault())
		visitStatementNoCaseNoDefault(n);
}

void ContextBuilder::visitStaticAssertStatement(IStaticAssertStatement *node)
{
	if(auto n = node->getAssertExpression())
		visitAssertExpression(n);
}

void ContextBuilder::visitAssertExpression(IAssertExpression *node)
{
	if (auto n = node->getAssertArguments())
        visitAssertArguments(n);
}

void ContextBuilder::visitAssertArguments(IAssertArguments *node)
{
    if(auto n = node->getAssertion())
		visitExpressionNode(n);
	if(auto n = node->getMessage())
		visitExpressionNode(n);
}

void ContextBuilder::visitAsmStatement(IAsmStatement *node)
{
	//TODO: Can we do anything for asm support?
	Q_UNUSED(node)
}

void ContextBuilder::visitToken(IToken *node)
{
	Q_UNUSED(node)
}

void ContextBuilder::visitForeachType(IForeachType *node)
{
	if(auto n = node->getType())
		visitTypeName(n);
}
