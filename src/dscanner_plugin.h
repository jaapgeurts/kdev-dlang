#ifndef DSCANNER_PLUGIN_H
#define DSCANNER_PLUGIN_H

#include <interfaces/iplugin.h>

#include "dproblemmodel.h"
#include "job.h"

class KJob;

namespace KDevelop
{
class IProject;
}

namespace dscannercheck {

class DScannerPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    DScannerPlugin(QObject* parent, const QVariantList& args);
    virtual ~DScannerPlugin() override;

    void killDScanner();
    bool isRunning();

    void runDScanner(bool checkProject);
    void runDScanner(KDevelop::IProject* project, const QString& path);

    void updateActions();
    void projectClosed(KDevelop::IProject* project);

    void raiseProblemsView();
    void raiseOutputView();

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;


    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

private:

    void result(KJob* job);

    // The scanner job
    DScannerJob* m_job;

    // The current project
    KDevelop::IProject* m_currentProject;

    QAction* m_menuActionFile;
    QAction* m_menuActionProject;
    QAction* m_contextActionFile;
    QAction* m_contextActionProject;
    QAction* m_contextActionProjectItem;

    QScopedPointer<DProblemModel> m_model;
};


}
#endif // DSCANNER_PLUGIN_H
