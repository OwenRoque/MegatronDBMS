#ifndef MEGATRON_TYPES_H
#define MEGATRON_TYPES_H

#include "qobjectdefs.h"
#include "qtmetamacros.h"
#include <QString>

namespace Types
{
    Q_NAMESPACE
    enum DataType : quint8
    {
        TinyInt,
        UTinyInt,
        SmallInt,
        USmallInt,
        Int,
        UInt,
        BigInt,
        UBigInt,
        Float,
        Double,
        Bool,
        Enum,
        Char,
        Varchar
    };
    Q_ENUM_NS(DataType)

    enum FileOrganization : quint8
    {
        Heap,
        Sequential,
        Hash,
        BPlusTree
    };
    Q_ENUM_NS(FileOrganization)

    enum Charset : quint8
    {
        Latin1,
        Utf8,
        Utf16,
        Utf32
    };
    Q_ENUM_NS(Charset)

    enum RecordFormat : quint8
    {
        Fixed,
        Variable
    };
    Q_ENUM_NS(RecordFormat)

    enum IndexType : quint8
    {
        // primary key default, but it can be nonCluster
        // ClusterIndex,
        // unique key default,
        // NonClusterIndex,
        SequentialIndex,
        HashIndex,
        BPlusTreeIndex
    };
    Q_ENUM_NS(IndexType)

    enum KeyConstraintType : quint8
    {
        Primary,    // doesn't allow nulls
        Unique,     // can allow nulls
        Index,      // can allow non-unique
        Foreign,
        None
    };
    Q_ENUM_NS(KeyConstraintType)

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

    enum Order : bool
    {
        DESC,
        ASC
    };
    Q_ENUM_NS(Order)

    // template specializations

    template <DataType T>
    struct DTAlias;

    template <>
    struct DTAlias<DataType::TinyInt> {
        using type = qint8;
    };

    template <>
    struct DTAlias<DataType::UTinyInt> {
        using type = quint8;
    };

    template <>
    struct DTAlias<DataType::SmallInt> {
        using type = qint16;
    };

    template <>
    struct DTAlias<DataType::USmallInt> {
        using type = quint16;
    };

    template <>
    struct DTAlias<DataType::Int> {
        using type = qint32;
    };

    template <>
    struct DTAlias<DataType::UInt> {
        using type = quint32;
    };

    template <>
    struct DTAlias<DataType::BigInt> {
        using type = qint64;
    };
    template <>
    struct DTAlias<DataType::UBigInt> {
        using type = quint64;
    };

    template <>
    struct DTAlias<DataType::Float> {
        using type = float;
    };
    template <>
    struct DTAlias<DataType::Double> {
        using type = double;
    };

    template <>
    struct DTAlias<DataType::Bool> {
        using type = bool;
    };

    template <>
    struct DTAlias<DataType::Enum> {
        using type = quint8;
    };

    template <>
    struct DTAlias<DataType::Char> {
        using type = QString;
    };

    template <>
    struct DTAlias<DataType::Varchar> {
        using type = QString;
    };
}

#endif // MEGATRON_TYPES_H
