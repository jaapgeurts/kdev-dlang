// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <language/duchain/types/typeregister.h>

#include "dintegraltype.h"
#include "duchaindebug.h"

REGISTER_TYPE(DIntegralType);

DIntegralType::DIntegralType(uint type) : IntegralType(type)
{
    d_func_dynamic()->setTypeClassId<DIntegralType>();
}

DIntegralType::DIntegralType(const DIntegralType& rhs) : IntegralType(rhs)
// TODO: JG Should we use this instead?
//IntegralType(copyData(<DIntegralType>(*rhs.d_func()))
{

}

DIntegralType::DIntegralType(DIntegralTypeData& data) : IntegralType(data)
{

}

KDevelop::AbstractType* DIntegralType::clone() const
{
    return new DIntegralType(*this);
}

uint DIntegralType::hash() const
{
    return KDevHash(AbstractType::hash()) << dataType();
}

bool DIntegralType::equals(const KDevelop::AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(fastCast<const DIntegralType*>(_rhs));

    const auto* rhs = static_cast<const DIntegralType*>(_rhs);

    return d_func()->m_dataType == rhs->d_func()->m_dataType;
}

QString DIntegralType::toString() const
{
    switch (dataType()) {
        case KDevelop::IntegralType::TypeSbyte:
            return QStringLiteral("byte");
        case KDevelop::IntegralType::TypeByte:
            return QStringLiteral("ubyte");
        case KDevelop::IntegralType::TypeShort:
            return QStringLiteral("short");
        case IntegralDTypes::TypeUshort:
            return QStringLiteral("ushort");
        case IntegralDTypes::TypeUint:
            return QStringLiteral("uint");
        case KDevelop::IntegralType::TypeLong:
            return QStringLiteral("long");
        case IntegralDTypes::TypeUlong:
            return QStringLiteral("ulong");
        case IntegralDTypes::TypeReal:
            return QStringLiteral("real");
        case KDevelop::IntegralType::TypeChar16_t:
            return QStringLiteral("wchar");
        case KDevelop::IntegralType::TypeChar32_t:
            return QStringLiteral("dchar");
        default:
            return IntegralType::toString();
    }
}


// DIntegralTypeData::DIntegralTypeData()
// {
// }
//
// DIntegralTypeData::DIntegralTypeData(const DIntegralTypeData& rhs)
//     : IntegralTypeData(rhs)
// {
// }
