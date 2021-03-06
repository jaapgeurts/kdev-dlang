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

DParseJob::DParseJob(const KDevelop::IndexedString &url, KDevelop::ILanguageSupport *languageSupport) : ParseJob(url, languageSupport)
{

}

void DParseJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
    Q_UNUSED(self);
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

	//When switching between files(even if they are not modified) KDevelop decides they need to be updated
	//and calls parseJob with VisibleDeclarations feature
	//so for now feature, identifying import will be AllDeclarationsAndContexts, without Uses
	bool forExport = false;
	if((newFeatures & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsAndContexts)
		forExport = true;

	if(!forExport)
		session.setIncludePaths(Helper::getSearchPaths(document().toUrl()));
	else
		session.setIncludePaths(Helper::getSearchPaths());

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
		//This notifies other opened files of changes.
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
//     	{
// 		DUChainReadLocker lock;
// 		DUChainDumper dumper;
// 		dumper.dump(context);
//
//         QTextStream out(stdout, QIODevice::WriteOnly);
//         PersistentSymbolTable::self().dump(out);
// 	}

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
