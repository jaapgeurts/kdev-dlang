#ifndef DUBMANAGER_H
#define DUBMANAGER_H

#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <project/projectconfigpage.h>

#include "dubbuilder.h"
#include "dubparser.h"
#include "dubsettings.h"

using namespace KDevelop;

class DUBProjectManager : public AbstractFileManagerPlugin, public IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )

public:


    // KPluginFactory-based plugin wants constructor with this signature
    explicit DUBProjectManager(QObject* parent, const QVariantList& args);

     ~DUBProjectManager() override;

     // BEGIN IPlugin

//      int configPages() const override;
//      ConfigPage * configPage(int number, QWidget * parent) override;

     // END IPlugin

     //
    //BEGIN AbstractFileManager
    ProjectFolderItem* import( IProject* project ) override;
    ProjectFolderItem* createFolderItem( IProject* project, const Path& path,
                                                   ProjectBaseItem* parent = nullptr ) override;
    Features features() const override;
    bool isValid( const Path& path, const bool isFolder, IProject* project ) const override;
    //END AbstractFileManager

    //BEGIN IBuildSystemManager
    //TODO
    IProjectBuilder*  builder() const override;
    Path buildDirectory(ProjectBaseItem*) const override;
    Path::List collectDirectories(ProjectBaseItem*, const bool collectIncludes=true) const;
    Path::List includeDirectories(ProjectBaseItem*) const override;
    Path::List frameworkDirectories(ProjectBaseItem* item) const override;
    QHash<QString,QString> defines(ProjectBaseItem*) const override;
    QString extraArguments(ProjectBaseItem *item) const override;
    bool hasBuildInfo(ProjectBaseItem*) const override;

    ProjectTargetItem* createTarget( const QString&, ProjectFolderItem* ) override
    {
        return nullptr;
    }

    bool addFilesToTarget(const QList<ProjectFileItem*>&, ProjectTargetItem*) override
    {
        return false;
    }

    bool removeTarget( ProjectTargetItem* ) override
    {
        return false;
    }

    bool removeFilesFromTargets(const QList<ProjectFileItem*>&) override
    {
        return false;
    }

    QList<ProjectTargetItem*> targets(ProjectFolderItem*) const override;

    Path compiler(ProjectTargetItem* item) const override;


    int perProjectConfigPages() const override;
    ConfigPage * perProjectConfigPage(int number, const ProjectConfigOptions & options, QWidget * parent) override;

    //END IBuildSystemManager

private Q_SLOTS:
    void slotFolderAdded( ProjectFolderItem* folder );
    void slotDirty(const QString& path);

private:
    IProjectBuilder* m_builder;
//     ProjectFolderItem* projectRootItem( IProject* project, const Path& path );
//     ProjectFolderItem* buildFolderItem( IProject* project, const Path& path, ProjectBaseItem* parent );


};

#endif // DUBMANAGER_H
