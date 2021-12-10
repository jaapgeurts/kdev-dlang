// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DCLASSFUNCTIONDECLARATION_H
#define DCLASSFUNCTIONDECLARATION_H

#include <language/duchain/classfunctiondeclaration.h>

/**
 * @todo write docs
 */
class DClassFunctionDeclaration : public KDevelop::ClassFunctionDeclaration
{
public:

    enum class MethodType {
        Normal,
        Constructor,
        Destructor
    };

    DClassFunctionDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    DClassFunctionDeclaration(KDevelop::ClassFunctionDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    explicit DClassFunctionDeclaration(KDevelop::ClassFunctionDeclarationData& data);
    ~DClassFunctionDeclaration() override = default;

    virtual void setMethodType(MethodType methodType);

    /**
     * @todo write docs
     *
     * @return TODO
     */
    virtual bool isConstructor() const override;

    /**
     * @todo write docs
     *
     * @return TODO
     */
    virtual bool isDestructor() const override;

private:
    MethodType m_methodType;

};

#endif // DCLASSFUNCTIONDECLARATION_H
