#include <fstream>
#include <variant>
#include <string>
#include "dubsettings.h"

#include "sdlparser.h"

#include "debug.h"
#include <QDebug>

DubSettings::DubSettings(const QString& filepath, const QSharedPointer<DubTag>& root) :
  m_root(root),
  m_filepath(filepath)
{

}

DubSettings::Ptr DubSettings::loadConfigFile(const QString& filepath)
{
    std::ifstream input(filepath.toStdString());
    if (!input) {
        qCDebug(DUB) << "Can't open config file: " << filepath;
        return nullptr;
    }
    std::shared_ptr<ConfigNode> root = readSDLProjectFile(input);

    auto rootTag = QSharedPointer<DubTag>::create(root, std::get<std::string>(root->value));

    return QSharedPointer<DubSettings>(new DubSettings(filepath, rootTag));

}

void DubSettings::saveConfigFile()
{
    // TODO: save to file
    std::ofstream output(m_filepath.toStdString());
    // NOTE: overwrites the file. Should write to temporary,
    // then swap with original
     // saveSDLProjectFile(root, output);
}

template<>
QString DubSettings::getValue<QString>(const QString& name)
{
    std::vector<std::shared_ptr<DubTag>> tags = m_root->findTag(name.toStdString());
    if (tags.size() == 0) {
        // nothing found
        qCDebug(DUB) << "Can't find tag with name: " << name;
        return QString();
    }



    qCDebug(DUB) << "Printing values for: " << tags[0]->name();
    if (tags[0]->values().size() == 0 ) {
        qCDebug(DUB) << "No values for tag: " << tags[0]->name();
    }
    for(const auto& val : tags[0]->values()) {
        if (auto p = std::get_if<int64_t>(&val)) {
            qCDebug(DUB) << "Values:: " << p;
        } else if (auto p = std::get_if<int32_t>(&val)) {
            qCDebug(DUB) << "Values:: " << p;
        } else if (auto p = std::get_if<std::string>(&val)) {
            qCDebug(DUB) << "Values:: " << p;
        }
    }

    return QString::fromStdString(std::get<std::string>(tags[0]->values()[0]));
}

template<>
int32_t DubSettings::getValue<int32_t>(const QString& name)
{
    std::vector<std::shared_ptr<DubTag>> tags = m_root->findTag(name.toStdString());
    if (tags.size() == 0) {
        // nothing found
        qCDebug(DUB) << "Can't find tag with name: " << name;
        return 0;
    }

    return std::get<int32_t>(tags[0]->values()[0]);
}

template<>
bool DubSettings::getValue<bool>(const QString& name)
{
    std::vector<std::shared_ptr<DubTag>> tags = m_root->findTag(name.toStdString());
    if (tags.size() == 0) {
        // nothing found
        qCDebug(DUB) << "Can't find tag with name: " << name;
        return 0;
    }

    return std::get<bool>(tags[0]->values()[0]);
}

// QList<QVariant> DubSettings::getValues(const QString& path)
// {
//     QList<QVariant> list;
//     QList<SDLNode*> nodes = findNode(path);
//
//     for(SDLNode* node : nodes) {
//         list.append(node->values());
//     }
//     return list;
// }

// void DubSettings::setValues(const QString& path, const QList<QVariant>& values)
// {
//     // TODO: JG
//   //  SDLNode* node = findNode(path);
//   //  node->replaceValues(values);
//
// }

// template<>
// void DubSettings::setValue<QString>(const QString& name, QString value) {
//   //  SDLNode* node = findNode(path);
//   //  node->setValue(value);
// }


