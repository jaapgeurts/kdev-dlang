#include <KPluginFactory>

#include <debug.h>

#include "dubjob.h"
#include "dubbuilder.h"


DUBBuilder::DUBBuilder()
{
    qCDebug(DUB) << "DUBBuilder started";
}


/**
    * Installs the given project @p item, exact behaviour depends
    * on the implementation.
    *
    * @param specificPrefix defines where the project will be installed.
    */
KJob* DUBBuilder::install(KDevelop::ProjectBaseItem* item, const QUrl &specificPrefix) {

    Q_UNUSED(specificPrefix);
    qCDebug(DUB) << "install(KDevelop::ProjectBaseItem*, const QUrl &)";
    return new DUBJob(this,item,DUBJob::CommandType::InstallCommand);
};

/**
    * Builds the given project @p item, exact behaviour depends
    * on the implementation
    */
KJob* DUBBuilder::build(KDevelop::ProjectBaseItem *item) {
        qCDebug(DUB) << "build(KDevelop::ProjectBaseItem *)";

        return new DUBJob(this,item,DUBJob::CommandType::BuildCommand);

}

/**
    * Cleans the given project @p item, exact behaviour depends
    * on the implementation. The cleaning should only include
    * output files, like C/C++ object files, binaries, files
    * that the builder needs shouldn't be removed.
    */
KJob* DUBBuilder::clean(KDevelop::ProjectBaseItem *item) {
        qCDebug(DUB) << "clean(KDevelop::ProjectBaseItem *)";

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

