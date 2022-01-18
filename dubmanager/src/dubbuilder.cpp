#include <KPluginFactory>

#include <debug.h>

#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>

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
    return createJobForAction(item,DUBJob::CommandType::InstallCommand);
};

/**
    * Builds the given project @p item, exact behaviour depends
    * on the implementation
    */
KJob* DUBBuilder::build(KDevelop::ProjectBaseItem *item) {
        qCDebug(DUB) << "build(KDevelop::ProjectBaseItem *)";

        return createJobForAction(item,DUBJob::CommandType::BuildCommand);

}

/**
    * Cleans the given project @p item, exact behaviour depends
    * on the implementation. The cleaning should only include
    * output files, like C/C++ object files, binaries, files
    * that the builder needs shouldn't be removed.
    */
KJob* DUBBuilder::clean(KDevelop::ProjectBaseItem *item) {
        qCDebug(DUB) << "clean(KDevelop::ProjectBaseItem *)";

        return createJobForAction(item,DUBJob::CommandType::CleanCommand);
}

KJob* DUBBuilder::createJobForAction(KDevelop::ProjectBaseItem *item, DUBJob::CommandType cmdType)
{
    auto bsm = item->project()->buildSystemManager();
    auto buildDir = bsm->buildDirectory(item);

    qCDebug(DUB) << buildDir;
    auto job = new DUBJob(this,buildDir.toUrl(),cmdType);
    return job;
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

