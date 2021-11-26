#include "dubmanager.h"

#include <debug.h>

#include <QDir>

#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>

#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

K_PLUGIN_FACTORY_WITH_JSON(DUBSupportFactory, "kdevdubmanager.json", registerPlugin<DUBProjectManager>(); )

using namespace KDevelop;

DUBProjectManager::DUBProjectManager(QObject *parent, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("kdevdubmanager"), parent),
    IBuildSystemManager()
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_KDEVDUBMANAGER) << "DUBProjectManager(QObject *, const QVariantList&";
}

DUBProjectManager::~DUBProjectManager() {
}

//BEGIN AbstractFileManager
ProjectFolderItem* DUBProjectManager::import( IProject* project )
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "import( IProject*)";

    ProjectFolderItem* item = AbstractFileManagerPlugin::import(project);
    // TODO: connect
    return item;
}

ProjectFolderItem* DUBProjectManager::createFolderItem( IProject* project, const Path& path,
                                                ProjectBaseItem* parent )
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "createFolderItem( IProject* , const Path& ,                                                ProjectBaseItem*)";

    if (!parent) {
        return new ProjectBuildFolderItem( project, path, parent );
    } else {
        return AbstractFileManagerPlugin::createFolderItem(project, path, parent);
    }
}

IProjectFileManager::Features DUBProjectManager::features() const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "features()";

    return IProjectFileManager::Features::enum_type::Folders | IProjectFileManager::Features::enum_type::Files;
}

bool DUBProjectManager::isValid( const Path& path, const bool isFolder, IProject* project ) const
{
    Q_UNUSED(isFolder);
    Q_UNUSED(project);
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "isValid( const Path& , const bool , IProject*)";
    return path.lastPathSegment()[0] != '.';
}

//END AbstractFileManager

 //BEGIN IBuildSystemManager
//TODO
IProjectBuilder*  DUBProjectManager::builder() const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "builder()";

    // Dynamically get the dub builder through the plugin system
    IPlugin* i = core()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("DUBBuilder"));
    Q_ASSERT(i);
    auto* _builder = i->extension<KDevelop::IProjectBuilder>();
    Q_ASSERT(_builder );
    return _builder;

}

Path DUBProjectManager::buildDirectory(ProjectBaseItem*) const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "buildDirectory(ProjectBaseItem*)";

    return Path();
}

Path::List DUBProjectManager::collectDirectories(ProjectBaseItem*, const bool collectIncludes) const
{
    Q_UNUSED(collectIncludes);
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "collectDirectories(ProjectBaseItem*, const bool)";

    return Path::List();
}

Path::List DUBProjectManager::includeDirectories(ProjectBaseItem*) const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "includeDirectories(ProjectBaseItem*)";

    return { Path("/usr/include/dlang/dmd") };
}

Path::List DUBProjectManager::frameworkDirectories(ProjectBaseItem* item) const
{
    Q_UNUSED(item);
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "frameworkDirectories(ProjectBaseItem*)";

    return Path::List();
}

QHash<QString,QString> DUBProjectManager::defines(ProjectBaseItem*) const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "defines(ProjectBaseItem*)";

    return QHash<QString,QString>();
}

QString DUBProjectManager::extraArguments(ProjectBaseItem *item) const
{
    Q_UNUSED(item);
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "extraArguments(ProjectBaseItem *)";

    return "";
}

bool DUBProjectManager::hasBuildInfo(ProjectBaseItem*) const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "hasBuildInfo(ProjectBaseItem*)";

    return false;
}


QList<ProjectTargetItem*> DUBProjectManager::targets(ProjectFolderItem*) const
{
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "targets(ProjectFolderItem*)";

    return QList<ProjectTargetItem*>();
}


Path DUBProjectManager::compiler(ProjectTargetItem* item) const
{
    Q_UNUSED(item);
    qCDebug(PLUGIN_KDEVDUBMANAGER) << "compiler(ProjectTargetItem*)";

    return Path();
}

void DUBProjectManager::slotFolderAdded( KDevelop::ProjectFolderItem* folder )
{
    Q_UNUSED(folder);
}

void DUBProjectManager::slotRunQMake()
{
}

void DUBProjectManager::slotDirty(const QString& path)
{
    Q_UNUSED(path);
}


//END IBuildSystemManager
/*
ProjectFolderItem* DUBProjectManager::projectRootItem(IProject* project, const Path& path)
{
    QDir dir(path.toLocalFile());

    auto item = new QMakeFolderItem(project, path);

    const auto projectfiles = dir.entryList(QStringList() << QStringLiteral("*.pro"));
    if (projectfiles.isEmpty()) {
        return item;
    }

    QHash<QString, QString> qmvars = QMakeUtils::queryQMake(project);
    const QString mkSpecFile = QMakeConfig::findBasicMkSpec(qmvars);
    Q_ASSERT(!mkSpecFile.isEmpty());
    auto* mkspecs = new QMakeMkSpecs(mkSpecFile, qmvars);
    mkspecs->setProject(project);
    mkspecs->read();
    QMakeCache* cache = findQMakeCache(project);
    if (cache) {
        cache->setMkSpecs(mkspecs);
        cache->read();
    }

    for (const auto& projectfile : projectfiles) {
        Path proPath(path, projectfile);
        /// TODO: use Path in QMakeProjectFile
        auto* scope = new QMakeProjectFile(proPath.toLocalFile());
        scope->setProject(project);
        scope->setMkSpecs(mkspecs);
        scope->setOwnMkSpecs(true);
        if (cache) {
            scope->setQMakeCache(cache);
        }
        scope->read();
        qCDebug(KDEV_QMAKE) << "top-level scope with variables:" << scope->variables();
        item->addProjectFile(scope);
    }
    return item;
}

ProjectFolderItem* DUBProjectManager::buildFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    // find .pro or .pri files in dir
    QDir dir(path.toLocalFile());
    const QStringList projectFiles = dir.entryList(QStringList{QStringLiteral("*.pro"), QStringLiteral("*.pri")},
                                             QDir::Files);
    if (projectFiles.isEmpty()) {
        return nullptr;
    }

    auto folderItem = new QMakeFolderItem(project, path, parent);

    // TODO: included by not-parent file (in a nother file-tree-branch).
    QMakeFolderItem* qmakeParent = findQMakeFolderParent(parent);
    if (!qmakeParent) {
        // happens for bad qmake configurations
        return nullptr;
    }

    for (const QString& file : projectFiles) {
        const QString absFile = dir.absoluteFilePath(file);

        // TODO: multiple includes by different .pro's
        QMakeProjectFile* parentPro = nullptr;
        foreach (QMakeProjectFile* p, qmakeParent->projectFiles()) {
            if (p->hasSubProject(absFile)) {
                parentPro = p;
                break;
            }
        }
        if (!parentPro && file.endsWith(QLatin1String(".pri"))) {
            continue;
        }
        qCDebug(KDEV_QMAKE) << "add project file:" << absFile;
        if (parentPro) {
            qCDebug(KDEV_QMAKE) << "parent:" << parentPro->absoluteFile();
        } else {
            qCDebug(KDEV_QMAKE) << "no parent, assume project root";
        }

        auto qmscope = new QMakeProjectFile(absFile);
        qmscope->setProject(project);

        const QFileInfo info(absFile);
        const QDir d = info.dir();
        /// TODO: cleanup
        if (parentPro) {
            // subdir
            if (QMakeCache* cache = findQMakeCache(project, Path(d.canonicalPath()))) {
                cache->setMkSpecs(parentPro->mkSpecs());
                cache->read();
                qmscope->setQMakeCache(cache);
            } else {
                qmscope->setQMakeCache(parentPro->qmakeCache());
            }

            qmscope->setMkSpecs(parentPro->mkSpecs());
        } else {
            // new project
            auto* root = dynamic_cast<QMakeFolderItem*>(project->projectItem());
            Q_ASSERT(root);
            qmscope->setMkSpecs(root->projectFiles().first()->mkSpecs());
            if (root->projectFiles().first()->qmakeCache()) {
                qmscope->setQMakeCache(root->projectFiles().first()->qmakeCache());
            }
        }

        if (qmscope->read()) {
            // TODO: only on read?
            folderItem->addProjectFile(qmscope);
        } else {
            delete qmscope;
            return nullptr;
        }
    }

    return folderItem;
}
*/

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dubmanager.moc"
