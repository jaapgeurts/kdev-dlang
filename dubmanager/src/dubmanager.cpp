#include "dubmanager.h"


#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <KPluginFactory>

#include <interfaces/iplugincontroller.h>
#include <interfaces/configpage.h>

#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>

#include <dubpreferences.h>

#include "toolchain/toolchainwidget.h"
#include "toolchain/ldc2toolchain.h"
#include "toolchain/gdctoolchain.h"
#include "toolchain/dmdtoolchain.h"

#include "dubbuilder.h"

#include "debug.h"

K_PLUGIN_FACTORY_WITH_JSON(DUBSupportFactory, "kdevddubmanager.json", registerPlugin<DUBProjectManager>(); )

using namespace KDevelop;


DUBProjectManager::DUBProjectManager(QObject *parent, const KPluginMetaData& metaData, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("kdevdubmanager"), parent, metaData),
    IBuildSystemManager(),
    // TODO: builder should be loaded on demand. (relieve constructor)
    m_builder(new DUBBuilder()),
    m_dubSettings(nullptr)
{
    Q_UNUSED(args);

    qCDebug(DUB) << "DUBProjectManager (QObject *, const QVariantList&)";

    // test each toolchain and add to the model.

    QSharedPointer<LDC2Toolchain> ldc2 = QSharedPointer<LDC2Toolchain>::create();
    if (ldc2->probeInstallation()) {
        qCDebug(DUB) << "Found LDC2";
        m_ToolChainList << ldc2;
    }

    QSharedPointer<GDCToolchain> gdc = QSharedPointer<GDCToolchain>::create();
    if (gdc->probeInstallation()) {
        qCDebug(DUB) << "Found GDC";
        m_ToolChainList << gdc;
    }

    QSharedPointer<DMDToolchain> dmd = QSharedPointer<DMDToolchain>::create();
    if (dmd->probeInstallation()) {
        qCDebug(DUB) << "Found DMD";
        m_ToolChainList << dmd;
    }


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

// read the settings at the start of the plugin
void DUBProjectManager::readSettings(IProject* project ) {
    qCDebug(DUB) << "void DUBProjectManager::readSettings(IProject* project )";
            // check standard locations for project file.
    QString basePath = project->path().toLocalFile();
    QString sdlFileName = basePath + QStringLiteral("/dub.sdl");
    QString jsonFileName = basePath + QStringLiteral("/dub.json");
    QString fileName;
    if (QFile::exists(sdlFileName)) {
        fileName = sdlFileName;
    }
    else if (QFile::exists(jsonFileName)) {
        fileName = jsonFileName;
    }
    else {
        // TODO: all reporting must go to problem output
        qCDebug(DUB) << "Missing dub.sdl or dub.json in project folder. This is not a D dub project. Maybe the project uses another build system?";
        return;
    }

    m_dubSettings = DubSettings::loadConfigFile(fileName);
    if (!m_dubSettings) {
        qCDebug(DUB) << "Dub project file '" << fileName << "' failed to load or parse.";
    }
    return;

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

    return IProjectFileManager::Features::enum_type::Folders
         | IProjectFileManager::Features::enum_type::Files
         | IProjectFileManager::Features::enum_type::Targets;
}

/**
* Filter interface making it possible to hide files and folders from a project.
*
* The default implementation will query all IProjectFilter plugins and ask them
* whether a given url should be included or not.
*/
bool DUBProjectManager::isValid( const Path& path, const bool isFolder, IProject* project ) const
{
    Q_UNUSED(isFolder);
    Q_UNUSED(project);


    // qCDebug(DUB) << "isValid( const Path& , const bool , IProject*)";

    // Do not show any hidden files.
    QString filename = path.lastPathSegment();
    if (filename[0] == QChar::fromLatin1('.')) {
       return false;
    }

    if (isFolder)
        return true;

    // only show the following files
    QList<QString> allowedExtensions = {
        QStringLiteral("d"),
        // QStringLiteral("md"),
        // QStringLiteral("txt"),
        QStringLiteral("json"),
        QStringLiteral("sdl"),
    };
    int dotIndex = filename.lastIndexOf(QChar::fromLatin1('.'));
    if (dotIndex == -1 || dotIndex == filename.length() - 1) {
        // no extension
        return false;
    }
    QString extension = filename.mid(dotIndex + 1);
    for(const QString& ext : allowedExtensions) {
        if (ext == extension)
            return true;
    }
    return false;

}

QList<ProjectFolderItem*> DUBProjectManager::parse(ProjectFolderItem *dom) {
    qCDebug(DUB) << "parse(ProjectFolderItem *dom)";

    Q_UNUSED(dom);

    return QList<ProjectFolderItem*>();
}

//END AbstractFileManager

 //BEGIN IBuildSystemManager
//TODO
IProjectBuilder*  DUBProjectManager::builder() const
{
    qCDebug(DUB) << "builder()";

    // for now the plugin is loaded at startup of the build manager
    // Dynamically get the dub builder through the plugin system
//     IPlugin* i = core()->pluginController()->pluginForExtension( QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("DUBBuilder"));
//     Q_ASSERT(i);
//     auto* _builder = i->extension<KDevelop::IProjectBuilder>();
//     Q_ASSERT(_builder );
    return m_builder;

}

/**
* Get the toplevel build directory for the project
*/
Path DUBProjectManager::buildDirectory(ProjectBaseItem* item) const
{
    qCDebug(DUB) << "buildDirectory(ProjectBaseItem*)";

    auto project = item->project();
    return project->path();
}


Path::List DUBProjectManager::includeDirectories(ProjectBaseItem* projectBaseItem) const
{
    qCDebug(DUB) << "DUBProjectManager::includeDirectories(ProjectBaseItem* projectBaseItem)";
    // TODO: include all project folders
    // for now just include the .dub packages from the home folder.
    Path::List folders;
    folders << getToolchainPaths(projectBaseItem->project());
    // TODO: read function and scan project folders
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

    return QString();
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


/**
* Get the number of available config pages for global settings.
* @return number of global config pages. The default implementation returns zero.
* @see configPage()
*/
int DUBProjectManager::configPages() const {
    return 1;
}

/**
* Get the global config page with the \p number, config pages from 0 to
* configPages()-1 are available if configPages() > 0.
*
* @param number index of config page
* @param parent parent widget for config page
* @return newly created config page or NULL, if the number is out of bounds, default implementation returns NULL.
* This config page should inherit from ProjectConfigPage, but it is not a strict requirement.
* The default implementation returns @c nullptr.
* @see perProjectConfigPages(), ProjectConfigPage
*/
ConfigPage * DUBProjectManager::configPage(int number, QWidget * parent) {
     if (number == 0) {
         auto form = new ToolChainWidget(parent);
         form->setToolChains(m_ToolChainList);
         return form;
     }
     return nullptr;
}

int DUBProjectManager::perProjectConfigPages() const
{
    qCDebug(DUB) << "perProjectConfigPages()";
    return 1;
}

ConfigPage* DUBProjectManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    Q_UNUSED(options);
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
    Q_UNUSED(project);
    // TODO: JG these should be configurable and depend on the detected build system
    static QString searchPaths[] = {
        // QLatin1String("/usr/include/dlang/ldc"),
        QStringLiteral("/usr/lib64/ldc/x86_64-suse-linux/include/d/"),
        QStringLiteral("/usr/include/dlang/dmd"),
        // QLatin1String("/usr/include/dlang/gcd"),
        QStringLiteral("/usr/lib64/gcc/x86_64-suse-linux/14/include/d/"),
        QStringLiteral("/usr/include/d/dmd"),
        QStringLiteral("/usr/include/d")
    };

	Path::List folders;
    for(const QString& path : searchPaths) {
        if (QFileInfo::exists(path)) {
            folders << Path(path);
        }
    }
    return folders;
}

Path::List DUBProjectManager::getProjectPaths(IProject* project) const
{
    Path::List folders;

    //Try to find path automatically for opened documents.
    QDir currentDir(project->path().toLocalFile());
    while(currentDir.exists() && (currentDir.dirName() != QStringLiteral("src") || currentDir.dirName() != QStringLiteral("source")))
    {
        if(!currentDir.cdUp())
            break;
    }
    // Add a src or source directory
    if(currentDir.exists() && (currentDir.dirName() == QStringLiteral("src") || currentDir.dirName() == QStringLiteral("source")))
        folders << Path(currentDir.absolutePath());

    return folders;
}

Path::List DUBProjectManager::getDependenciesPaths(IProject* project) const
{
    Q_UNUSED(project);
    Path::List folders;
    // TODO: figure out what to return here.
    // QString home = qEnvironmentVariable("HOME");
    // QString basePath = home + "/.dub/packages";
    // // dependencies
    // int count = m_dubSettings->numNodes("dependency");
    // for (int i=0;i<count;i++ ) {
    //     QString dep = m_dubSettings->getValue<QString>("dependency", i);
    //     QString depPath = m_dubSettings->getAttribute<QString>("dependency","path",i);
    //     QString depVersion = m_dubSettings->getAttribute<QString>("dependency", "version", i);
    //     if (!depPath.isEmpty()) {
    //         QDir canonicalPath(project->path().toLocalFile()+"/"+depPath);
    //         folders << Path(canonicalPath.absolutePath());
    //     } else if (!depVersion.isEmpty()) {
    //         depVersion.remove(0,2);
    //         QString path = basePath + "/" + dep +"-"+depVersion+"/"+dep; // TODO or source
    //         if (QFileInfo::exists(path + "/source"))
    //             folders << Path(path+"/source");
    //         else if (QFileInfo::exists(path + "/src"))
    //             folders << Path(path+"/src");
    //     }
    //     else
    //         qCDebug(DUB) << "Dependency not found. Run 'dub build' at least once: "+dep;
    // }
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
