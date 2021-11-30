#ifndef DFORMAT_PLUGIN_H
#define DFORMAT_PLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/isourceformatter.h>

#include "dformatter.h"

#include <QMimeType>

class DFormatPlugin : public KDevelop::IPlugin, public KDevelop::ISourceFormatter
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ISourceFormatter)

public:
    // KPluginFactory-based plugin wants constructor with this signature
    DFormatPlugin(QObject* parent, const QVariantList& args);
    ~DFormatPlugin() override;

    QString name() const override;
    QString caption() const override;
    QString description() const override;

    /** Formats using the current style.
    */
    QString formatSource(const QString& text, const QUrl &url, const QMimeType& mime, const QString& leftContext, const QString& rightContext) const override;

    QString formatSourceWithStyle(const KDevelop::SourceFormatterStyle& style,
                                const QString& text,
                                const QUrl &url,
                                const QMimeType& mime,
                                const QString& leftContext = QString(),
                                const QString& rightContext = QString()) const override;

    /** \return A map of predefined styles (a key and a caption for each type)
    */
    QVector<KDevelop::SourceFormatterStyle> predefinedStyles() const override;

    /** \return The widget to edit a style.
    */
    KDevelop::SettingsWidget* editStyleWidget(const QMimeType& mime) const override;

    /** \return The text used in the config dialog to preview the current style.
    */
    QString previewText(const KDevelop::SourceFormatterStyle& style, const QMimeType& mime) const override;

    /** \return The indentation type of the currently selected style.
    */
    KDevelop::ISourceFormatter::Indentation indentation(const QUrl &url) const override;

    static QString formattingSample();
    static QString indentingSample();

private:
    QScopedPointer<DFormatter> m_formatter;

};

#endif // DFORMATTER_PLUGIN_H
