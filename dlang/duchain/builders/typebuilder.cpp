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

#include "typebuilder.h"

#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/structuretype.h>

#include "dintegraltype.h"

#include "helper.h"

using namespace KDevelop;

#include "duchaindebug.h"

void TypeBuilder::visitTypeName(IType *node)
{
	if(!node)
	{
		injectType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
		return;
	}
	if (node->getType2()->getTypeofExpression()) {
        qCDebug(DUCHAIN) << "WARNING: typeof() expressions not implemented";
        return;
    }
	// TODO: JG this should change to a visit method
	QualifiedIdentifier ident;
	if(auto identPart = node->getType2()->getTypeIdentifierPart()) {
        if(auto templateInstance = identPart->getIdentifierOrTemplateInstance()->getTemplateInstance())
            ident = identifierForNode(templateInstance->getIdentifier());
        else if (auto templateInstance = identPart->getIdentifierOrTemplateInstance())
            ident = identifierForNode(templateInstance->getIdentifier());
    } else {
        // TODO: JG fix this
        if (auto t = node->getType2()->getBuiltinType()) {
            ident = QualifiedIdentifier(t);
        } else if (auto t  = node->getType2()->getType()) {
            if (auto t2= t->getType2())
                if (auto t3 = t2->getBuiltinType())
                    ident = QualifiedIdentifier(t3);
        }
    }
    if (ident.isEmpty()) {
        qCDebug(DUCHAIN) << "ERROR: Empty identifier for node at line : " << node->getStartLine();
        return;
    }
    buildTypeName(ident);

	for(size_t i=0; i<node->numTypeSuffixes(); i++)
	{
		if(node->getTypeSuffix(i)->getArray())
		{
			KDevelop::ArrayType::Ptr array(new KDevelop::ArrayType());
			array->setElementType(lastType());
			array->setDimension(0);
			injectType(array);
		}

		if(QString::fromUtf8(node->getTypeSuffix(i)->getStar()->getType()) != "")
		{
			KDevelop::PointerType::Ptr pointer(new KDevelop::PointerType());
			pointer->setBaseType(lastType());
			injectType(pointer);
		}
	}
}

void TypeBuilder::buildTypeName(QualifiedIdentifier typeName)
{
	uint type = IntegralType::TypeNone;
	QString name = typeName.toString();
	//Builtin types.
	if(name == "void")
		type = KDevelop::IntegralType::TypeVoid;
	else if(name == "ubyte")
		type = KDevelop::IntegralType::TypeByte;
	else if(name == "byte")
		type = KDevelop::IntegralType::TypeSbyte;
    else if(name == "ushort")
		type = DIntegralType::TypeUshort;
	else if(name == "short")
		type = KDevelop::IntegralType::TypeShort;
    else if(name == "uint")
		type = DIntegralType::TypeUint;
	else if(name == "int")
		type = KDevelop::IntegralType::TypeInt;
    else if(name == "ulong")
		type = DIntegralType::TypeUlong;
	else if(name == "long")
		type = KDevelop::IntegralType::TypeLong;
	else if(name == "float")
		type = KDevelop::IntegralType::TypeFloat;
	else if(name == "double")
		type = KDevelop::IntegralType::TypeDouble;
	else if(name == "real")
		type = DIntegralType::TypeReal;
	else if(name == "char")
		type = KDevelop::IntegralType::TypeChar;
	else if(name == "wchar")
		type = KDevelop::IntegralType::TypeChar16_t;
	else if(name == "dchar")
		type = KDevelop::IntegralType::TypeChar32_t;
	else if(name == "bool")
		type = KDevelop::IntegralType::TypeBoolean;


	if(type == IntegralType::TypeNone)
	{
		DeclarationPointer decl = getTypeDeclaration(typeName, currentContext());
//        qCDebug(DUCHAIN) << "TypeBuilder: getting decl for: " << typeName;
		if(decl)
		{
			DUChainReadLocker lock;
//            qCDebug(DUCHAIN) << "TypeBuilder: Found " << decl->qualifiedIdentifier();
			StructureType *type = new StructureType();
			type->setDeclaration(decl.data());
			injectType(AbstractType::Ptr(type));
			return;
		}
		// This happens e.g. with foreach without type spec.
		// It needs to be supplied later depending on the expression of foreach
		DelayedType *unknown = new DelayedType();
		unknown->setIdentifier(IndexedTypeIdentifier(typeName));
        // TODO: JG BEGIN added by me:
        //unknown->setKind(DelayedType::Unresolved);
        //
        //JG END
		injectType(AbstractType::Ptr(unknown));
		return;
	}
	if(type != IntegralType::TypeNone) {
        qCDebug(DUCHAIN) << "Recording type: " << name;
		injectType(AbstractType::Ptr(new DIntegralType(type)));
    }
}

void TypeBuilder::visitParameter(IParameter *node)
{
	TypeBuilderBase::visitParameter(node);
    //qCDebug(DUCHAIN) << "Recording type :  " << lastType()->toString();
	currentFunctionType->addArgument(lastType());
}

void TypeBuilder::visitFuncDeclaration(IFunctionDeclaration *node)
{
	DUChainWriteLocker lock;
	clearLastType();

	visitTypeName(node->getReturnType());

	FunctionType::Ptr functionType = FunctionType::Ptr(new FunctionType());
	currentFunctionType = functionType;

	if(lastType())
		functionType->setReturnType(lastType());

	openType(functionType);

	closeType();
}

void TypeBuilder::visitConstructor(IConstructor *node)
{
    Q_UNUSED(node);
	DUChainWriteLocker lock;
	clearLastType();

	FunctionType::Ptr functionType = FunctionType::Ptr(new FunctionType());
	currentFunctionType = functionType;

	openType(functionType);

	closeType();
}

void TypeBuilder::visitDestructor(IDestructor *node)
{
    Q_UNUSED(node);
	DUChainWriteLocker lock;
	clearLastType();

	FunctionType::Ptr functionType = FunctionType::Ptr(new FunctionType());
	currentFunctionType = functionType;

	openType(functionType);

	closeType();
}

void TypeBuilder::visitClassDeclaration(IClassDeclaration *node)
{

    StructureType::Ptr structureType = KDevelop::StructureType::Ptr(new KDevelop::StructureType);
    currentStructureType = structureType;
    openType(structureType);

	TypeBuilderBase::visitClassDeclaration(node);
	closeType();
}

void TypeBuilder::visitStructDeclaration(IStructDeclaration *node)
{
    StructureType::Ptr structureType = KDevelop::StructureType::Ptr(new KDevelop::StructureType);
    currentStructureType = structureType;

    openType(structureType);
	TypeBuilderBase::visitStructDeclaration(node);

	closeType();
}

void TypeBuilder::visitInterfaceDeclaration(IInterfaceDeclaration *node)
{
    StructureType::Ptr structureType = KDevelop::StructureType::Ptr(new KDevelop::StructureType);
    currentStructureType = structureType;
    openType(structureType);

	TypeBuilderBase::visitInterfaceDeclaration(node);
	closeType();
}

void TypeBuilder::visitEnumDeclaration(IEnumDeclaration *node)
{
	enumValueCounter = 0;
	TypeBuilderBase::visitEnumDeclaration(node);
	//TODO: Save type for use in members?
	if(auto n = node->getType())
		visitTypeName(n);
	else
		injectType(AbstractType::Ptr(new IntegralType(IntegralType::TypeInt)));
}

void TypeBuilder::visitEnumMember(IEnumMember *node)
{
	EnumeratorType::Ptr enumerator(new EnumeratorType());
	openType(enumerator);
	enumerator->setValue<qint64>(enumValueCounter);
	TypeBuilderBase::visitEnumMember(node);
	closeType();
	enumValueCounter++;
}
