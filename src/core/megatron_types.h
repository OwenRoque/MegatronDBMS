#ifndef MEGATRON_TYPES_H
#define MEGATRON_TYPES_H

#include "qobjectdefs.h"
#include "qtmetamacros.h"

namespace Types
{
    Q_NAMESPACE
    enum Return
    {
        OpenError,
        ParseError,
        DuplicateError,
        Success
    };
    Q_ENUM_NS(Return)

    enum QueryClauses
    {
        SelectAll       = 'A',
        SelectCustom    = 'C',
        Where           = 'W',
        SelectInto      = 'I'
    };
    Q_ENUM_NS(QueryClauses)

    enum FileOrganization : quint8
    {
        Heap,
        Sequential,
        Hash,
        BPlusTree
    };
    Q_ENUM_NS(FileOrganization)

    enum DataType : quint8
    {
        TinyInt,
        SmallInt,
        Int,
        BigInt,
        Float,
        Double,
        Bool,
        Char,
        Varchar
    };
    Q_ENUM_NS(DataType)

    enum RecordFormat : quint8
    {
        Fixed,
        Variable
    };
    Q_ENUM_NS(RecordFormat)

    enum IndexType : quint8
    {
        // primary key default, but it can be nonCluster
        ClusterIndex,
        // unique key default,
        NonClusterIndex,
        HashIndex,
        BPlusTreeIndex
    };
    Q_ENUM_NS(IndexType)

    enum KeyType : quint8
    {
        None,
        Primary,
        Unique,
        Foreign
    };
    Q_ENUM_NS(KeyType)
}

#endif // MEGATRON_TYPES_H
