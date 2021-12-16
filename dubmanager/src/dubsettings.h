#ifndef DUBSETTINGS_H
#define DUBSETTINGS_H

#include <QString>
#include <QStringList>

class DubSettings {

public:
        QString name;
        QString description;
        QStringList authors;
        QString copyright;
        QString license;

        void setValue(const QString& key, const QString& value);
};

#endif
