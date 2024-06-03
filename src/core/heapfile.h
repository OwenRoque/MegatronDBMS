#ifndef HEAPFILE_H
#define HEAPFILE_H

#include "file.h"

namespace Core
{
    class HeapFile : public File
    {
    public:
        HeapFile(QSharedPointer<SystemCatalog> sc, QSharedPointer<DiskManager> dm);
        void insertRecord() override;
        void bulkInsertRecords() override;
        void deleteRecord() override;
    };
}

#endif // HEAPFILE_H
