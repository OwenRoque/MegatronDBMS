#ifndef FILE_H
#define FILE_H

#include <QSet>
// #include <set>
#include "megatron_types.h"
// #include "buffermanager.h"

namespace Core
{
    class File
    {
    public:
        File(const QString& relationName) : relationName(relationName) {}
        virtual ~File() = default;
        virtual Types::Return insertRecord() = 0;
        virtual Types::Return bulkInsertRecords(const QString&) = 0;
        virtual Types::Return deleteRecord() = 0;

    protected:
        QString relationName;
        // cluster (primary) index
        // std::set<QVariant> primaryIndex;
        // list of non-cluster indexes
        // crear objetos index que funcionen como indices,
        // acepten null keys or not
    };
}

#endif // FILE_H
