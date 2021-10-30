#include "dubbuilder.h"

#include <debug.h>

#include <KPluginFactory>

#include "dubjob.h"



K_PLUGIN_FACTORY_WITH_JSON(DUBBuilderFactory, "dubbuilder.json", registerPlugin<DUBBuilder>(); )

DUBBuilder::DUBBuilder(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("dubbuilder"), parent)
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_DUBBUILDER) << "Hello world, my plugin is loaded!";
}

DUBBuilder::~DUBBuilder() {
}

/**
    * Installs the given project @p item, exact behaviour depends
    * on the implementation.
    *
    * @param specificPrefix defines where the project will be installed.
    */
KJob* DUBBuilder::install(KDevelop::ProjectBaseItem* item, const QUrl &specificPrefix) {

    return new DUBJob(this,item,DUBJob::CommandType::InstallCommand);
};

/**
    * Builds the given project @p item, exact behaviour depends
    * on the implementation
    */
KJob* DUBBuilder::build(KDevelop::ProjectBaseItem *item) {
        return new DUBJob(this,item,DUBJob::CommandType::BuildCommand);

}

/**
    * Cleans the given project @p item, exact behaviour depends
    * on the implementation. The cleaning should only include
    * output files, like C/C++ object files, binaries, files
    * that the builder needs shouldn't be removed.
    */
KJob* DUBBuilder::clean(KDevelop::ProjectBaseItem *item) {
    return new DUBJob(this,item,DUBJob::CommandType::CleanCommand);
}


    /**
     * Emitted when the build for the given item was finished
     */
//    void built(ProjectBaseItem *dom);
    /**
     * Emitted when the install for the given item was finished
     */
//    void installed(ProjectBaseItem*);
    /**
     * Emitted when the clean for the given item was finished
     */
//    void cleaned(ProjectBaseItem*);
    /**
     * Emitted when any of the scheduled actions for the given item was failed
     */
//    void failed(ProjectBaseItem *dom);
    /**
     * Emitted when the configure for the given item was finished
     */
//    void configured(IProject*);
    /**
     * Emitted when the pruning for the given item was finished
     */
//    void pruned(IProject*);

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dubbuilder.moc"
