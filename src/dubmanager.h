#ifndef DUBMANAGER_H
#define DUBMANAGER_H

#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>


class DUBProjectManager : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )

public:
    // KPluginFactory-based plugin wants constructor with this signature
    explicit DUBProjectManager(QObject* parent, const QVariantList& args);

     ~DUBProjectManager() override;

     //
    //BEGIN AbstractFileManager
    KDevelop::ProjectFolderItem* import( KDevelop::IProject* project ) override;
    KDevelop::ProjectFolderItem* createFolderItem( KDevelop::IProject* project, const KDevelop::Path& path,
                                                   KDevelop::ProjectBaseItem* parent = nullptr ) override;
    Features features() const override;
    bool isValid( const KDevelop::Path& path, const bool isFolder, KDevelop::IProject* project ) const override;
    //END AbstractFileManager

 //BEGIN IBuildSystemManager
    //TODO
    KDevelop::IProjectBuilder*  builder() const override;
    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path::List collectDirectories(KDevelop::ProjectBaseItem*, const bool collectIncludes=true) const;
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem* item) const override;
    QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const override;
    QString extraArguments(KDevelop::ProjectBaseItem *item) const override;
    bool hasBuildInfo(KDevelop::ProjectBaseItem*) const override;

    KDevelop::ProjectTargetItem* createTarget( const QString&, KDevelop::ProjectFolderItem* ) override
    {
        return nullptr;
    }

    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*>&, KDevelop::ProjectTargetItem*) override
    {
        return false;
    }

    bool removeTarget( KDevelop::ProjectTargetItem* ) override
    {
        return false;
    }

    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*>&) override
    {
        return false;
    }

    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override;

    KDevelop::Path compiler(KDevelop::ProjectTargetItem* item) const override;
    //END IBuildSystemManager

private Q_SLOTS:
    void slotFolderAdded( KDevelop::ProjectFolderItem* folder );
    void slotRunQMake();
    void slotDirty(const QString& path);

private:
    KDevelop::ProjectFolderItem* projectRootItem( KDevelop::IProject* project, const KDevelop::Path& path );
    KDevelop::ProjectFolderItem* buildFolderItem( KDevelop::IProject* project, const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent );
};

#endif // DUBMANAGER_H
