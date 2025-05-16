// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGNODE_H
#define CONFIGNODE_H

#include <memory>
#include <vector>
#include <string>
#include <variant>

/**
 * @todo write docs
 */

typedef std::variant<int32_t,int64_t,float,double,std::string, bool> Value;


enum class NodeType {
    Generic,
    Identifier,
    Namespace,
    Attribute,
    Nested,

    Num32,
    Num64,
    Float32,
    Float64,
    Bool,
    String,
    RawString,
    DateTime,
    Null,
    WhiteSpace,
    Comment
};

struct ConfigNode
{
public:
    /**
     * Default constructor
     */

    ConfigNode(NodeType type,std::string value) :
        value(value),
        type(type) {}

    ConfigNode(int32_t value) :
        value(value),
        type(NodeType::Num32) {}

    ConfigNode(int64_t value) :
        value(value),
        type(NodeType::Num64) {}

    ConfigNode(float value) :
        value(value),
        type(NodeType::Float32) {}

    ConfigNode(double value) :
        value(value),
        type(NodeType::Float64) {}

    ConfigNode(std::string value) :
        value(value),
        type(NodeType::String) {}

    ConfigNode() :
        type(NodeType::Null) {}


    Value value;
    std::vector<std::shared_ptr<ConfigNode>> children;

    NodeType type;
};

#endif // CONFIGNODE_H
