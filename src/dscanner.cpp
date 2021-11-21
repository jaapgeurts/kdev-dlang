#include "dscanner.h"

#include <debug.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(dscannerFactory, "dscanner.json", registerPlugin<dscanner>(); )

dscanner::dscanner(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("dscanner"), parent)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_DSCANNER) << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dscanner.moc"
