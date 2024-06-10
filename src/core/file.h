#ifndef FILE_H
#define FILE_H

#include "megatron_types.h"
// #include "buffermanager.h"

namespace Core
{
    class File
    {
    public:
        File() = default;
        virtual ~File() = default;
        virtual Types::Return insertRecord() = 0;
        virtual Types::Return bulkInsertRecords(const QString&) = 0;
        virtual Types::Return deleteRecord() = 0;

    protected:
        // cluster index
        // list of non-cluster indexes
        // QList<>

    };
}

#endif // FILE_H
