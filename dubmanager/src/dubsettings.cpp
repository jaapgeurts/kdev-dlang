#include "dubsettings.h"

#include "dubparser.h"

#include <QDebug>

DubSettings::DubSettings(const QSharedPointer<SDLNode>& root) :
  m_root(root)
{
}

// TODO: error handling in case nothing found
SDLNode* DubSettings::findNode(const QString& path)
{
    QStringList sections = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    SDLNode* node = m_root.data();
    // Find the node first
    int count = sections.count();
    for(int i=0;i<count; i++ ) {
        node = node->nodes().value(sections.at(i)).data();
    }
    return node;
}

template<>
QString DubSettings::getValue<QString>(const QString& path, int i)
{
    return findNode(path)->values().at(i).toString();
}

template<>
int DubSettings::getValue<int>(const QString& path, int i)
{
    return findNode(path)->values().at(i).toInt();
}

template<>
bool DubSettings::getValue<bool>(const QString& path,int i)
{
    return findNode(path)->values().at(i).toBool();
}

const QList<QVariant>& DubSettings::getValues(const QString& path)
{
        return findNode(path)->values();
}

void DubSettings::setValues(const QString& path, const QList<QVariant>& values)
{
    SDLNode* node = findNode(path);
    node->replaceValues(values);

}

template<>
void DubSettings::setValue(const QString& path, QString value) {
    SDLNode* node = findNode(path);
    node->setValue(value);
}


