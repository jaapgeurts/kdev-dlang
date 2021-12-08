/*
 * DUChain Declaration class for template scopes (like in D. Not for C++ templates)
 * Copyright (C) 2021  Jaap Geurts <jaapgeurts@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <language/duchain/duchainregister.h>

#include "ddeclaration.h"

#include "duchaindebug.h"

using namespace KDevelop;

REGISTER_DUCHAIN_ITEM(DDeclaration);

DEFINE_LIST_MEMBER_HASH(DDeclarationData, m_templateargs, IndexedString)

DDeclaration::DDeclaration(const DDeclaration& rhs) :
    Declaration(*new DDeclarationData(*rhs.d_func()))
{
}

DDeclaration::DDeclaration(DDeclarationData& data) :
    Declaration(data)
{
}

DDeclaration::DDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context) :
    Declaration(*new DDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

DDeclaration::DDeclaration(DDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context) :
    Declaration(data,range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

DDeclaration::~DDeclaration()
{
}

void DDeclaration::clearTemplateArguments()
{
    d_func_dynamic()->m_templateargsList().clear();
}

///Count of base-classes
uint DDeclaration::templateArgumentsSize() const
{
    return d_func()->m_templateargsSize();
}

///The types this class is based on
const IndexedString* DDeclaration::templateArguments() const
{
    return d_func()->m_templateargs();
}

void DDeclaration::addTemplateArguments(const IndexedString& argument)
{
    d_func_dynamic()->m_templateargsList().append(argument);
}

DDeclaration::Kind DDeclaration::dKind() const
{
    return m_kind;
}

void DDeclaration::setDKind(Kind kind)
{
    m_kind = kind;
}


QString DDeclaration::toString() const
{
    QString s;
    switch (m_kind) {
        case Kind::Import:
            return "Import";
        case Kind::Template:
            return "Template";
        case Kind::Module:
            return "Module";
    }

    return "Unspecified kind";
}

Declaration* DDeclaration::clonePrivate() const {
    return new DDeclaration(*this);
}

DUCHAIN_DEFINE_TYPE(DDeclaration)
