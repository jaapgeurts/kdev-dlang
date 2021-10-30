#ifndef DUBMANAGER_H
#define DUBMANAGER_H

#include <interfaces/iplugin.h>

class dubmanager : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    dubmanager(QObject* parent, const QVariantList& args);
};

#endif // DUBMANAGER_H
