#ifndef FILE_H
#define FILE_H

#include "diskmanager.h"
#include "systemcatalog.h"
// #include "buffermanager.h"

namespace Core
{
    class File
    {
    public:
        File(QSharedPointer<SystemCatalog> sc, QSharedPointer<DiskManager> dm);
        virtual ~File() = default;
        virtual void insertRecord() = 0;
        virtual void bulkInsertRecords() = 0;
        virtual void deleteRecord() = 0;

    protected:
        QSharedPointer<SystemCatalog> syscat;
        QSharedPointer<DiskManager> dm;

    };
}

#endif // FILE_H
