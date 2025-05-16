// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dubsettingitem.h"

//////////////////////
// DubSettingItem
//////////////////

DubSettingItem::DubSettingItem(const std::shared_ptr<ConfigNode>& node, const std::string& name)
    : m_name(name),
      m_node(node)
{

}

std::string DubSettingItem::name()
{
    return m_name;
}

const std::shared_ptr<ConfigNode>& DubSettingItem::node() const {
    return m_node;
}

std::vector<std::shared_ptr<DubTag>> DubTag::findTag(const std::string& name) {
    std::vector<std::shared_ptr<DubTag>> result;
    findTagRec(node(), name, result);
    return result;
}

void DubTag::findTagRec(const std::shared_ptr<ConfigNode>& root, const std::string& name, std::vector<std::shared_ptr<DubTag>>& tagList) {
    using namespace std;

    // is this node a tag?
    if (root->type == NodeType::Generic && get<string>(root->value) == "Tag") {
        // find the identifier tag.
        auto id = findIdentifierRec(root->children[0]);
        if (id && id == name) {
            tagList.push_back(make_shared<DubTag>(root,*id));
        }
        return;
    }

    // this node was not a tag, now descend
    for(const auto& child : root->children) {
        // Do not enter nested tags
        if (child->type == NodeType::Generic && get<string>(child->value) != "Nested") {
            findTagRec(child, name, tagList);
        }
    }

}

std::optional<std::string> DubSettingItem::findIdentifierRec(const std::shared_ptr<ConfigNode>& root) {
    using namespace std;

    // breath first search
    for(const auto& node : root->children) {
        if (node->type == NodeType::Identifier) {
            return get<string>(node->value);
        }
    }

    for(const auto& node : root->children) {
        if (optional<string> id = findIdentifierRec(node)) {
            return id;
        }
    }
    return nullopt;
}

////////////////////
// Dub tag.
////////////////////

DubTag::DubTag(const std::shared_ptr<ConfigNode>& node, const std::string& name) :
    DubSettingItem(node,name)
{
}

std::vector<Value>& DubTag::values()
{
    return m_values;
}

////////////////////////////
// DubKeyValuePair
//////////////////////////////

std::shared_ptr<DubKeyValuePair> DubTag::findAttribute(const std::string& name)
{
    using namespace std;

    // breath first;
    for(const auto& child : node()->children) {
        if (child->type == NodeType::Attribute && get<string>(child->value) == "Attribute") {
            auto id = findIdentifierRec(child->children[0]);
            if (id == name) {
                return make_shared<DubKeyValuePair>(child, *id, child->children[1]->children[1]->value);
            }
        }
    }

    return nullptr;
}

DubKeyValuePair::DubKeyValuePair(const std::shared_ptr<ConfigNode>& node, const std::string& name, const Value& value) :
    DubSettingItem(node,name),
    m_value(value)
{
}

Value& DubKeyValuePair::value()
{
    return m_value;
}
