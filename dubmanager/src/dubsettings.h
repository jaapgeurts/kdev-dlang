/* This class handles loading and saving dub files and keeps track of the tree */

#ifndef DUBSETTINGS_H
#define DUBSETTINGS_H


#include <QString>
#include <QStringList>
#include <QSharedPointer>

#include <memory>

#include "dubsettingitem.h"

class DubSettings {

public:

    typedef QSharedPointer<DubSettings> Ptr;

    template<typename T>
    T getValue(const QString& name);

    template<typename T>
    void setValue(const QString& name, T value);

    // QList<QVariant> getValues(const QString& name);

    // void setValues(const QString& path, const QList<QVariant>& values);

    static Ptr loadConfigFile(const QString& filename);

    void saveConfigFile();

private:

    DubSettings(const QString& filepath,const QSharedPointer<DubTag>& root);

    const QSharedPointer<DubTag> m_root;
    const QString m_filepath;

};

#endif
