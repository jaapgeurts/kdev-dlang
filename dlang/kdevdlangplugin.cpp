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

#include "kdevdlangplugin.h"

#include <ddebug.h>
#include <KPluginFactory>
#include <KAboutData>

#include <language/codecompletion/codecompletion.h>
#include <language/duchain/use.h>
#include <language/duchain/duchainutils.h>
#include <language/editor/documentcursor.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "duchain/builders/ddeclaration.h"
#include "duchain/navigationwidget.h"
#include "duchain/helper.h"

#include "codecompletion/model.h"
#include "dlangparsejob.h"

#include "parser/dparser.h"

K_PLUGIN_FACTORY_WITH_JSON(DPluginFactory, "kdevdlang.json", registerPlugin<DPlugin>();)

using namespace KDevelop;


DPlugin::DPlugin(QObject *parent, const QVariantList &) : KDevelop::IPlugin("kdevdlangplugin", parent), ILanguageSupport()
{
//	KDEV_USE_EXTENSION_INTERFACE(ILanguageSupport)

	qCDebug(D) << "D Language Plugin is loaded\n";

    // If this is included: then error:
    // kf.xmlgui: cannot find .rc file "kdevdlangplugin.rc" for component "kdevdlangplugin"

   // setXMLFile( QStringLiteral("kdevdlangplugin.rc") );

   // dlang::Helper::registerDUChainItems();

	initDParser();

	CodeCompletionModel *codeCompletion = new dlang::CodeCompletionModel(this);
	new CodeCompletion(this, codeCompletion, name());

	m_highlighting = new Highlighting(this);
}

DPlugin::~DPlugin()
{

    // TODO: JG think about the order or destruction
   //dlang::Helper::unregisterDUChainItems();

	deinitDParser();
}

QString DPlugin::name() const
{
	return "D";
}

// NOTE must not run multithreaded because libdparse is not re-entrant
ParseJob *DPlugin::createParseJob(const IndexedString &url)
{
	qCDebug(D) << "Creating dlang parse job\n";
    // TODO: only parse files in source directories (not in build directories)
    if (url.str().contains("build",Qt::CaseSensitivity::CaseInsensitive))
        return nullptr;

    // Queue jobs so that one 1 is execute at a time

    return new DParseJob(url, this);
//     ParseJob* pj = new DParseJob(url, this);
//     connect(pj,&ParseJob::
}

// TODO: reconsider placement of this code because this pulls in too many
// dependencies
QPair<TopDUContextPointer, Use> useForPosition(const QUrl &url, const KTextEditor::Cursor& position)
{

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);

    if (topContext) {
        CursorInRevision cursor = topContext->transformToLocalRevision(position);
        DUContext* context = topContext->findContextAt(cursor,false);
        int useAt = context->findUseAt(cursor);
        if (useAt >= 0) {
            Use use = context->uses()[useAt];
            if (dynamic_cast<DDeclaration*>(use.usedDeclaration(topContext))) {
                return {TopDUContextPointer(topContext), use};
            }
        }
    }

    return {{}, Use()};
}

QPair<TopDUContextPointer, DDeclaration*> declarationForPosition(const QUrl &url, const KTextEditor::Cursor& position)
{

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);

    if (topContext) {
        CursorInRevision cursor = topContext->transformToLocalRevision(position);
        DUContext* context = topContext->findContextAt(cursor,false);
        while (context) {
            Declaration* decl = context->findDeclarationAt(cursor);
            if (dynamic_cast<DDeclaration*>(decl)) {
                return {TopDUContextPointer(topContext), dynamic_cast<DDeclaration*>(decl)};
            }
            context = context->parentContext();
        }
    }

    // symbol not found here?
    // Also search PersistentSymbolTable(search modules)


    return {{}, nullptr};
}

KTextEditor::Range DPlugin::specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position)
{

    DUChainReadLocker lock;
    const QPair<TopDUContextPointer, Use> templateDecl = useForPosition(url, position);

    if (templateDecl.first) {
        return templateDecl.first->transformFromLocalRevision(templateDecl.second.m_range);
    }

    return KTextEditor::Range::invalid();
}

QPair<QWidget*, KTextEditor::Range> DPlugin::specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) {

    DUChainReadLocker lock;

    const QPair<TopDUContextPointer, Use> templateUse = useForPosition(url, position);

    Declaration* declaration = nullptr;
    TopDUContextPointer pointer;
    RangeInRevision range;
    if (templateUse.first) {
        declaration = templateUse.second.usedDeclaration(templateUse.first.data());
        pointer = templateUse.first;
        range = templateUse.second.m_range;
    } else {
        const QPair<TopDUContextPointer, DDeclaration*> templateDecl = declarationForPosition(url, position);
        if (templateDecl.first) {
            range = templateDecl.second->range();
            declaration = templateDecl.second;
            pointer = templateDecl.first;
        }
    }

    if (declaration) {
        const DDeclaration::Ptr ddeclaration(dynamic_cast<DDeclaration*>(declaration));

        auto rangeInRevision = pointer->transformFromLocalRevision(range.start);
        return {
            new DlangNavigationWidget(ddeclaration, DocumentCursor(IndexedString(url), rangeInRevision)),
            range.castToSimpleRange()
        };
    }

    return { nullptr,KTextEditor::Range::invalid() };
}

KDevelop::ICodeHighlighting *DPlugin::codeHighlighting() const
{
	return m_highlighting;
}

#include "kdevdlangplugin.moc"
