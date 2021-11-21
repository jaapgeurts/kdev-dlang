#ifndef DSCANNER_H
#define DSCANNER_H

#include <interfaces/iplugin.h>

class dscanner : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    dscanner(QObject* parent, const QVariantList& args);
};

#endif // DSCANNER_H
