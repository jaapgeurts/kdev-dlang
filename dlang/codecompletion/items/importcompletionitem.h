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

#pragma once

#include <language/codecompletion/normaldeclarationcompletionitem.h>

using namespace KDevelop;

namespace dlang
{

class ImportCompletionItem : public KDevelop::NormalDeclarationCompletionItem
{
public:
	ImportCompletionItem(QString packagename);
	virtual QVariant data(const QModelIndex &index, int role, const KDevelop::CodeCompletionModel *model) const override;
	void execute(KTextEditor::View *view, const KTextEditor::Range &word) override;

private:
	QString m_packageName;
};

}
