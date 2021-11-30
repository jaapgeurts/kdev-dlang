#include "dfmt_plugin.h"

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>

#include <KPluginFactory>

#include <QMimeDatabase>

#include "dfmt_preferences.h"

static const char formattingDSample[] =
    "void func(){\n"
    "\tif(isFoo(a,b))\n"
    "\tbar(a,b);\n"
    "if(isFoo)\n"
    "\ta=bar((b-c)*a,*d--);\n"
    "if(  isFoo( a,b ) )\n"
    "\tbar(a, b);\n"
    "if (isFoo) {isFoo=false;cat << isFoo <<endl;}\n"
    "if(isFoo)DoBar();if (isFoo){\n"
    "\tbar();\n"
    "}\n"
    "\telse if(isBar()){\n"
    "\tannotherBar();\n"
    "}\n"
    "int var = 1;\n"
    "int *ptr = &var;\n"
    "int& ref = i;\n"
    "\n"
    "}\n"
    "class someClass {\n"
    "void foo() {\n"
    "  if (true) {\n"
    "    func();\n"
    "  } else {\n"
    "    // bla\n"
    "  }\n"
    "}\n"
    "}\n"
    "}\n"
    "}\n";

static const char indentingDSample[] =
    "{Foo();Bar();}\n"
    "return Bar()\n"
    "\n"
    "class Foo\n"
    "{public:\n"
    "Foo();\n"
    "virtual ~Foo();\n"
    "};\n"
    "void bar(int foo)\n"
    "{\n"
    "switch (foo)\n"
    "{\n"
    "case 1:\n"
    "a+=1;\n"
    "break;\n"
    "case 2:\n"
    "{\n"
    "a += 2;\n"
    " break;\n"
    "}\n"
    "}\n"
    "if (isFoo)\n"
    "{\n"
    "bar();\n"
    "}\n"
    "else\n"
    "{\n"
    "anotherBar();\n"
    "}\n"
    "}\n"
    "int foo()\n"
    "\twhile(isFoo)\n"
    "\t\t{\n"
    "\t\t\t// ...\n"
    "\t\t\tgoto error;\n"
    "\t\t/* .... */\n"
    "\t\terror:\n"
    "\t\t\t//...\n"
    "\t\t}\n"
    "fooArray[]={ red,\n"
    "\tgreen,\n"
    "\tdarkblue};\n"
    "fooFunction(barArg1,\n"
    "\tbarArg2,\n"
    "\tbarArg3);\n"
    "struct foo{ int bar() {} };\n";

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(DFormatterPluginFactory, "kdevdfmt_plugin.json", registerPlugin<DFormatPlugin>(); )

DFormatPlugin::DFormatPlugin(QObject *parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevdfmt"), parent),
    m_formatter(new DFormatter())
{
    Q_UNUSED(args);

    qCDebug(DFMT) << "DFmt plugin: D source formatter started!";
}

DFormatPlugin::~DFormatPlugin()
{
}


QString DFormatPlugin::name() const
{
    // This needs to match the X-KDE-PluginInfo-Id entry from the .desktop file!
    return QStringLiteral("kdevdfmt");
}

QString DFormatPlugin::caption() const
{
    return QStringLiteral("dfmt");
}

QString DFormatPlugin::description() const
{
    return QStringLiteral("<b>DFmt</b> is a source code indenter, formatter "
        "and beautifier for the D language.<br/>Home page: <a href=\"https://github.com/dlang-community/dfmt\">https://github.com/dlang-community/dfmt</a>");
}

QString DFormatPlugin::formatSource(const QString& text, const QUrl &url, const QMimeType& mime, const QString& leftContext, const QString& rightContext) const
{
    auto style = ICore::self()->sourceFormatterController()->styleForUrl(url, mime);
    return formatSourceWithStyle(style, text, url, mime, leftContext, rightContext);

}

QString DFormatPlugin::formatSourceWithStyle(const KDevelop::SourceFormatterStyle& style,
                                const QString& text,
                                const QUrl &url,
                                const QMimeType& mime,
                                const QString& leftContext,
                                const QString& rightContext) const
{
    Q_UNUSED(url);
    Q_UNUSED(mime);
    if (style.content().isEmpty()) {
        m_formatter->predefinedStyle(style.name());
    } else {
        m_formatter->loadStyle(style.content());
    }

     return m_formatter->formatSource(text,leftContext,rightContext);
}

static SourceFormatterStyle createPredefinedStyle(const QString& name, const QString& caption = QString())
{
    SourceFormatterStyle st = SourceFormatterStyle( name );
    st.setCaption( caption.isEmpty() ? name : caption );
    DFormatter fmt;
    fmt.predefinedStyle( name );
    st.setContent( fmt.saveStyle() );
    st.setMimeTypes({SourceFormatterStyle::MimeHighlightPair{"text/x-dsrc","D"}});
    st.setUsePreview(true);
    return st;
}

QVector<SourceFormatterStyle> DFormatPlugin::predefinedStyles() const
{
    static const QVector<SourceFormatterStyle> list = {
        createPredefinedStyle(QStringLiteral("Default"))
        // Consider adding more default styles
    };
    return list;
}


SettingsWidget* DFormatPlugin::editStyleWidget(const QMimeType& mime) const
{
    if (mime.inherits(QStringLiteral("text/x-dsrc")))
        qCDebug(DFMT) << "Found a D file to format";
    else
        qCDebug(DFMT) << "Can't format file of type " << mime;

    return new DFMTPreferences();
}

QString DFormatPlugin::previewText(const SourceFormatterStyle& style, const QMimeType& mime) const
{
    Q_UNUSED(style);
    Q_UNUSED(mime);
   return
      QLatin1String("// Indentation\n") +
      indentingSample() +
      QLatin1String("\t// Formatting\n") +
      formattingSample();
}

ISourceFormatter::Indentation DFormatPlugin::indentation(const QUrl &url) const{

    // Call formatSource first, to initialize the m_formatter data structures according to the URL
    formatSource(QString(), url, QMimeDatabase().mimeTypeForUrl(url), QString(), QString());

    Indentation ret;

    ret.indentWidth = m_formatter->option(QStringLiteral("IndentSize")).toInt();

    QString s = m_formatter->option(QStringLiteral("IndentStyle")).toString();
    if(s == QLatin1String("Tabs"))
    {
        // Do tabs-only indentation
        ret.indentationTabWidth = m_formatter->option(QStringLiteral("TabWidth")).toInt();;
    }else{
        // Don't use tabs at all
        ret.indentationTabWidth = -1;
    }

    return ret;
}

QString DFormatPlugin::formattingSample()
{
    return QLatin1String(formattingDSample);

}

QString DFormatPlugin::indentingSample()
{
    return QLatin1String(indentingDSample);
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "dfmt_plugin.moc"
