// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dclassfunctiondeclaration.h"

using namespace KDevelop;

DClassFunctionDeclaration::DClassFunctionDeclaration(const RangeInRevision& range, DUContext* context) :
  ClassFunctionDeclaration(range,context),
  m_methodType(MethodType::Normal)
{
}

DClassFunctionDeclaration::DClassFunctionDeclaration(ClassFunctionDeclarationData& data, const RangeInRevision& range, DUContext* context) :
    ClassFunctionDeclaration(data,range,context),
    m_methodType(MethodType::Normal)
{
}

DClassFunctionDeclaration::DClassFunctionDeclaration(ClassFunctionDeclarationData& data) :
    ClassFunctionDeclaration(data),
    m_methodType(MethodType::Normal)
{
}

void DClassFunctionDeclaration::setMethodType(DClassFunctionDeclaration::MethodType methodType)
{
    m_methodType = methodType;
}

bool DClassFunctionDeclaration::isConstructor() const
{
    return m_methodType == MethodType::Constructor;
}

bool DClassFunctionDeclaration::isDestructor() const
{
    return m_methodType == MethodType::Destructor;
}
