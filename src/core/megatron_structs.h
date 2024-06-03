#ifndef MEGATRON_STRUCTS_H
#define MEGATRON_STRUCTS_H

#include <QString>
#include <QList>
#include <tuple>
#include "megatron_types.h"

namespace Core
{
    struct IndexProperties;

    struct RelationInput
    {
        QString relationName;
        // attributeMeta
        QList<std::tuple<QString, Types::DataType, int, bool, int>> attributes;
        QList<std::tuple<QString, QString, Types::IndexType, Core::IndexProperties>> indexes;
        Types::FileOrganization fileOrg;
        Types::RecordFormat recFormat;
        QString dataPath;
    };

    struct IndexProperties
    {
        QStringList columns;                // single or composite
        Types::KeyType keyType;
        enum Order : quint8 { ASC, DESC };
        QList<Order> columnOrders;          // default ASC
    };
}

#endif // MEGATRON_STRUCTS_H
