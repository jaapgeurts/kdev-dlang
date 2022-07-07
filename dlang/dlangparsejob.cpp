/*************************************************************************************
 *  Copyright (C) 2015 by Thomas Brix Larsen <brix@brix-verden.dk>                   *
 *  Copyright (C) 2014 by Pavel Petrushkov <onehundredof@gmail.com>                  *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "dlangparsejob.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <language/backgroundparser/urlparselock.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/duchaindumper.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/interfaces/ilanguagesupport.h>

#include <QReadLocker>
#include <QProcess>
#include <QDirIterator>

#include "parser/parsesession.h"
#include "duchain/builders/declarationbuilder.h"
#include "duchain/builders/usebuilder.h"
#include "duchain/helper.h"
#include "ddebug.h"

#include <threadweaver/thread.h>


// TODO: JG REMOVE
#include "language/duchain/codemodel.h"

using namespace KDevelop;

void dumpDUChain(DUContext*);

//HACK. How do we remember dependency folders between parsejobs?
// because a dependency module doesn't belong to a project
// how do we set the include folders for public imports?
static QSet<QString> cachedIncludePaths;

DParseJob::DParseJob(const KDevelop::IndexedString &url, KDevelop::ILanguageSupport *languageSupport) : ParseJob(url, languageSupport)
{
    qCDebug(DPLUGIN) << "Parsejob: " << url;
    // fetch the project dependencies so we can figure out include dirs
    // find the project for this url
    IProjectController* controller = ICore::self()->projectController();
    QList<IProject*> projects = controller->projects();
    for(int i=0; i<projects.count();i++) {
        IProject* project = projects.at(i);
        qCDebug(DPLUGIN) << "Project: " << project->name();
        QSet<IndexedString> files = project->fileSet();
        foreach(const IndexedString& val, files) {
            qCDebug(DPLUGIN) << "\t" << val;
        }
    }
    IProject* project = controller->findProjectForUrl(url.toUrl());
    if (project != nullptr) {
        // add includes from the project
        IBuildSystemManager* buildManager = project->buildSystemManager();
        Path::List folders = buildManager->includeDirectories(project->projectItem());
        // Add the current document directory
        folders << Path(url.toUrl().adjusted(QUrl::RemoveFilename).path());
        for(const Path& path : folders) {
            QString includePath = path.toLocalFile();
            if (!cachedIncludePaths.contains(includePath))
                cachedIncludePaths.insert(includePath);
            m_includeDirs <<  includePath;
        }
    }

    // add any missing cached locations
    for(const QString& path : cachedIncludePaths) {
        if (!m_includeDirs.contains(path))
            m_includeDirs.append(path);
    }

    qCDebug(DPLUGIN) << m_includeDirs;
}

void DParseJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
    Q_UNUSED(self);
    Q_UNUSED(thread);
	qCDebug(DPLUGIN) << "DParseJob succesfully created for document " << document();

	UrlParseLock urlLock(document());
	if(abortRequested())
		return;

	ProblemPointer p = readContents();
	if(p) // there are problems
		return abortJob();

	QByteArray code = contents().contents;
 	while(code.endsWith('\0'))
		code.chop(1);
    code.append('\0');

	ParseSession session(code, parsePriority());
	session.setCurrentDocument(document());
	session.setFeatures(minimumFeatures());

	if(abortRequested() || ICore::self()->shuttingDown())
		return;

	ReferencedTopDUContext context;
	{
		DUChainReadLocker lock;
		context = DUChainUtils::standardContextForUrl(document().toUrl());
	}

	if(context)
	{
		translateDUChainToRevision(context);
		context->setRange(RangeInRevision(0, 0, INT_MAX, INT_MAX));
	}

	TopDUContext::Features newFeatures = minimumFeatures();
	if(context)
		newFeatures = (TopDUContext::Features)(newFeatures | context->features());
	newFeatures = (TopDUContext::Features)(newFeatures & TopDUContext::AllDeclarationsContextsAndUses);

	if(newFeatures & TopDUContext::ForceUpdate)
		qCDebug(DPLUGIN) << "update enforced";

	session.setFeatures(newFeatures);

	qCDebug(DPLUGIN) << "Job features: " << newFeatures;
	qCDebug(DPLUGIN) << "Job priority: " << parsePriority();

    // The actual parsing is done in the session
    bool parseSuccess = session.startParsing();

	// When switching between files(even if they are not modified) KDevelop
    // decides they need to be updated and calls parseJob with VisibleDeclarations
    // feature so for now feature, identifying import will be
    // AllDeclarationsAndContexts, without Uses
	bool forExport = false;
	if((newFeatures & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsAndContexts)
		forExport = true;

    // TODO: JG add computed include paths here
// 	if(!forExport)
// 		session.setIncludePaths(Helper::getSearchPaths(document().toUrl()));
// 	else
// 		session.setIncludePaths(Helper::getSearchPaths());

    session.setIncludePaths(m_includeDirs);

	if(parseSuccess)
	{
		QReadLocker parseLock(languageSupport()->parseLock());

		if(abortRequested())
			return abortJob();

        DeclarationBuilder builder(&session, forExport);
		context = builder.build(document(), session.ast(), context);

		if(!forExport && (newFeatures & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsContextsAndUses)
		{
			UseBuilder useBuilder(&session);
			useBuilder.buildUses(session.ast());
		}
		//TODO: jg What is this? This notifies other opened files of changes.
		//session.reparseImporters(context);
	}
	if(!context)
	{
		DUChainWriteLocker lock;
		ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
		file->setLanguage(ParseSession::languageString());
		context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
		DUChain::self()->addDocumentChain(context);
	}

	setDuChain(context);

    {
        DUChainWriteLocker lock;
        context->setProblems(session.problems());
    }

	highlightDUChain();

    DUChain::self()->emitUpdateReady(document(), duChain());

    // Dumps DU Chain to output
    dumpDUChain(context);




	// BEGIN JG
// 	uint count;
//     const CodeModelItem* items;
//     IndexedString file = IndexedString("/home/jaapg/tmp/trial.d");
//
//     Retrieve the items for the given file
//
//     KDevelop::CodeModel::self().items(file, count, items);
//
//     for (uint i = 0; i < count; ++i) {
//         const CodeModelItem* thisItem = items++;
//
//         qCDebug(DPLUGIN) << thisItem->id;
//
//     }
        // END JG


	if(parseSuccess)
		qCDebug(DPLUGIN) << "===Success===" << document().str();
	else
		qCDebug(DPLUGIN) << "===Failed===" << document().str();
}

const QString genSpaces(int multiplier) {
    QString str;
    for(int i=0;i<multiplier;i++) {
        str += "   ";
    }
    return str;
}

void doDump(DUContext* context, int level) {

    if (context == nullptr)
        return;
    qCDebug(DPLUGIN) << genSpaces(level) << context->url();

    DUChainDumper dumper;
    dumper.dump(context,2);

    QVector<DUContext::Import> imports = context->importedParentContexts();
    for (const DUContext::Import& import : imports) {
        doDump(import.indexedContext().context(),level++);
    }
}

void dumpDUChain(DUContext* ctxt)
{
    Q_UNUSED(ctxt);
    {

 		DUChainReadLocker lock;

        // first dump structure
        QList<TopDUContext*> allChains = DUChain::self()->allChains();

        qCDebug(DPLUGIN) << "All DU Contexts";
        for(TopDUContext* context : allChains) {
            doDump(context,1);
        }


        // Dump symbol table
        QTextStream out(stdout, QIODevice::WriteOnly);
        PersistentSymbolTable::self().dump(out);
	}

}
