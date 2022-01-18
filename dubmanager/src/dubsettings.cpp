#include "dubsettings.h"

#include "dubparser.h"

#include <QDebug>

DubSettings::DubSettings(const QSharedPointer<SDLNode>& root) :
  m_root(root)
{
}

int DubSettings::numNodes(const QString& path)
{
    return  findNode(path).count();
}

int DubSettings::numValues(const QString& path, int nodeIndex)
{
    QList<SDLNode*> nodes = findNode(path);
    if (nodes.isEmpty())
        return 0;
    return nodes.at(nodeIndex)->values().count();
}

// TODO: error handling in case nothing found
QList<SDLNode*> DubSettings::findNode(const QString& path)
{
    QList<SDLNode*> nodes;
    QStringList sections = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    SDLNode* node = m_root.data();
    // Find the node first
    int count = sections.count();
    for(int i=0;i<count-1; i++ ) {
        node = node->nodes().at(0).data();
    }
    for(const QSharedPointer<SDLNode>& n : node->nodes()) {
        if (n->name() == sections.last())
            nodes.append(n.data());
    }
    return nodes;
}

template<>
QString DubSettings::getAttribute<QString>(const QString& path, const QString& attrib, int nodeIndex) {
    QList<SDLNode*> nodes = findNode(path);
    return nodes[nodeIndex]->attribs().value(attrib).toString();
}


template<>
QString DubSettings::getValue<QString>(const QString& path,int nodeIndex, int valueIndex)
{
    QList<SDLNode*> nodes = findNode(path);
    if (nodes.isEmpty())
        return QString();
    return nodes[nodeIndex]->values().at(valueIndex).toString();
}

template<>
int DubSettings::getValue<int>(const QString& path,int nodeIndex, int valueIndex)
{
    QList<SDLNode*> nodes = findNode(path);
    return nodes[nodeIndex]->values().at(valueIndex).toInt();
}

template<>
bool DubSettings::getValue<bool>(const QString& path,int nodeIndex, int valueIndex)
{
    QList<SDLNode*> nodes = findNode(path);
    return nodes[nodeIndex]->values().at(valueIndex).toBool();
}

QList<QVariant> DubSettings::getValues(const QString& path)
{
    QList<QVariant> list;
    QList<SDLNode*> nodes = findNode(path);

    for(SDLNode* node : nodes) {
        list.append(node->values());
    }
    return list;
}

void DubSettings::setValues(const QString& path, const QList<QVariant>& values)
{
    // TODO: JG
  //  SDLNode* node = findNode(path);
  //  node->replaceValues(values);

}

template<>
void DubSettings::setValue(const QString& path, QString value) {
  //  SDLNode* node = findNode(path);
  //  node->setValue(value);
}


