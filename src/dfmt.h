#ifndef DFMT_H
#define DFMT_H

#include <interfaces/iplugin.h>

class dfmt : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    dfmt(QObject* parent, const QVariantList& args);
};

#endif // DFMT_H
