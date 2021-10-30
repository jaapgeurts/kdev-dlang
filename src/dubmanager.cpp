#include "dubmanager.h"

#include <debug.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(dubmanagerFactory, "dubmanager.json", registerPlugin<dubmanager>(); )

dubmanager::dubmanager(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("dubmanager"), parent)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_DUBMANAGER) << "Hello world, my plugin is loaded!";
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dubmanager.moc"
