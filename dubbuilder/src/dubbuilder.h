#ifndef DUBBUILDER_H
#define DUBBUILDER_H


#include <interfaces/iplugin.h>
#include <project/projectmodel.h>

#include "idubbuilder.h"


class DUBBuilder : public KDevelop::IPlugin, public IDUBBuilder
{
    Q_OBJECT
    Q_INTERFACES( IDUBBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    // KPluginFactory-based plugin wants constructor with this signature
    explicit DUBBuilder(QObject* parent, const QVariantList& args);

    ~DUBBuilder() override;

    KJob* install(KDevelop::ProjectBaseItem* item, const QUrl &specificPrefix = {}) override;
    /**
    * Builds the given project @p item, exact behaviour depends
    * on the implementation
    */
    KJob* build(KDevelop::ProjectBaseItem *item) override;

    /**
    * Cleans the given project @p item, exact behaviour depends
    * on the implementation. The cleaning should only include
    * output files, like C/C++ object files, binaries, files
    * that the builder needs shouldn't be removed.
    */
    KJob* clean(KDevelop::ProjectBaseItem *item) override;

};

#endif // DUBBUILDER_H
