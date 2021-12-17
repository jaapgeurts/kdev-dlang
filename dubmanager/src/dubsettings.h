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
    T getValue(const QString& path, int i=0);

    template<typename T>
    void setValue(const QString& path, T value);

    const QList<QVariant>& getValues(const QString& path);

    void setValues(const QString& path, const QList<QVariant>& values);


    void setRoot(SDLNode* root);

private:
    QSharedPointer<SDLNode> m_root;

    SDLNode* findNode(const QString& path);
};

#endif