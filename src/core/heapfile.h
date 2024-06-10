#ifndef HEAPFILE_H
#define HEAPFILE_H

#include "file.h"

namespace Core
{
    class HeapFile : public File
    {
    public:
        HeapFile() = default;
        Types::Return insertRecord() override;
        Types::Return bulkInsertRecords(const QString&) override;
        Types::Return deleteRecord() override;
    };
}

#endif // HEAPFILE_H
