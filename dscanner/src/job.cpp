/*
    SPDX-FileCopyrightText: 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
    SPDX-FileCopyrightText: 2011 Damien Coppel <damien.coppel@gmail.com>
    SPDX-FileCopyrightText: 2011 Lionel Duc <lionel.data@gmail.com>
    SPDX-FileCopyrightText: 2011 Sebastien Rannou <mxs@sbrk.org>
    SPDX-FileCopyrightText: 2011 Lucas Sarie <lucas.sarie@gmail.com>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2002 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016-2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "job.h"

#include "debug.h"
#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <language/editor/documentrange.h>
#include <sublime/message.h>
#include <shell/problem.h>

// KF
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QElapsedTimer>
#include <QRegularExpression>

#include "utils.h"

using namespace KDevelop;

namespace dscannercheck
{

DScannerJob::DScannerJob(const DScannerParameters& params, QObject* parent)
    : OutputExecuteJob(parent),
     m_timer(new QElapsedTimer),
//     , m_showXmlOutput(params.showXmlOutput),
     m_document(params.checkPath),
     m_projectRootPath(params.projectRootPath())
{
    setJobName(i18n("DScanner Analysis (%1)", prettyPathName(params.checkPath)));

    setCapabilities(KJob::Killable);
    setStandardToolView(IOutputView::TestView);
    setBehaviours(IOutputView::AutoScroll);

    setProperties(OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.commandLine();
    qCDebug(DSCANNER) << "checking path" << params.checkPath;
}

DScannerJob::~DScannerJob()
{
    doKill();
}

void DScannerJob::postProcessStdout(const QStringList& lines)
{
    qCDebug(DSCANNER) << "processing output";

    for (const QString& line : lines) {
        // parsing lines is simple so we do it here.
        if (line[0] == QLatin1Char('#') && line[1] == QLatin1Char('#')) {
            // this is an analysis message
            QStringList parts = line.split(QLatin1Char(':'));
            QString fileName = parts[0].remove(0,2);
            int line  = parts[1].toInt();
            int column = parts[2].toInt();
            QString type = parts[3];
            QString message = parts[4];

            KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem(i18n("DScanner")));

            if (type == "error")
                problem->setSeverity(KDevelop::IProblem::Error);
            if (type == "warn")
                problem->setSeverity(KDevelop::IProblem::Warning);
            else
                problem->setSeverity(KDevelop::IProblem::Hint);
            problem->setDescription(message);
            problem->setExplanation(message);

            KDevelop::DocumentRange range;
            range.document = IndexedString(m_document);
            // TODO: JG
            // figure out the distance from the position of the error
            // till the next white space char
//             int endcolumn = column;
//             range.setRange(KTextEditor::Range(line,column,line,endcolumn));
             range.setBothLines(line-1); // dscanner reports 1 indexed.
             range.setBothColumns(column-1); // DocumentRange is 0 indexed

            problem->setFinalLocation(range);
            problem->setFinalLocationMode(KDevelop::IProblem::TrimmedLine);

            //problem->addDiagnostic();

            qCDebug(DSCANNER) << "Adding problem: " << problem;
            m_problems << problem;
        }
        else {
            emit infoMessage(this, line);
        }
    }

    m_standardOutput << lines;

    if (status() == OutputExecuteJob::JobStatus::JobRunning) {
        OutputExecuteJob::postProcessStdout(lines);
    }
}



/*
void DScannerJob::postProcessStderr(const QStringList& lines)

    static const auto xmlStartRegex = QRegularExpression(QStringLiteral("\\s*<"));

    for (const QString & line : lines) {
        // unfortunately sometime cppcheck send non-XML messages to stderr.
        // For example, if we pass '-I /missing_include_dir' to the argument list,
        // then stderr output will contains such line (tested on cppcheck 1.72):
        //
        // (information) Couldn't find path given by -I '/missing_include_dir'
        //
        // Therefore we must 'move' such messages to m_standardOutput.

        if (line.indexOf(xmlStartRegex) != -1) { // the line contains XML
            m_xmlOutput << line;

            m_parser->addData(line);

            m_problems = m_parser->parse();
            emitProblems();
        }
        else {
            IProblem::Ptr problem(new DetectedProblem(i18n("DScanner")));

            problem->setSeverity(IProblem::Error);
            problem->setDescription(line);
            problem->setExplanation(QStringLiteral("Check your DScanner settings"));

            m_problems = {problem};
            emitProblems();

            if (m_showXmlOutput) {
                m_standardOutput << line;
            } else {
                postProcessStdout({line});
            }
        }
    }

    if (status() == OutputExecuteJob::JobStatus::JobRunning && m_showXmlOutput) {
        OutputExecuteJob::postProcessStderr(lines);
    }
}
*/

void DScannerJob::start()
{
    m_standardOutput.clear();

    qCDebug(DSCANNER) << "executing:" << commandLine().join(QLatin1Char(' '));

    m_timer->restart();
    OutputExecuteJob::start();
}

void DScannerJob::childProcessError(QProcess::ProcessError e)
{
    QString messageText;

    switch (e) {
    case QProcess::FailedToStart:
        messageText = i18n("Failed to start DScanner from \"%1\".", commandLine()[0]);
        break;

    case QProcess::Crashed:
        if (status() != OutputExecuteJob::JobStatus::JobCanceled) {
            messageText = i18n("DScanner crashed.");
        }
        break;

    case QProcess::Timedout:
        messageText = i18n("DScanner process timed out.");
        break;

    case QProcess::WriteError:
        messageText = i18n("Write to DScanner process failed.");
        break;

    case QProcess::ReadError:
        messageText = i18n("Read from DScanner process failed.");
        break;

    case QProcess::UnknownError:
        // current cppcheck errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if (!messageText.isEmpty()) {
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }

    OutputExecuteJob::childProcessError(e);
}

void DScannerJob::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(DSCANNER) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

     postProcessStdout({QStringLiteral("Elapsed time: %1 s.").arg(m_timer->elapsed()/1000.0)});

    // Can't make any meaningful statement. Since DScanner returns 1 when there is something to parse
//     if (exitCode != 0) {
// //        there was something to parse
//         qCDebug(DSCANNER) << "DScanner failed, standard output: ";
//         qCDebug(DSCANNER) << m_standardOutput.join(QLatin1Char('\n'));
//         qCDebug(DSCANNER) << "DScanner failed, XML output: ";
//         qCDebug(DSCANNER) << m_xmlOutput.join(QLatin1Char('\n'));
//     }

    emitProblems();

    OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

void DScannerJob::emitProblems()
{
    if (!m_problems.isEmpty()) {
        emit problemsDetected(m_problems);
    }
}

}
