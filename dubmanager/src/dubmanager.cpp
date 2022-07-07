#include "dubmanager.h"

#include <debug.h>

#include <QDir>

#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/configpage.h>

#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

#include <dubpreferences.h>

#include "dubbuilder.h"


K_PLUGIN_FACTORY_WITH_JSON(DUBSupportFactory, "kdevdubmanager.json", registerPlugin<DUBProjectManager>(); )

using namespace KDevelop;

static void sdl_emit_token(const struct sdlang_token_t* token, void* user);
static size_t sdl_read(void* ptr, size_t size, void* user);
static void sdl_report_error(enum sdlang_error_t error, int line, void* user);

DUBProjectManager::DUBProjectManager(QObject *parent, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("kdevdubmanager"), parent),
    IBuildSystemManager(),
    m_builder(new DUBBuilder()),
    m_dubSettings(nullptr)
{
    Q_UNUSED(args);

    qCDebug(DUB) << "DUBProjectManager(QObject *, const QVariantList&)";
}

DUBProjectManager::~DUBProjectManager() {
}

 // BEGIN IPlugin

// int DUBProjectManager::configPages() const
// {
//     return 1;
// }
//
// ConfigPage* DUBProjectManager::configPage(int number, QWidget * parent)
// {
//     if (number == 0) {
//         ConfigPage* page = new DubPreferences(parent);
//         return page;
//     }
//     return nullptr;
// }

     // END IPlugin


//BEGIN AbstractFileManager
ProjectFolderItem* DUBProjectManager::import(IProject* project )
{
    qCDebug(DUB) << "Importing project at: "<< project->path();

    ProjectFolderItem* item = AbstractFileManagerPlugin::import(project);

    // TODO: JG This should be read and kept per project
    readSettings(project);
    // TODO: connect
    return item;
}

void DUBProjectManager::readSettings(IProject* project ) {
            // check standard locations for project file.
    QString sdlFileName = project->path().toLocalFile() + QStringLiteral("/dub.sdl");
    QString jsonFileName = project->path().toLocalFile() + QStringLiteral("/dub.json");
    if (QFile::exists(sdlFileName)) {
         m_dubSettings = m_Parser.parseProjectFileSdl(sdlFileName);
    }
    else if (QFile::exists(jsonFileName)) {
        //m_Parser.parseProjectFileJson(jsonFileName);
        qCDebug(DUB) << "dub JSON parsing not implemented yet";
    }

}

ProjectFolderItem* DUBProjectManager::createFolderItem( IProject* project, const Path& path,
                                                ProjectBaseItem* parent )
{
    qCDebug(DUB) << "createFolderItem( IProject* , const Path& , ProjectBaseItem*)";

    if (!parent) {
        return new ProjectBuildFolderItem( project, path, parent );
    } else {
        return AbstractFileManagerPlugin::createFolderItem(project, path, parent);
    }
}

IProjectFileManager::Features DUBProjectManager::features() const
{
    qCDebug(DUB) << "features()";

    return IProjectFileManager::Features::enum_type::Folders | IProjectFileManager::Features::enum_type::Files;
}

bool DUBProjectManager::isValid( const Path& path, const bool isFolder, IProject* project ) const
{
    Q_UNUSED(isFolder);
    Q_UNUSED(project);


//    qCDebug(DUB) << "isValid( const Path& , const bool , IProject*)";
    return path.lastPathSegment()[0] != '.';
}

//END AbstractFileManager

 //BEGIN IBuildSystemManager
//TODO
IProjectBuilder*  DUBProjectManager::builder() const
{
    qCDebug(DUB) << "builder()";

    // Dynamically get the dub builder through the plugin system
//     IPlugin* i = core()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("DUBBuilder"));
//     Q_ASSERT(i);
//     auto* _builder = i->extension<KDevelop::IProjectBuilder>();
//     Q_ASSERT(_builder );
    return m_builder;

}

Path DUBProjectManager::buildDirectory(ProjectBaseItem* item) const
{
    qCDebug(DUB) << "buildDirectory(ProjectBaseItem*)";

    auto project = item->project();
    return project->path();
}


Path::List DUBProjectManager::includeDirectories(ProjectBaseItem* projectBaseItem) const
{

    // TODO: include all project folders
    // for now just include the .dub packages from the home folder.
    Path::List folders;
    folders << getToolchainPaths(projectBaseItem->project());
    // TODO: JG read function and scan project folders
    folders << getProjectPaths(projectBaseItem->project());
    folders << getDependenciesPaths(projectBaseItem->project());
    return folders;
}

Path::List DUBProjectManager::frameworkDirectories(ProjectBaseItem* item) const
{
    Q_UNUSED(item);
    qCDebug(DUB) << "frameworkDirectories(ProjectBaseItem*)";

    return {  };
}

QHash<QString,QString> DUBProjectManager::defines(ProjectBaseItem*) const
{
    qCDebug(DUB) << "defines(ProjectBaseItem*)";

    return QHash<QString,QString>();
}

QString DUBProjectManager::extraArguments(ProjectBaseItem *item) const
{
    Q_UNUSED(item);
    qCDebug(DUB) << "extraArguments(ProjectBaseItem *)";

    return "";
}

bool DUBProjectManager::hasBuildInfo(ProjectBaseItem*) const
{
    qCDebug(DUB) << "hasBuildInfo(ProjectBaseItem*)";

    return false;
}


QList<ProjectTargetItem*> DUBProjectManager::targets(ProjectFolderItem*) const
{
    qCDebug(DUB) << "targets(ProjectFolderItem*)";

    return QList<ProjectTargetItem*>();
}


Path DUBProjectManager::compiler(ProjectTargetItem* item) const
{
    Q_UNUSED(item);
    qCDebug(DUB) << "compiler(ProjectTargetItem*)";

    return Path();
}

void DUBProjectManager::slotFolderAdded( KDevelop::ProjectFolderItem* folder )
{
    Q_UNUSED(folder);
}

void DUBProjectManager::slotDirty(const QString& path)
{
    Q_UNUSED(path);
}

int DUBProjectManager::perProjectConfigPages() const
{
    qCDebug(DUB) << "perProjectConfigPages()";
    return 1;
}

ConfigPage* DUBProjectManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    qCDebug(DUB) << "perProjectConfigPage()";
    if (number == 0) {
        ConfigPage* page = new DubPreferences(this, m_dubSettings , parent);
        return page;
    }
    return nullptr;
}

//END IBuildSystemManager


Path::List DUBProjectManager::getToolchainPaths(IProject* project) const
{
    // TODO: JG these should be configurable
    static QString searchPaths[] = {
        QLatin1String("/usr/include/dlang/dmd"),
        QLatin1String("/usr/include/dlang/ldc"),
        QLatin1String("/usr/include/dlang/gcd"),
        QLatin1String("/usr/include/d/dmd"),
        QLatin1String("/usr/include/d")
    };

	Path::List folders;
    for(const QString& path : searchPaths) {
        if (QFileInfo::exists(path)) {
            folders << Path(path);
            return folders;
        }
    }
    return folders;
}

Path::List DUBProjectManager::getProjectPaths(IProject* project) const
{
    Path::List folders;

    //Try to find path automatically for opened documents.
    QDir currentDir(project->path().toLocalFile());
    while(currentDir.exists() && (currentDir.dirName() != "src" || currentDir.dirName() != "source"))
    {
        if(!currentDir.cdUp())
            break;
    }
    // Add a src or source directory
    if(currentDir.exists() && (currentDir.dirName() == "src" || currentDir.dirName() == "source"))
        folders << Path(currentDir.absolutePath());

    return folders;
}

Path::List DUBProjectManager::getDependenciesPaths(IProject* project) const
{
    Path::List folders;
    QString home = qEnvironmentVariable("HOME");
    QString basePath = home + "/.dub/packages";
    // dependencies
    int count = m_dubSettings->numNodes("dependency");
    for (int i=0;i<count;i++ ) {
        QString dep = m_dubSettings->getValue<QString>("dependency", i);
        QString depPath = m_dubSettings->getAttribute<QString>("dependency","path",i);
        QString depVersion = m_dubSettings->getAttribute<QString>("dependency", "version", i);
        if (!depPath.isEmpty()) {
            QDir canonicalPath(project->path().toLocalFile()+"/"+depPath);
            folders << Path(canonicalPath.absolutePath());
        } else if (!depVersion.isEmpty()) {
            depVersion.remove(0,2);
            QString path = basePath + "/" + dep +"-"+depVersion+"/"+dep; // TODO or source
            if (QFileInfo::exists(path + "/source"))
                folders << Path(path+"/source");
            else if (QFileInfo::exists(path + "/src"))
                folders << Path(path+"/src");
        }
        else
            qCDebug(DUB) << "Dependency not found. Run 'dub build' at least once: "+dep;
    }
    return folders;
}


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
