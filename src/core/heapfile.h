#ifndef HEAPFILE_H
#define HEAPFILE_H

#include <QFile>
#include <QQueue>
#include "freespacemap.h"
#include "file.h"

namespace Core
{
    class HeapFile : public File
    {
    public:
        HeapFile(const QString& relationName);
        Types::Return insertRecord() override;
        Types::Return bulkInsertRecords(const QString&) override;
        Types::Return deleteRecord() override;
        bool autogrow() override;

    private:
        FreeSpaceMap freeSpace;
    };

}

#endif // HEAPFILE_H
