/*
 * DUChain Declaration class for D declarations such as templates and imports
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

#ifndef DDECLARATION_H
#define DDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/classmemberdeclarationdata.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/duchainregister.h>

namespace KDevelop {
class DUContext;
class TopDUContext;
}

using namespace KDevelop;

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DDeclarationData, m_templateargs, IndexedString)

class KDEVPLATFORMLANGUAGE_EXPORT DDeclarationData
    : public DeclarationData
{
public:


    DDeclarationData()
    {
        initializeAppendedLists();
    }

    ~DDeclarationData()
    {
        freeAppendedLists();
    }

    DDeclarationData(const DDeclarationData& rhs)
        : DeclarationData(rhs)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
    }

    DDeclarationData& operator=(const DDeclarationData& rhs) = delete;

    START_APPENDED_LISTS_BASE(DDeclarationData, DeclarationData);
    APPENDED_LIST_FIRST(DDeclarationData, IndexedString, m_templateargs);
    END_APPENDED_LISTS(DDeclarationData, m_templateargs);
};

/**
 * Represents a d declaration
 */
class KDEVPLATFORMLANGUAGE_EXPORT DDeclaration
    : public Declaration
{
public:
    using Ptr = DUChainPointer<DDeclaration>;

    enum class Kind {
        Template,
        Import,
        Module
   };

    DDeclaration(const DDeclaration& rhs);
    explicit DDeclaration(DDeclarationData& data);
    DDeclaration(const RangeInRevision& range, DUContext* context);
    DDeclaration(DDeclarationData& data, const RangeInRevision& range, DUContext* context);
    ~DDeclaration() override;

    void clearTemplateArguments();
    ///Count of base-classes
    uint templateArgumentsSize() const;
    ///The types this class is based on
    const IndexedString* templateArguments() const;
    void addTemplateArguments(const IndexedString& argument);

    Kind dKind() const;
    void setDKind(Kind kind);

    QString toString() const override;

    enum {
        Identity = 18
    };

private:
    Kind m_kind;
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(DDeclaration)
};

DUCHAIN_DECLARE_TYPE(DDeclaration)

#endif // DDECLARATION_H
