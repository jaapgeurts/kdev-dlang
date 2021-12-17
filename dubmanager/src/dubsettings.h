#ifndef DUBSETTINGS_H
#define DUBSETTINGS_H

#include <QString>
#include <QStringList>
#include <QSharedPointer>

class SDLNode;
class QVariant;

class DubSettings {

public:

    DubSettings(const QSharedPointer<SDLNode>& root);

    /** Path is specified as A/B/C where A,B,C are nodes and / indicates a child node */
    template<typename T>
    T getValue(const QString& path);

    void setRoot(SDLNode* root);

private:
    QSharedPointer<SDLNode> m_root;

    const QVariant& findValue(const QString& path);
};

#endif
