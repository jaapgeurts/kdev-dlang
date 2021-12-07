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

/**
 * This plugin is heavily influenced by kdev-go, kdev-php and kdev-qmljs plugins.
 * If you have problems figuring out how something works, try looking for
 * similar code in these plugins, it should be better documented there.
 */

#pragma once

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>
#include <QVariantList>

#include "dhighlighting.h"

namespace KDevelop
{
class IProject;
class IDocument;
class ParseJob;
}

class DPlugin : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
	Q_OBJECT
	Q_INTERFACES(KDevelop::ILanguageSupport)

public:
	explicit DPlugin(QObject *parent, const QVariantList &args);

	virtual ~DPlugin();

    virtual QString name() const override;

	virtual KDevelop::ParseJob *createParseJob(const KDevelop::IndexedString &url) override;

    KDevelop::ICodeHighlighting* codeHighlighting() const override;
//    KDevelop::BasicRefactoring* refactoring() const override;
//    KDevelop::ICreateClassHelper* createClassHelper() const override;

    //void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;
    //KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    KTextEditor::Range specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position) override;
    //QPair<QUrl, KTextEditor::Cursor> specialLanguageObjectJumpCursor(const QUrl &url, const KTextEditor::Cursor& position) override;
    QPair<QWidget*, KTextEditor::Range> specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) override;

private:
	Highlighting *m_highlighting;
};
