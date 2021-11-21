#include "dfmt.h"

#include <debug.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(dfmtFactory, "dfmt.json", registerPlugin<dfmt>(); )

dfmt::dfmt(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("dfmt"), parent)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_DFMT) << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dfmt.moc"
