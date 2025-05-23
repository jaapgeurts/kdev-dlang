// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dubsettingitem.h"
#include "debug.h"

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

// Finding the value means, descending to the leaf node and returning the value
Value findValue(const std::shared_ptr<ConfigNode>& root) {
    if (root->children.size() == 0)
        return root->value;

    return findValue(root->children[0]);
}


void DubTag::findTagRec(const std::shared_ptr<ConfigNode>& root, const std::string& name, std::vector<std::shared_ptr<DubTag>>& tagList) {
    using namespace std;

    // is this node a tag?
    if (root->type == NodeType::Generic && get<string>(root->value) == "Tag") {
        // find the identifier tag.
        auto id = findIdentifierRec(root->children[0]);
        if (id && id == name) {
            auto tag = make_shared<DubTag>(root,*id);
            // get this tag's values
            qCDebug(DUB) << "searching value";
            for (size_t i=1;i<root->children.size();++i) {
                qCDebug(DUB) << "adding value";
                // TODO: accessing the first child to skip spacing is fragile.
                // Make a recursive function that pulls out the value from a subtree.
                auto& child = root->children[i]->children[1];
                switch(child->type) {
                    case  NodeType::Num32:
                    case  NodeType::Num64:
                    case  NodeType::Float32:
                    case  NodeType::Float64:
                    case  NodeType::Bool:
                    case  NodeType::String:
                    case  NodeType::RawString:
                    case  NodeType::DateTime:
                    case  NodeType::Null:
                            tag->values().push_back(findValue(child));
                            break;
                    default:
                        qCDebug(DUB) << "not the right type: " << static_cast<int>(child->type);
                        break;
                }
            }
            tagList.push_back(tag);
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
