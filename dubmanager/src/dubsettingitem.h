// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2025 Jaap Geurts <jaapg@gmx.net>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DUBSETTINGITEM_H
#define DUBSETTINGITEM_H

#include <variant>
#include <memory>
#include <vector>
#include <optional>

#include "confignode.h"


/**
 * @todo write docs
 */
class DubSettingItem
{
public:
    /**
     * Default constructor
     */
    DubSettingItem(const std::shared_ptr<ConfigNode>& node, const std::string& name);

    // getters/setters
    std::string name();
    const std::shared_ptr<ConfigNode>& node() const;

protected:

    std::optional<std::string> findIdentifierRec(const std::shared_ptr<ConfigNode>& root);

private:

    std::string m_name;
    std::shared_ptr<ConfigNode> m_node;
};

class DubKeyValuePair : public DubSettingItem {
public:
    DubKeyValuePair(const std::shared_ptr<ConfigNode>& node, const std::string& name, const Value& value);
    Value& value();
private:
    Value m_value;

};

class DubTag : public DubSettingItem {
public:
    DubTag(const std::shared_ptr<ConfigNode>& node, const std::string& name);

    std::vector<std::shared_ptr<DubTag>> findTag(const std::string& name);
    std::shared_ptr<DubKeyValuePair> findAttribute(const std::string& name);

    // getters/setters
    std::vector<Value>& values();
private:

    void findTagRec(const std::shared_ptr<ConfigNode>& root, const std::string& name, std::vector<std::shared_ptr<DubTag>>& tagList);

    std::vector<Value> m_values;

};


#endif // DUBSETTINGITEM_H
