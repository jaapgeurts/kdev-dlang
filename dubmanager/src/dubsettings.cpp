#include "dubsettings.h"

#include "dubparser.h"

#include <QDebug>

DubSettings::DubSettings(const QSharedPointer<SDLNode>& root) :
  m_root(root)
{
}

const QVariant& DubSettings::findValue(const QString& path)
{
    QStringList sections = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    SDLNode* node = m_root.data();
    // Find the node first
    int count = sections.count();
    for(int i=0;i<count; i++ ) {
        node = node->nodes().value(sections.at(i)).data();
    }
    return node->values().at(0);
}

template<>
QString DubSettings::getValue<QString>(const QString& path)
{
    return findValue(path).toString();
}

template<>
int DubSettings::getValue<int>(const QString& path)
{
    return findValue(path).toInt();
}

template<>
bool DubSettings::getValue<bool>(const QString& path)
{
    return findValue(path).toBool();
}

