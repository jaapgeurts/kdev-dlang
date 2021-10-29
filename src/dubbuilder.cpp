#include "dubbuilder.h"

#include <debug.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(dubbuilderFactory, "dubbuilder.json", registerPlugin<dubbuilder>(); )

dubbuilder::dubbuilder(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("dubbuilder"), parent)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_DUBBUILDER) << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dubbuilder.moc"
