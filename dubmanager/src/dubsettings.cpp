#include "dubsettings.h"

void DubSettings::setValue(const QString& key, const QString& value)
{
    if (key == QStringLiteral("name"))
        name = value;
    else if (key == QStringLiteral("description"))
        description = value;
}
