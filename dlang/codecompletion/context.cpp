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

//Completion code is mostly based on KDevelop QmlJS plugin which should be referenced for more details amd comments.

#include "context.h"

#include <QtCore/QDir>

#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/structuretype.h>

#include "items/completionitem.h"
#include "items/functionitem.h"
#include "items/importcompletionitem.h"
#include "helper.h"
#include "completiondebug.h"

#include <language/duchain/duchainlock.h>

using namespace KDevelop;

DCodeCompletionContext::DCodeCompletionContext(const KDevelop::DUContextPointer &context, const QString &text, const KDevelop::CursorInRevision &position, int depth) :
    KDevelop::CodeCompletionContext(context, extractLastLine(text), position, depth), m_fullText(text)
{

}

QList<CompletionTreeItemPointer> DCodeCompletionContext::completionItems(bool &abort, bool fullCompletion)
{
    Q_UNUSED(abort);
    Q_UNUSED(fullCompletion);
	qCDebug(COMPLETION) << m_text;
	QList<CompletionTreeItemPointer> items;

	//We shouldn't need anything before last semicolon (previous statements).
	if(m_text.lastIndexOf(';') != -1)
		m_text = m_text.mid(m_text.lastIndexOf(';'));

	//TODO: Fix for D.
	if(m_text.contains(QRegExp("import.*$")))
	{
		items << importCompletion();
		return items;
	}

	if(isInsideCommentOrString())
		return items;

	items << functionCallTips();

	QChar lastChar = m_text.size() > 0? m_text.at(m_text.size() - 1) : QLatin1Char('\0');
	if(lastChar == QLatin1Char('.'))
	{
		//Imports and member completions.
		items << importAndMemberCompletion();
	}
	else
		items << normalCompletion();
	return items;
}

QList<CompletionTreeItemPointer> DCodeCompletionContext::normalCompletion()
{
	//All declarations.
	QList<CompletionTreeItemPointer> items;
	DUChainReadLocker lock;
	auto declarations = m_duContext->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext());
	for(const QPair<Declaration *, int> &decl : declarations)
	{
		if(decl.first->topContext() != m_duContext->topContext())
			continue;
		if(decl.first->identifier() == globalImportIdentifier() || decl.first->identifier() == globalAliasIdentifier() || decl.first->identifier() == Identifier())
			continue;
		items << itemForDeclaration(decl);
	}
	return items;
}

QList<CompletionTreeItemPointer> DCodeCompletionContext::functionCallTips()
{
	QStack<ExpressionStackEntry> stack = expressionStack(m_text);
	QList<CompletionTreeItemPointer> items;
	int depth = 1;
	bool isTopOfStack = true;
	while(!stack.empty())
	{
		ExpressionStackEntry entry = stack.pop();
		if(entry.startPosition > 0 && m_text.at(entry.startPosition - 1) == QLatin1Char('('))
		{
			DeclarationPointer function = lastDeclaration(m_text.left(entry.startPosition - 1));
			if(function && fastCast<KDevelop::FunctionType *>(function->abstractType().constData()))
			{
				FunctionCompletionItem *item = new FunctionCompletionItem(function, depth, entry.commas);
				depth++;
				items << CompletionTreeItemPointer(item);

				if(isTopOfStack && !m_typeToMatch)
				{
					KDevelop::FunctionType::Ptr ftype(fastCast<KDevelop::FunctionType *>(function->abstractType().constData()));
					auto args = ftype->arguments();
					if(args.count() != 0)
					{
						int argument = entry.commas >= args.count()? args.count()-1 : entry.commas;
						m_typeToMatch = args.at(argument);
					}
				}
			}
		}
		isTopOfStack = false;
	}
	return items;
}

QList<CompletionTreeItemPointer> DCodeCompletionContext::importAndMemberCompletion()
{
	QList<CompletionTreeItemPointer> items;
	AbstractType::Ptr lasttype = lastType(m_text.left(m_text.size()-1));
	if(lasttype)
	{
		//Evaluate pointers,
		if(fastCast<PointerType *>(lasttype.constData()))
		{
			DUChainReadLocker lock;
			PointerType *ptype = fastCast<PointerType *>(lasttype.constData());
			if(ptype->baseType())
				lasttype = ptype->baseType();
		}
		if(fastCast<StructureType *>(lasttype.constData()))
		{
			//We have to look for module declarations.
			//TODO: Handle module aliases.
			DUChainReadLocker lock;
			Declaration *lastdeclaration = fastCast<StructureType *>(lasttype.constData())->declaration(m_duContext->topContext());
			if(lastdeclaration->kind() == Declaration::Namespace)
			{
				auto decls = getDeclarations(lastdeclaration->qualifiedIdentifier(), m_duContext.data());
				for(Declaration *declaration : decls)
				{
					DUContext *context = declaration->internalContext();
					if(!context)
						continue;
					auto declarations = context->allDeclarations(CursorInRevision::invalid(), declaration->topContext(), false);
					for(const QPair<Declaration *, int> decl : declarations)
					{
						if(decl.first == declaration)
							continue;
						QualifiedIdentifier fullname = decl.first->qualifiedIdentifier();
						Identifier ident = fullname.last();
						if(ident.toString().size() <= 0)
							continue;
						//TODO: Only import public stuff?
						items << itemForDeclaration(decl);
					}
				}
			}
		}
		//Descends through type hierarchy until it hits basic types.
		int count = 0;
		do
		{
			count++;
			StructureType *structure = fastCast<StructureType *>(lasttype.constData());
			if(structure)
			{
				//Get members.
				DUContext *context = structure->internalContext(m_duContext->topContext());
				DUChainReadLocker lock;
				auto declarations = context->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext(), false);
				lock.unlock();
				for(const QPair<Declaration *, int> &decl : declarations)
					items << itemForDeclaration(decl);
			}
			StructureType *identType = fastCast<StructureType *>(lasttype.constData());
			if(identType)
			{
				DUChainReadLocker lock;
				lasttype = identType->declaration(m_duContext->topContext())->abstractType();
			}
			else
				break;
		}
		while(lasttype && count<100);
	}
	return items;
}

QList<CompletionTreeItemPointer> DCodeCompletionContext::importCompletion()
{
	auto searchPaths = Helper::getSearchPaths();
	QList<CompletionTreeItemPointer> items;
	QString fullPath = m_text;

	QStringList pathChain = fullPath.split('.', Qt::SkipEmptyParts);
	qCDebug(COMPLETION) << pathChain;
	for(const QString &path : searchPaths)
	{
		QDir dir(path);
		if(dir.exists())
		{
			bool isValid = true;
			for(const QString &nextDir : pathChain)
			{
				isValid = dir.cd(nextDir);
				if(!isValid)
					break;
			}
			if(!dir.exists() || !isValid)
				continue;
			for(const QFileInfo &package : dir.entryInfoList(QDir::Dirs|QDir::Files))
			{
				if(package.isFile() && (package.fileName().endsWith(".d") || package.fileName().endsWith(".di")))
					items << CompletionTreeItemPointer(new ImportCompletionItem(package.baseName()));
				else if(package.isDir())
					items << CompletionTreeItemPointer(new ImportCompletionItem(package.fileName()));
			}
		}
	}
	return items;
}


QStack<DCodeCompletionContext::ExpressionStackEntry> DCodeCompletionContext::expressionStack(const QString &expression)
{
	//For details see similar function in QmlJS KDevelop plugin.
	QStack<DCodeCompletionContext::ExpressionStackEntry> stack;
	QByteArray expr(expression.toUtf8());
	/*KDevPG::QByteArrayIterator iter(expr);
	Lexer lexer(iter);
	bool atEnd=false;
	ExpressionStackEntry entry;

	entry.startPosition = 0;
	entry.operatorStart = 0;
	entry.operatorEnd = 0;
	entry.commas = 0;

	stack.push(entry);

	qint64 line, lineEnd, column, columnEnd;
	while(!atEnd)
	{
	KDevPG::Token token(lexer.read());
	switch(token.kind)
	{
	case Parser::Token_EOF:
		atEnd=true;
		break;
	case Parser::Token_LBRACE:
	case Parser::Token_LBRACKET:
	case Parser::Token_LPAREN:
	    qint64 sline, scolumn;
	    lexer.locationTable()->positionAt(token.begin, &sline, &scolumn);
	        entry.startPosition = scolumn+1;
	        entry.operatorStart = entry.startPosition;
	        entry.operatorEnd = entry.startPosition;
	        entry.commas = 0;

	        stack.push(entry);
	        break;
	case Parser::Token_RBRACE:
	case Parser::Token_RBRACKET:
	case Parser::Token_RPAREN:
	        if (stack.count() > 1) {
	            stack.pop();
	        }
	        break;
	case Parser::Token_IDENT:
	    //temporary hack to allow completion in variable declarations
	    //two identifiers in a row is not possible?
	    if(lexer.size() > 0 && lexer.at(lexer.index()-2).kind == Parser::Token_IDENT)
	    {
		lexer.locationTable()->positionAt(lexer.at(lexer.index()-2).begin, &line, &column);
		lexer.locationTable()->positionAt(lexer.at(lexer.index()-2).end+1, &lineEnd, &columnEnd);
		stack.top().operatorStart = column;
		stack.top().operatorEnd = columnEnd;
	    }
	    break;
	case Parser::Token_DOT:
	        break;
	case Parser::Token_COMMA:
	        stack.top().commas++;
	    default:
	        // The last operator of every sub-expression is stored on the stack
	        // so that "A = foo." can know that attributes of foo having the same
	        // type as A should be highlighted.
	    qCDebug(COMPLETION) << token.kind;
	    lexer.locationTable()->positionAt(token.begin, &line, &column);
	    lexer.locationTable()->positionAt(token.end+1, &lineEnd, &columnEnd);
	        stack.top().operatorStart = column;
	        stack.top().operatorEnd = columnEnd;

	}
	}*/
	return stack;
}

AbstractType::Ptr DCodeCompletionContext::lastType(const QString &expression)
{
        // TODO: JG investigate
    Q_UNUSED(expression);

	//QStack<ExpressionStackEntry> stack = expressionStack(expression);
	//QString lastExpression(expression.mid(stack.top().operatorEnd));
	//qCDebug(COMPLETION) << lastExpression;

	/*ParseSession session(lastExpression.toUtf8(), 0, false);
	ExpressionAst* expressionAst;
	if(!session.parseExpression(&expressionAst))
		return AbstractType::Ptr();*/


	/*ExpressionVisitor expVisitor(&session, this->m_duContext.data());
	expVisitor.visitExpression(expressionAst);
	if(expVisitor.lastTypes().size() != 0)
	{
	 AbstractType::Ptr type = expVisitor.lastTypes().first();
	 return type;
	}*/

	return AbstractType::Ptr();
}

DeclarationPointer DCodeCompletionContext::lastDeclaration(const QString &expression)
{
    // TODO: JG investigate
    Q_UNUSED(expression);
	//QStack<ExpressionStackEntry> stack = expressionStack(expression);
	//QString lastExpression(expression.mid(stack.top().operatorEnd));
	//qCDebug(COMPLETION) << lastExpression;

	/*ParseSession session(lastExpression.toUtf8(), 0, false);
	ExpressionAst* expressionAst;
	if(!session.parseExpression(&expressionAst))
	    return DeclarationPointer();*/

	/*ExpressionVisitor expVisitor(&session, this->m_duContext.data());
	expVisitor.visitExpression(expressionAst);
	if(expVisitor.lastDeclaration())
	    return expVisitor.lastDeclaration();*/

	return DeclarationPointer();
}

CompletionTreeItemPointer DCodeCompletionContext::itemForDeclaration(QPair<Declaration *, int> declaration)
{
	if(declaration.first->isFunctionDeclaration())
		return CompletionTreeItemPointer(new FunctionCompletionItem(DeclarationPointer(declaration.first)));
	return CompletionTreeItemPointer(new CompletionItem(DeclarationPointer(declaration.first), QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), declaration.second));
}

bool DCodeCompletionContext::isInsideCommentOrString()
{
	bool inLineComment = false;
	bool inComment = false;
	int multiCommentDepth = 0;
	bool inQuotes = false;
	bool inDoubleQuotes = false;
	bool inBackQuotes = false;
	QString text = ' ' + m_fullText;
	for(int index = 0; index < text.size()-1; ++index)
	{
		const QChar c = text.at(index);
		const QChar next = text.at(index + 1);
		if(inLineComment)
		{
			if(c == QLatin1Char('\n'))
			{
				inLineComment = false;
				continue;
			}
		}
		if(inComment)
		{
			if(c == QLatin1Char('*') && next == QLatin1Char('/'))
			{
				inComment = false;
				continue;
			}
		}
		else if(multiCommentDepth)
		{
			if(c == QLatin1Char('+') && next == QLatin1Char('/'))
			{
				multiCommentDepth--;
				continue;
			}
		}
		else if(inQuotes)
		{
			if(c != QLatin1Char('\\') && next == QLatin1Char('\''))
			{
				inQuotes = false;
				continue;
			}
		}
		else if(inDoubleQuotes)
		{
			if(c != QLatin1Char('\\') && next == QLatin1Char('\"'))
			{
				inDoubleQuotes = false;
				continue;
			}
		}
		else if(inBackQuotes)
		{
			if(c != QLatin1Char('\\') && next == QLatin1Char('`'))
			{
				inBackQuotes = false;
				continue;
			}
		}
		else
		{
			if(c == QLatin1Char('/') && next == QLatin1Char('/'))
				inLineComment = true;
			if(c == QLatin1Char('/') && next == QLatin1Char('*'))
				inComment = true;
			if(c == QLatin1Char('/') && next == QLatin1Char('+'))
				multiCommentDepth++;
			if(next == QLatin1Char('\''))
				inQuotes = true;
			if(next == QLatin1Char('\"'))
				inDoubleQuotes = true;
			if(next == QLatin1Char('`'))
				inBackQuotes = true;
		}
	}
	if(inLineComment || inComment || multiCommentDepth || inQuotes || inDoubleQuotes || inBackQuotes)
		return true;
	return false;
}


