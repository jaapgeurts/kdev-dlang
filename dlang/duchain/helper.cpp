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

#include "helper.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainregister.h>
#include <language/duchain/types/structuretype.h>

#include <QReadLocker>
#include <QProcess>

#include "builders/ddeclaration.h"
#include "duchaindebug.h"


void Helper::registerDUChainItems() {

    duchainRegisterType<DDeclaration>();

    // TODO: JG cleanup
    // Copied from Clang support. D doesn't need this.
//    TypeSystem::self().registerTypeClass<ClassSpecializationType, ClassSpecializationTypeData>();

}


void Helper::unregisterDUChainItems()
{
    // TODO: JG cleanup
    // Copied from Clang support. D doesn't need this.
//     TypeSystem::self().unregisterTypeClass<ClassSpecializationType, ClassSpecializationTypeData>();

    /// FIXME: this is currently not supported by the DUChain code...
    /// When the items are unregistered on plugin destruction, we'll get hit by
    /// assertions later on when the DUChain is finalized. There, when the data is getting cleaned up,
    /// we try to load all kinds of items again which would fail to find our items if we unregister.
    /// So let's not do it...
/*
    duchainUnregisterType<TemplateDeclaration>();

*/
}


DeclarationPointer resolveTypeDeclarationForIdentifier(QualifiedIdentifier newIdentifier, DUContext *context)
{
    DeclarationPointer typeDecl = getTypeOrVarDeclaration(newIdentifier,context);
    if (typeDecl) {
//         qCDebug(DUCHAIN) << "Decl found: " << typeDecl->toString();
        StructureType::Ptr type = typeDecl->type<StructureType>();
        if (type) {
            Declaration* decl = type->declaration(nullptr);
            if (decl) {
                return DeclarationPointer(decl);
            } else {
                qCDebug(DUCHAIN) << "Type found but declaration not found: " << type->toString();
            }
        } else {
                qCDebug(DUCHAIN) << "Type not found";
        }
    } else {
        qCDebug(DUCHAIN) << "Type decl not found; check to see if it's an import decl";
//         typeDecl = context->findDeclarations(newIdentifier,)
    }
    return DeclarationPointer();
}

DeclarationPointer getDeclaration(QualifiedIdentifier id, DUContext *context, bool searchInParent)
{
    // TODO: jg make it search in parent contexts and resolve types
    Q_UNUSED(searchInParent);
//    qCDebug(DUCHAIN) << "Looking for decl: " << id;
    if (id.count() == 0) {
        qCDebug(DUCHAIN) << "ERROR: lookup for empty declaration requested." << id;
        return DeclarationPointer();
    }

    if(context)
	{
        // Try to find the full qualifiedidentifier
        DUChainReadLocker lock;
        QList<Declaration*> declarations = context->findDeclarations(id,CursorInRevision(INT_MAX,INT_MAX));
        for(Declaration *decl : declarations)
            // TODO: JG duplicated code
        {
            //Import declarations are just decorations and need not be returned.
            if(decl->kind() == Declaration::Import)
                continue;
            return DeclarationPointer(decl);
        }
        // Not found? Try to resolve identifier to type names
        QualifiedIdentifier newIdentifier;
        newIdentifier.push(id.at(0)); // start with the left part first
        // Find the typename declaration
        int maxQualifiers = id.count();
        for (int i=1;i<maxQualifiers;i++) {
            // replace variable names with type names
//             qCDebug(DUCHAIN) << "getType:: newIdentifier: " << newIdentifier;
            DeclarationPointer typeDecl = resolveTypeDeclarationForIdentifier(newIdentifier,context);
            if (typeDecl) {
                // Found
                newIdentifier.clear();
                newIdentifier.push(typeDecl->identifier());
            } else {
                // not found
                qCDebug(DUCHAIN) << "Couldn't resolve type for identifier " << newIdentifier;
                break;
            }
            newIdentifier.push(id.at(i));
        }
//         qCDebug(DUCHAIN) << "getDecl:: newIdentifier: " << newIdentifier;
        // find the declaration that we need
        declarations = context->findDeclarations(newIdentifier,CursorInRevision(INT_MAX,INT_MAX));
        for(Declaration *decl : declarations)
        {
            // TODO: JG duplicated code
            //Import declarations are just decorations and need not be returned.
            if(decl->kind() == Declaration::Import)
                continue;
            return DeclarationPointer(decl);
        }

	}
	return DeclarationPointer();
}

DeclarationPointer getTypeOrVarDeclaration(QualifiedIdentifier id, DUContext *context, bool searchInParent)
{
    Q_UNUSED(searchInParent);
	DUChainReadLocker lock;
	if(context)
	{
		auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
		for(Declaration *decl : declarations)
		{
			if(decl->kind() == Declaration::Import || decl->kind() == Declaration::Namespace || decl->kind() == Declaration::NamespaceAlias)
				continue;
			return DeclarationPointer(decl);
		}
	}
	return DeclarationPointer();
}

Declaration* searchUp(DUContext* context, const QualifiedIdentifier& identifier,
                                         const CursorInRevision& position = CursorInRevision::invalid()) {

    Declaration* decl;
    if (context == nullptr)
        return nullptr;

    QVector<QPair<Declaration*, int>> alldecls = context->allDeclarations(position, context->topContext());
    // search upwards
    for(const QPair<Declaration*, int>& pair : alldecls) {
            qCDebug(DUCHAIN) << "KK" << pair.first->toString() << "Depth:" << pair.second;
            // if found == return pair.first
    }
    return searchUp(context->parentContext(),identifier,position);
}


Declaration* searchDown(DUContext* context, const QualifiedIdentifier& identifier,
                                         const CursorInRevision& position = CursorInRevision::invalid()) {
    Declaration* decl;

    QVector<QPair<Declaration*, int>> alldecls = context->allDeclarations(position, context->topContext());
    // search upwards
    for(const QPair<Declaration*, int>& pair : alldecls) {
            qCDebug(DUCHAIN) << "PP" << pair.first->toString() << "Depth:" << pair.second;
            // if found == return pair.first
    }

    // search downwards
//    QVector<DUContext::Import> imports = context->importedParentContexts();
    QVector<DUContext*> children = context->childContexts();
    for (DUContext* ctxt: children) {
        decl = searchDown(ctxt,identifier,position);
        if (decl != nullptr)
            return decl;
    }
    return nullptr;
}

Declaration* myFindDeclaration(DUContext* context, const QualifiedIdentifier& identifier,
                                         const CursorInRevision& position = CursorInRevision::invalid()) {
    // then back down to all imported contexts
    Declaration* decl;

    if (context == nullptr)
        return nullptr;

    decl = searchUp(context,identifier,position);
    if (decl != nullptr)
        return decl;

    return searchDown(context->topContext(),identifier,position);

}


DeclarationPointer getTypeDeclaration(QualifiedIdentifier id, DUContext *context, bool searchInParent)
{
    Q_UNUSED(searchInParent);
	DUChainReadLocker lock;
	if(context)
	{
        // recursive??
//         DUContext* walker = context;
//         while (walker != nullptr) {
            QString idstr = id.toString();
            auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
            // TODO: JG Search ourselves. DUChain is too focussed on C++
//            auto declarations = myFindDeclaration(context,id,CursorInRevision(INT_MAX, INT_MAX));
            for(Declaration *decl : declarations)
            {
                QString declstr = decl->toString();
                if(decl->kind() != Declaration::Type)
                    continue;
                return DeclarationPointer(decl);
            }
//             walker = walker->parentContext();
//         }
	}
	return DeclarationPointer();
}

QList<Declaration *> getDeclarations(QualifiedIdentifier id, DUContext *context, bool searchInParent)
{
    Q_UNUSED(searchInParent);
	DUChainReadLocker lock;
	if(context)
	{
		QList<Declaration *> decls;
		auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
		for(Declaration *decl: declarations)
		{
			if(decl->kind() == Declaration::Import)
				continue;
			decls << decl;
		}
		return decls;
	}
	return QList<Declaration *>();
}

DeclarationPointer getFirstDeclaration(DUContext *context, bool searchInParent)
{
	DUChainReadLocker lock;
	auto declarations = context->allDeclarations(CursorInRevision::invalid(), context->topContext(), searchInParent);
	if(declarations.size() > 0)
		return DeclarationPointer(declarations.first().first);
	return DeclarationPointer();
}

DeclarationPointer checkPackageDeclaration(Identifier id, TopDUContext *context)
{
	DUChainReadLocker lock;
	auto declarations = context->findLocalDeclarations(id);
	if(declarations.size() > 0)
		return DeclarationPointer(declarations.first());
	return DeclarationPointer();
}

