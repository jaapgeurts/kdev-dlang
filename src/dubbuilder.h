#ifndef DUBBUILDER_H
#define DUBBUILDER_H

#include <interfaces/iplugin.h>

class dubbuilder : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    dubbuilder(QObject* parent, const QVariantList& args);
};

#endif // DUBBUILDER_H
