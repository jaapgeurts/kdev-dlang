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

#include "declarationbuilder.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/templatedeclaration.h>
#include <language/duchain/duchainutils.h>

#include "helper.h"
#include "duchaindebug.h"


using namespace KDevelop;


DeclarationBuilder::DeclarationBuilder(ParseSession *session, bool forExport) : m_export(forExport), inClassScope(false), m_preBuilding(false), m_ownPriority(0)
{
	setParseSession(session);
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString &url, INode *node, const ReferencedTopDUContext& updateContext)
{
    ReferencedTopDUContext updateContext2 = updateContext;
	qCDebug(DUCHAIN) << "DeclarationBuilder start";
	if(!m_preBuilding)
	{
		qCDebug(DUCHAIN) << "Running prebuilder";
		DeclarationBuilder preBuilder(m_session, m_export);
		preBuilder.m_preBuilding = true;
		updateContext2 = preBuilder.build(url, node, updateContext);
	}
	return DeclarationBuilderBase::build(url, node, updateContext2);
}

void DeclarationBuilder::startVisiting(INode *node)
{
	{
		DUChainWriteLocker lock;
		topContext()->clearImportedParentContexts();
		topContext()->updateImportsCache();
	}

	return DeclarationBuilderBase::startVisiting(node);
}

void DeclarationBuilder::visitVarDeclaration(IVariableDeclaration *node)
{
	DeclarationBuilderBase::visitVarDeclaration(node);
	for(size_t i=0; i<node->numDeclarators(); i++)
		declareVariable(node->getDeclarator(i)->getName(), lastType());
}

void DeclarationBuilder::declareVariable(IToken *id, const AbstractType::Ptr &type)
{
	DUChainWriteLocker lock;
	Declaration *dec = openDefinition<Declaration>(identifierForNode(id), editorFindRange(id, id));
	dec->setType(type);
	dec->setKind(Declaration::Instance);
	closeDeclaration();
}

void DeclarationBuilder::visitClassDeclaration(IClassDeclaration *node)
{
	inClassScope = true;
	DeclarationBuilderBase::visitClassDeclaration(node);
	if(node->getComment())
		setComment(node->getComment());
	DUChainWriteLocker lock;
    // TODO: JG: class declarations don't have correct name
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), 0));
	dec->setType(lastType());
	dec->setKind(Declaration::Type);
	dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Class);
	closeDeclaration();
	inClassScope = false;
}

void DeclarationBuilder::visitStructDeclaration(IStructDeclaration *node)
{
	inClassScope = true;
	DeclarationBuilderBase::visitStructDeclaration(node);
	if(node->getComment())
		setComment(node->getComment());
	DUChainWriteLocker lock;
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), 0));
	dec->setType(lastType());
	dec->setKind(Declaration::Type);
	dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Struct);
	closeDeclaration();
	inClassScope = false;
}

void DeclarationBuilder::visitTemplateDeclaration ( ITemplateDeclaration* node )
{
    // TODO: JG remember template in template scope
    // inTemplateScope = true;
    DeclarationBuilderBase::visitTemplateDeclaration(node);
    if(node->getComment())
		setComment(node->getComment());
    DUChainWriteLocker lock;
    TemplateDeclaration* dec = openDefinition<TemplateDeclaration>(identifierForNode(node->getName()),editorFindRange(node->getName(),0));
	dec->setKind(Declaration::Template);
	dec->setInternalContext(lastContext());
	closeDeclaration();
    // inTemplateScope = false;

}

void DeclarationBuilder::visitTemplateParameter(ITemplateParameter* node)
{
    if (auto n = node->getTemplateTypeParameter()) {
        DUChainWriteLocker lock;
        Declaration *parameter = openDeclaration<Declaration>(n->getIdentifier(), node);
        parameter->setKind(Declaration::Instance);
        // TODO: JG template parameters have no type
//         parameter->setAbstractType(lastType());
        closeDeclaration();
    }
    if (auto n = node->getTemplateAliasParameter()) {
        qCDebug(DUCHAIN) << "Unhandled template alias parameter";
    }
    if (auto n = node->getTemplateThisParameter()) {
        qCDebug(DUCHAIN) << "Unhandled template this parameter";
    }
    if (auto n = node->getTemplateTupleParameter()) {
        qCDebug(DUCHAIN) << "Unhandled template tuple parameter";
    }
    if (auto n = node->getTemplateValueParameter()) {
        qCDebug(DUCHAIN) << "Unhandled template value parameter";
    }
}


void DeclarationBuilder::visitInterfaceDeclaration(IInterfaceDeclaration *node)
{
	inClassScope = true;
	DeclarationBuilderBase::visitInterfaceDeclaration(node);
	if(node->getComment())
		setComment(node->getComment());
	DUChainWriteLocker lock;
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), 0));
	dec->setType(lastType());
	dec->setKind(Declaration::Type);
	dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Interface);
	closeDeclaration();
	inClassScope = false;
}

void DeclarationBuilder::visitParameter(IParameter *node)
{
	TypeBuilder::visitParameter(node);
	DUChainWriteLocker lock;
	Declaration *parameter = openDeclaration<Declaration>(node->getName(), node);
	parameter->setKind(Declaration::Instance);
	parameter->setAbstractType(lastType());
	closeDeclaration();
}

void DeclarationBuilder::visitFuncDeclaration(IFunctionDeclaration *node)
{
	TypeBuilder::visitFuncDeclaration(node);
	DUChainWriteLocker lock;
	if(inClassScope)
	{
		ClassFunctionDeclaration *newMethod = openDefinition<ClassFunctionDeclaration>(node->getName(), node);
		if(node->getComment())
			newMethod->setComment(QString::fromUtf8(node->getComment()));
		newMethod->setKind(Declaration::Type);
		lock.unlock();
		ContextBuilder::visitFuncDeclaration(node);
		lock.lock();
		closeDeclaration();
		newMethod->setInternalContext(lastContext());
		newMethod->setType(currentFunctionType);
	}
	else
	{
		FunctionDeclaration *newMethod = openDefinition<FunctionDeclaration>(node->getName(), node);
		if(node->getComment())
			newMethod->setComment(QString::fromUtf8(node->getComment()));
		newMethod->setKind(Declaration::Type);
		lock.unlock();
		ContextBuilder::visitFuncDeclaration(node);
		lock.lock();
		closeDeclaration();
		newMethod->setInternalContext(lastContext());
		newMethod->setType(currentFunctionType);
	}
}

void DeclarationBuilder::visitConstructor(IConstructor *node)
{
	TypeBuilder::visitConstructor(node);
	DUChainWriteLocker lock;
	ClassFunctionDeclaration *newMethod = openDefinition<ClassFunctionDeclaration>(QualifiedIdentifier("this"), editorFindRange(node, node));
	if(node->getComment())
		newMethod->setComment(QString::fromUtf8(node->getComment()));
	newMethod->setKind(Declaration::Type);
	lock.unlock();
	ContextBuilder::visitConstructor(node);
	lock.lock();
	closeDeclaration();
	newMethod->setInternalContext(lastContext());
	newMethod->setType(currentFunctionType);
}

void DeclarationBuilder::visitDestructor(IDestructor *node)
{
	TypeBuilder::visitDestructor(node);
	DUChainWriteLocker lock;
	ClassFunctionDeclaration *newMethod = openDefinition<ClassFunctionDeclaration>(QualifiedIdentifier("~this"), editorFindRange(node, node));
	if(node->getComment())
		newMethod->setComment(QString::fromUtf8(node->getComment()));
	newMethod->setKind(Declaration::Type);
	lock.unlock();
	ContextBuilder::visitDestructor(node);
	lock.lock();
	closeDeclaration();
	newMethod->setInternalContext(lastContext());
	newMethod->setType(currentFunctionType);
}

void DeclarationBuilder::visitSingleImport(ISingleImport *node)
{
	DUChainWriteLocker lock;
	QualifiedIdentifier import = identifierForNode(node->getIdentifierChain());
    // TODO: JG consider making ImportDeclaration class
	NamespaceAliasDeclaration *importDecl = openDefinition<NamespaceAliasDeclaration>(QualifiedIdentifier(globalImportIdentifier()), editorFindRange(node->getIdentifierChain(), 0));
	importDecl->setImportIdentifier(import);
    // TODO: JG should be an import kind
    //importDecl->setKind(Declaration::Import);
	closeDeclaration();
	DeclarationBuilderBase::visitSingleImport(node);
}

void DeclarationBuilder::visitModule(IModule *node)
{
    // Always open a context here. Modules /do/ require a module statement
    // but if omitted an AST will still be generated and parsing will crash
    // because there is no context.
	if(node->getModuleDeclaration())
	{
		if(node->getModuleDeclaration()->getComment())
			setComment(node->getModuleDeclaration()->getComment());

        DUChainWriteLocker lock;

        auto m_thisPackage = identifierForNode(node->getModuleDeclaration()->getModuleName());
		RangeInRevision range = editorFindRange(node->getModuleDeclaration()->getModuleName(), node->getModuleDeclaration()->getModuleName());

        Identifier localId;
        if (!m_thisPackage.isEmpty())
            localId = m_thisPackage.last();
        else
            qCDebug(DUCHAIN) << "openDeclaration() called without identifier";
		Declaration *packageDeclaration = openDeclaration<Declaration>(localId, range);
		packageDeclaration->setKind(Declaration::Namespace);

		openContext(node, editorFindRange(node, 0), DUContext::Namespace, m_thisPackage);
		packageDeclaration->setInternalContext(currentContext());
		lock.unlock();
    }

    // TODO: JG this should be inside if statement. If it's inside it will
    // crash during buildUses because currentContext() is nullptr. Figure out why
    DeclarationBuilderBase::visitModule(node);

    if(node->getModuleDeclaration())
    {
        closeContext();
        closeDeclaration();
        topContext()->updateImportsCache();
    }


}

void DeclarationBuilder::visitForeachType(IForeachType *node)
{
	TypeBuilder::visitForeachType(node);
	DUChainWriteLocker lock;
	Declaration *argument = openDeclaration<Declaration>(node->getIdentifier(), node);
	argument->setKind(Declaration::Instance);
	argument->setAbstractType(lastType());
	closeDeclaration();
}

void DeclarationBuilder::visitLabeledStatement(ILabeledStatement *node)
{
	DeclarationBuilderBase::visitLabeledStatement(node);
	DUChainWriteLocker lock;
	Declaration *label = openDeclaration<Declaration>(node->getIdentifier(), node);
	label->setKind(Declaration::Type);
	closeDeclaration();
}

void DeclarationBuilder::visitDebugSpecification(IDebugSpecification *node)
{
	DeclarationBuilderBase::visitDebugSpecification(node);
	DUChainWriteLocker lock;
	Declaration *label = openDeclaration<Declaration>(node->getIdentifierOrInteger(), node);
	label->setKind(Declaration::Type);
	closeDeclaration();
}

void DeclarationBuilder::visitVersionSpecification(IVersionSpecification *node)
{
	DeclarationBuilderBase::visitVersionSpecification(node);
	DUChainWriteLocker lock;
	Declaration *label = openDeclaration<Declaration>(node->getToken(), node);
	label->setKind(Declaration::Type);
	closeDeclaration();
}

void DeclarationBuilder::visitCatch(ICatch *node)
{
	TypeBuilder::visitCatch(node);
	DUChainWriteLocker lock;
	Declaration *parameter = openDeclaration<Declaration>(node->getIdentifier(), node);
	parameter->setKind(Declaration::Instance);
	parameter->setAbstractType(lastType());
	closeDeclaration();
}

void DeclarationBuilder::visitEnumDeclaration(IEnumDeclaration *node)
{
	DUChainWriteLocker lock;
	Declaration *e = openDeclaration<Declaration>(node->getName(), node);
	e->setKind(Declaration::Type);
	lock.unlock();
	DeclarationBuilderBase::visitEnumDeclaration(node);
	lock.lock();
	e->setInternalContext(lastContext());
	//e->setAbstractType(lastType());
	closeDeclaration();
}

void DeclarationBuilder::visitEnumMember(IEnumMember *node)
{
	DeclarationBuilderBase::visitEnumMember(node);
	DUChainWriteLocker lock;
	ClassMemberDeclaration *e = openDeclaration<ClassMemberDeclaration>(node->getName(), node);
	e->setStatic(true);
	EnumeratorType::Ptr enumeratorType = lastType().cast<EnumeratorType>();
	if(enumeratorType)
	{
		enumeratorType->setDeclaration(e);
		e->setAbstractType(enumeratorType.cast<AbstractType>());
	}
	closeDeclaration();
}
