// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2021 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DINTEGRALTYPE_H
#define DINTEGRALTYPE_H

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/abstracttype.h>

using namespace KDevelop;

class DIntegralTypeData;

/**
 * @todo write docs
 */
class DIntegralType : public IntegralType
{
public:
    enum IntegralDTypes {
        // Missing types in Kdevelop
        TypeUshort = 200,
        TypeUint,
        TypeUlong,
// reserved for future use
//         cent
//         ucent
        TypeReal,
    // imaginary types are deprecated
    //     ifloat
    //     idouble
    //     ireal
    //     cfloat
    //     cdouble
    //     creal
    };


    /**
     * Constructor
     *
     * @param type TODO
     */
    DIntegralType(uint type);

    /**
     * Constructor
     *
     * @param rhs TODO
     */
    DIntegralType(const DIntegralType& rhs);

    /**
     * Constructor
     *
     * @param data TODO
     */
    DIntegralType(DIntegralTypeData& data);

    /**
     * @todo write docs
     *
     * @return TODO
     */
    QString toString() const override;

    AbstractType* clone() const override;

    uint hash() const override;
    bool equals(const AbstractType* rhs) const override;

    enum {
        Identity = 214
    };

    using Data = DIntegralTypeData;

    TYPE_DECLARE_DATA(DIntegralType)
};

class DIntegralTypeData
    : public IntegralTypeData
{
public:

//     // Constructor
//     DIntegralTypeData();
//     // Copy constructor. \param rhs data to copy
//     DIntegralTypeData(const DIntegralTypeData& rhs);
//     ~DIntegralTypeData() = default;

};

#endif // DINTEGRALTYPE_H
