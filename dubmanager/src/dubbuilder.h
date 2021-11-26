#ifndef DUBBUILDER_H
#define DUBBUILDER_H


#include <interfaces/iplugin.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>



class DUBBuilder :  public QObject, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:

    DUBBuilder();
    virtual ~DUBBuilder() = default;

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
