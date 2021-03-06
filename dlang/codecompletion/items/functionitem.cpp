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

#include "functionitem.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/types/functiontype.h>


FunctionCompletionItem::FunctionCompletionItem(DeclarationPointer decl, int depth, int atArgument) :
    CompletionItem(decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), 0),
    m_depth(depth), m_atArgument(atArgument)
{
	auto function = decl.dynamicCast<KDevelop::FunctionDeclaration>();
	KDevelop::FunctionType::Ptr type(fastCast<KDevelop::FunctionType *>(decl->abstractType().constData()));
	if(!type)
		return;

	DUContext *argsContext = 0;
	if(function)
		argsContext = function->internalContext();
	m_arguments = "(";
	if(argsContext)
	{
		DUChainReadLocker lock;
		auto args = argsContext->allDeclarations(CursorInRevision::invalid(), decl->topContext(), false);
		int count = 0;
		for(auto arg : args)
		{
			if(m_atArgument == count)
				m_currentArgStart = m_arguments.length();
			m_arguments += (arg.first->toString());
			if(m_atArgument == count)
				m_currentArgEnd = m_arguments.length();
			count++;
			if(count < args.size())
				m_arguments += ", ";
		}
	}
	else if(type->arguments().size() != 0)
	{
		DUChainReadLocker lock;
		auto args = type->arguments();
		int count = 0;
		for(auto arg : args)
		{
			if(m_atArgument == count)
				m_currentArgStart = m_arguments.length();
			m_arguments += (arg->toString());
			if(m_atArgument == count)
				m_currentArgEnd = m_arguments.length();
			count++;
			if(count < args.size())
				m_arguments += ", ";
		}
	}
	m_arguments += ")";
	if(m_prefix == "")
	{
		DUChainReadLocker lock;
		if(type && type->returnType())
			m_prefix += type->returnType()->toString();
	}
}

void FunctionCompletionItem::executed(KTextEditor::View *view, const KTextEditor::Range &word)
{
	KTextEditor::Document *document = view->document();
	QString suffix = "()";
	KTextEditor::Range checkSuffix(word.end().line(), word.end().column(), word.end().line(), document->lineLength(word.end().line()));
	if(document->text(checkSuffix).startsWith('('))
		suffix.clear();
	document->replaceText(word, declaration()->identifier().toString() + suffix);
	AbstractType::Ptr type = declaration()->abstractType();
	if(fastCast<KDevelop::FunctionType *>(type.constData()))
	{
		KDevelop::FunctionType *ftype = fastCast<KDevelop::FunctionType *>(type.constData());
		//Put cursor inside parentheses if function takes arguments.
		if(ftype->arguments().size() > 0)
			view->setCursorPosition(KTextEditor::Cursor(word.end().line(), word.end().column() + 1));
	}
}

QVariant FunctionCompletionItem::data(const QModelIndex &index, int role, const CodeCompletionModel *model) const
{
	/*switch(role)
	{
		case Qt::DisplayRole:
		{
			switch(index.column())
			{
				case CodeCompletionModel::Prefix:
					return m_prefix;
				case CodeCompletionModel::Arguments:
					return m_arguments;
			}
			break;
		}
		case CodeCompletionModel::CompletionRole:
			return (int)completionProperties();

		case CodeCompletionModel::HighlightingMethod:
			if(index.column() == CodeCompletionModel::Arguments)
				return (int)CodeCompletionModel::CustomHighlighting;
			break;
		case KDevelop::CodeCompletionModel::CustomHighlight:
		{
			if(index.column() == CodeCompletionModel::Arguments && m_atArgument != -1)
			{
				QTextFormat format;

				format.setBackground(QBrush(QColor::fromRgb(142, 186, 255))); //Same color as kdev-python.
				format.setProperty(QTextFormat::FontWeight, 99);

				return QVariantList()
				       << m_currentArgStart
				       << m_currentArgEnd - m_currentArgStart
				       << format;
			}
		}
	}*/
	return CompletionItem::data(index, role, model);
}

CodeCompletionModel::CompletionProperties FunctionCompletionItem::completionProperties() const
{
	return CodeCompletionModel::Function;
}

int FunctionCompletionItem::argumentHintDepth() const
{
	return m_depth;
}

int FunctionCompletionItem::inheritanceDepth() const
{
	return 0;
}
