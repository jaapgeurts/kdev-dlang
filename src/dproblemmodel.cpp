/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iproject.h>
#include <language/editor/documentrange.h>
#include <shell/problemmodelset.h>

#include <KLocalizedString>

#include "dproblemmodel.h"

#include "dscanner_plugin.h"

#include "utils.h"

using namespace KDevelop;


inline ProblemModelSet* problemModelSet()
{
    return ICore::self()->languageController()->problemModelSet();
}

namespace Strings {
QString problemModelId() { return QStringLiteral("DScanner"); }
}

namespace dscannercheck
{

DProblemModel::DProblemModel(DScannerPlugin* plugin)
    : ProblemModel(plugin)
    , m_plugin(plugin)
    , m_project(nullptr)
    , m_pathLocation(DocumentRange::invalid())
{
    setFeatures(CanDoFullUpdate | ScopeFilter | SeverityFilter | Grouping | CanByPassScopeFilter);
    reset();
    problemModelSet()->addModel(Strings::problemModelId(), i18n("DScanner"), this);
}

DProblemModel::~DProblemModel()
{
    problemModelSet()->removeModel(Strings::problemModelId());
}

IProject* DProblemModel::project() const
{
    return m_project;
}

void DProblemModel::fixProblemFinalLocation(IProblem::Ptr problem)
{
    // Fix problems with incorrect range, which produced by cppcheck's errors
    // without <location> element. In this case location automatically gets "/".
    // To avoid this we set current analysis path as problem location.

    if (problem->finalLocation().document.isEmpty()) {
        problem->setFinalLocation(m_pathLocation);
    }

    const auto& diagnostics = problem->diagnostics();
    for (auto& diagnostic : diagnostics) {
        fixProblemFinalLocation(diagnostic);
    }
}

bool DProblemModel::problemExists(IProblem::Ptr newProblem)
{
    for (auto problem : qAsConst(m_problems)) {
        if (newProblem->source() == problem->source() &&
            newProblem->severity() == problem->severity() &&
            newProblem->finalLocation() == problem->finalLocation() &&
            newProblem->description() == problem->description() &&
            newProblem->explanation() == problem->explanation())
            return true;
    }

    return false;
}

void DProblemModel::setMessage(const QString& message)
{
    setPlaceholderText(message, m_pathLocation, i18n("DScanner"));
}

void DProblemModel::addProblems(const QVector<IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty()) {
        maxLength = 0;
    }

    for (auto problem : problems) {
        fixProblemFinalLocation(problem);

        if (problemExists(problem)) {
            continue;
        }

        m_problems.append(problem);
        addProblem(problem);

        // This performs adjusting of columns width in the ProblemsView
        if (maxLength < problem->description().length()) {
            maxLength = problem->description().length();
            setProblems(m_problems);
        }
    }
}

void DProblemModel::setProblems()
{
    setMessage(i18n("Analysis completed, no problems detected."));
    setProblems(m_problems);
}

void DProblemModel::reset()
{
    reset(nullptr, QString());
}

void DProblemModel::reset(IProject* project, const QString& path)
{
    m_project = project;

    m_path = path;
    m_pathLocation.document = IndexedString(m_path);

    clearProblems();
    m_problems.clear();

    QString tooltip;
    if (m_project) {
        setMessage(i18n("Analysis started..."));
        tooltip = i18nc("@info:tooltip %1 is the path of the file", "Re-run last DScanner analysis (%1)", prettyPathName(m_path));
    } else {
        tooltip = i18nc("@info:tooltip", "Re-run last DScanner analysis");
    }

    setFullUpdateTooltip(tooltip);
}

void DProblemModel::show()
{
    problemModelSet()->showModel(Strings::problemModelId());
}

void DProblemModel::forceFullUpdate()
{
    if (m_project && !m_plugin->isRunning()) {
        m_plugin->runDScanner(m_project, m_path);
    }
}

}
