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

#include <language/duchain/classdeclaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>

#include "helper.h"
#include "duchaindebug.h"
#include "ddeclaration.h"
#include "dclassfunctiondeclaration.h"

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

void DeclarationBuilder::visitAliasInitializer(IAliasInitializer* node, const QString& comment)
{
    QualifiedIdentifier identifier = identifierForNode(node->getName());
    visitTypeName(node->getType());
    AbstractType::Ptr type = getLastType();

    DUChainWriteLocker lock;
    Declaration* aliasDecl = openDeclaration<Declaration>(identifier,editorFindRange(node->getName(),nullptr));
    aliasDecl->setKind(Declaration::Type);
    aliasDecl->setAbstractType(type);
    aliasDecl->setComment(comment);
    aliasDecl->setIsTypeAlias(true);
    closeDeclaration();

}


void DeclarationBuilder::visitDeclaration(IDeclaration* node)
{
    m_visibility = Visibility::Public; // this is the default in D

    for (size_t i =0;i<node->numAttributes();i++) {
        if (auto t = node->getAttribute(i)->getAttribute()) {
            if (QStringLiteral("protected") == t->getType())
                m_visibility = Visibility::Protected;
            else if (QStringLiteral("private") == t->getType())
                m_visibility = Visibility::Private;
        }
    }

    DeclarationBuilderBase::visitDeclaration(node);

}

void DeclarationBuilder::visitVarDeclaration(IVariableDeclaration *node)
{
	DeclarationBuilderBase::visitVarDeclaration(node);
	for(size_t i=0; i<node->numDeclarators(); i++) {
		declareVariable(node->getDeclarator(i), lastType());
    }
}

void DeclarationBuilder::declareVariable(IDeclarator* declarator, const AbstractType::Ptr &type)
{
    IToken *id = declarator->getName();
	DUChainWriteLocker lock;
	Declaration *dec = openDefinition<Declaration>(identifierForNode(id), editorFindRange(id, id));
	dec->setType(type);
	dec->setKind(Declaration::Instance);
    if (auto c = declarator->getComment())
        dec->setComment(QString::fromUtf8(c));
    closeDeclaration();
}

void DeclarationBuilder::visitClassDeclaration(IClassDeclaration *node)
{
	inClassScope = true;
	DeclarationBuilderBase::visitClassDeclaration(node);
	if(node->getComment())
		setComment(node->getComment());
	DUChainWriteLocker lock;
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), nullptr));
    currentStructureType->setDeclaration(dec);
	dec->setType(currentStructureType);
	dec->setKind(Declaration::Type);
    dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Class);
    if (m_visibility == Visibility::Protected)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Protected);
    else if (m_visibility == Visibility::Private)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Private);
    else {
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Public);
    }
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
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), nullptr));
    currentStructureType->setDeclaration(dec);
	dec->setType(currentStructureType);
	dec->setKind(Declaration::Type);
	dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Struct);
    if (m_visibility == Visibility::Protected)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Protected);
    else if (m_visibility == Visibility::Private)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Private);
    else {
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Public);
    }
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
    DDeclaration* dec = openDefinition<DDeclaration>(identifierForNode(node->getName()),editorFindRange(node->getName(),nullptr));
 	dec->setDKind(DDeclaration::Kind::Template);
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
	ClassDeclaration *dec = openDefinition<ClassDeclaration>(identifierForNode(node->getName()), editorFindRange(node->getName(), nullptr));
	dec->setKind(Declaration::Type);
    // TODO: JG set access visibility
    //dec->setAccessPolicy();
    currentStructureType->setDeclaration(dec);
    dec->setType(currentStructureType);
	dec->setInternalContext(lastContext());
	dec->setClassType(ClassDeclarationData::Interface);
    if (m_visibility == Visibility::Protected)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Protected);
    else if (m_visibility == Visibility::Private)
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Private);
    else {
        dec->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Public);
    }
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
        if (m_visibility == Visibility::Protected)
            newMethod->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Protected);
        else if (m_visibility == Visibility::Private)
            newMethod->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Private);
        else
            newMethod->setAccessPolicy(ClassFunctionDeclaration::AccessPolicy::Public);
        // TODO: JG also consider, final and abstract
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
	DClassFunctionDeclaration *newMethod = openDefinition<DClassFunctionDeclaration>(QualifiedIdentifier("this"), editorFindRange(node, node));
	if(node->getComment())
		newMethod->setComment(QString::fromUtf8(node->getComment()));
	newMethod->setKind(Declaration::Type);
	lock.unlock();
	ContextBuilder::visitConstructor(node);
	lock.lock();
	closeDeclaration();
	newMethod->setInternalContext(lastContext());
	newMethod->setType(currentFunctionType);
    newMethod->setMethodType(DClassFunctionDeclaration::MethodType::Constructor);
}

void DeclarationBuilder::visitDestructor(IDestructor *node)
{
	TypeBuilder::visitDestructor(node);
	DUChainWriteLocker lock;
	DClassFunctionDeclaration *newMethod = openDefinition<DClassFunctionDeclaration>(QualifiedIdentifier("~this"), editorFindRange(node, node));
	if(node->getComment())
		newMethod->setComment(QString::fromUtf8(node->getComment()));
	newMethod->setKind(Declaration::Type);
	lock.unlock();
	ContextBuilder::visitDestructor(node);
	lock.lock();
	closeDeclaration();
	newMethod->setInternalContext(lastContext());
	newMethod->setType(currentFunctionType);
    newMethod->setMethodType(DClassFunctionDeclaration::MethodType::Destructor);

}

void DeclarationBuilder::visitSingleImport(ISingleImport *node)
{
	DeclarationBuilderBase::visitSingleImport(node);
	DUChainWriteLocker lock;
	QualifiedIdentifier importId = identifierForNode(node->getIdentifierChain());

    NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(globalImportIdentifier(), editorFindRange(node->getIdentifierChain(), nullptr));
    decl->setImportIdentifier(importId);
    decl->setKind(Declaration::NamespaceAlias);

	closeDeclaration();
}

void DeclarationBuilder::visitModule(IModule *node)
{
    Declaration* packageDeclaration = nullptr;

	if(auto moduleDeclaration = node->getModuleDeclaration())
	{
		if(moduleDeclaration->getComment())
			setComment(moduleDeclaration->getComment());

        DUChainWriteLocker lock;

        auto m_thisPackage = identifierForNode(moduleDeclaration->getModuleName());
		RangeInRevision range = editorFindRange(moduleDeclaration->getModuleName(), moduleDeclaration->getModuleName());

        Identifier localId;
        if (!m_thisPackage.isEmpty())
            localId = m_thisPackage.last();
        else
            qCDebug(DUCHAIN) << "visitModule::openDeclaration() called without identifier";

        qCDebug(DUCHAIN) << "Localid: " << localId << " packageid: " << m_thisPackage;
// 		packageDeclaration = openDeclaration<DDeclaration>(localId, range);
// 		packageDeclaration->setDKind(DDeclaration::Kind::Module);
        packageDeclaration = openDeclaration<Declaration>(localId,editorFindRange(moduleDeclaration->getModuleName(),nullptr));
        packageDeclaration->setKind(Declaration::Namespace);
        // Always open a context here
		openContext(node, editorFindRange(node, nullptr), DUContext::Namespace, m_thisPackage);

        packageDeclaration->setInternalContext(currentContext());
    }

    // always visit: Modules /do/ require a module statement
    // but if omitted further visiting will crash because of a missing a context.
    DeclarationBuilderBase::visitModule(node);


    if(node->getModuleDeclaration())
    {
        closeContext();
        closeDeclaration();
    }
    topContext()->updateImportsCache();

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
