#include "dscanner_plugin.h"

#include "config/globalconfigpage.h"
#include "config/projectconfigpage.h"
#include "globalsettings.h"

#include <debug.h>

#include <language/interfaces/editorcontext.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iproject.h>
#include <util/jobstatus.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>

#include <klocalizedstring.h>

#include <KActionCollection>
#include <KPluginFactory>

#include <QAction>
#include <QMimeDatabase>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(DScannerFactory, "kdevdscanner_plugin.json", registerPlugin<dscannercheck::DScannerPlugin>(); )

// TODO: remove namespace
namespace dscannercheck {

DScannerPlugin::DScannerPlugin(QObject *parent, const QVariantList& args)
    : IPlugin(QStringLiteral("kdevdscanner"), parent),
     m_job(nullptr),
     m_currentProject(nullptr),
     m_model(new DProblemModel(this))
{
    Q_UNUSED(args);

    qCDebug(DSCANNER) << "Starting DScanner";

    qCDebug(DSCANNER) << "setting dscanner rc file";
    setXMLFile(QStringLiteral("kdevdscanner.rc"));

    QIcon dscannerIcon = QIcon::fromTheme(QStringLiteral("dscanner"));

    m_menuActionFile = new QAction(dscannerIcon, i18nc("@action", "Analyze Current File with DScanner"), this);
    connect(m_menuActionFile, &QAction::triggered, this, [this](){
        runDScanner(false);
    });
    actionCollection()->addAction(QStringLiteral("dscanner_file"), m_menuActionFile);

    m_contextActionFile = new QAction(dscannerIcon, i18nc("@item:inmenu", "DScanner"), this);
    connect(m_contextActionFile, &QAction::triggered, this, [this]() {
        runDScanner(false);
    });

    m_menuActionProject = new QAction(dscannerIcon, i18nc("@action", "Analyze Current Project with DScanner"), this);
    connect(m_menuActionProject, &QAction::triggered, this, [this](){
        runDScanner(true);
    });
    actionCollection()->addAction(QStringLiteral("dscanner_project"), m_menuActionProject);

    m_contextActionProject = new QAction(dscannerIcon, i18nc("@item:inmenu", "DScanner"), this);
    connect(m_contextActionProject, &QAction::triggered, this, [this]() {
        runDScanner(true);
    });

    m_contextActionProjectItem = new QAction(dscannerIcon, i18nc("@item:inmenu", "DScanner"), this);

    connect(core()->documentController(), &KDevelop::IDocumentController::documentClosed,
            this, &DScannerPlugin::updateActions);
    connect(core()->documentController(), &KDevelop::IDocumentController::documentActivated,
            this, &DScannerPlugin::updateActions);

    connect(core()->projectController(), &KDevelop::IProjectController::projectOpened,
            this, &DScannerPlugin::updateActions);
    connect(core()->projectController(), &KDevelop::IProjectController::projectClosed,
            this, &DScannerPlugin::projectClosed);

    updateActions();
}

DScannerPlugin::~DScannerPlugin()
{
    killDScanner();
}


bool DScannerPlugin::isRunning()
{
    return m_job != nullptr;
}

void DScannerPlugin::killDScanner()
{
    if (m_job) {
        m_job->kill(KJob::EmitResult);
    }
}

static bool isSupportedMimeType(const QMimeType& mimeType)
{
    const QString mimeName = mimeType.name();
    qCDebug(DSCANNER) << "checking mimetype: " << mimeName;
    return (mimeName == QLatin1String("text/x-dsrc"));
}

void DScannerPlugin::raiseProblemsView()
{
    m_model->show();
}

void DScannerPlugin::raiseOutputView()
{
    core()->uiController()->findToolView(
        i18nc("@title:window", "Test"),
        nullptr,
        KDevelop::IUiController::FindFlags::Raise);
}

ContextMenuExtension DScannerPlugin::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension extension = IPlugin::contextMenuExtension(context, parent);

    // check if we're called from an editor
    if (context->hasType(Context::EditorContext) && m_currentProject && !isRunning()) {
        auto eContext = static_cast<EditorContext*>(context);
        QMimeDatabase db;
        const auto mime = db.mimeTypeForUrl(eContext->url());

        if (isSupportedMimeType(mime)) {
            extension.addAction(ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
            extension.addAction(ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProject);
        }
    }

    // check if we're called from the project view
    if (context->hasType(Context::ProjectItemContext) && !isRunning()) {
        auto pContext = static_cast<ProjectItemContext*>(context);
        if (pContext->items().size() != 1) {
            return extension;
        }

        auto item = pContext->items().first();

        switch (item->type()) {
            case ProjectBaseItem::File: {
                const QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(item->path().toUrl());
                if (!isSupportedMimeType(mimetype)) {
                    return extension;
                }
                break;
            }
            case ProjectBaseItem::Folder:
            case ProjectBaseItem::BuildFolder:
                break;

            default:
                return extension;
        }

        m_contextActionProjectItem->disconnect();
        connect(m_contextActionProjectItem, &QAction::triggered, this, [this, item](){
            runDScanner(item->project(), item->path().toLocalFile());
        });

        extension.addAction(ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProjectItem);
    }

    return extension;
}

void DScannerPlugin::runDScanner(bool checkProject)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    Q_ASSERT(doc);

    if (checkProject) {
        runDScanner(m_currentProject, m_currentProject->path().toUrl().toLocalFile());
    } else {
        runDScanner(m_currentProject, doc->url().toLocalFile());
    }
}

void DScannerPlugin::runDScanner(IProject* project, const QString& path)
{
    m_model->reset(project, path);

    DScannerParameters params(project);
    params.checkPath = path;

    m_job = new DScannerJob(params);

    connect(m_job, &DScannerJob::problemsDetected, m_model.data(), &DProblemModel::addProblems);
    connect(m_job, &DScannerJob::finished, this, &DScannerPlugin::result);

    core()->uiController()->registerStatus(new JobStatus(m_job, QStringLiteral("DScanner")));
    core()->runController()->registerJob(m_job);

     if (params.hideOutputView) {
         raiseProblemsView();
     } else {
         raiseOutputView();
     }

    updateActions();
}


void DScannerPlugin::updateActions()
{
    m_currentProject = nullptr;

    m_menuActionFile->setEnabled(false);
    m_menuActionProject->setEnabled(false);

    if (isRunning()) {
        return;
    }

    KDevelop::IDocument* activeDocument = core()->documentController()->activeDocument();
    if (!activeDocument) {
        return;
    }

    QUrl url = activeDocument->url();

    m_currentProject = core()->projectController()->findProjectForUrl(url);
    if (!m_currentProject) {
        return;
    }

    m_menuActionFile->setEnabled(true);
    m_menuActionProject->setEnabled(true);
}

void DScannerPlugin::projectClosed(KDevelop::IProject* project)
{
    if (project != m_model->project()) {
        return;
    }

    killDScanner();
    m_model->reset();
}


int DScannerPlugin::configPages() const
{
    return 1;
}

ConfigPage* DScannerPlugin::configPage(int number, QWidget* parent)
{
    return number ? nullptr : new GlobalConfigPage(this, parent);
}

int DScannerPlugin::perProjectConfigPages() const
{
    return 1;
}

ConfigPage* DScannerPlugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
        return number ? nullptr : new ProjectConfigPage(this, options.project, parent);
}

void DScannerPlugin::result(KJob*)
{
    if (!core()->projectController()->projects().contains(m_model->project())) {
        m_model->reset();
    } else {
        m_model->setProblems();

        if (m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded ||
            m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
            raiseProblemsView();
        }
        else {
            raiseOutputView();
        }
    }

    m_job = nullptr; // job is automatically deleted later

    updateActions();
}

}
// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dscanner_plugin.moc"
