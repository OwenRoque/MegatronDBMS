#ifndef MEGATRON_STRUCTS_H
#define MEGATRON_STRUCTS_H

#include <QString>
#include <QList>
#include <tuple>
#include "megatron_types.h"

namespace Core
{
    struct IndexProperties
    {
        bool isNonUnique;
        bool isNullable;
        bool isClustered;
        Types::Order order;                // default ASC
        quint8 ordinalPosition;
        QString comment;
    };

    struct RelationInput
    {
        QString relationName;
        // attributeName, dataType, columnType, maxCharLen, defaultValue, nullable, unsigned, ai, constraint, comment
        // missing attribute properties will be calculated internally, or are already included in this struct
        QList<std::tuple<QString, Types::DataType, QString, quint16, QString, bool, bool,
                         bool, Types::KeyConstraintType, QString>> attributes;
        // indexName, attributeName, indexType, idxProperties
        QList<std::tuple<QString, QString, Types::IndexType, Core::IndexProperties>> indexes;
        Types::FileOrganization fileOrg;
        Types::RecordFormat recFormat;
        Types::Charset charset;
        QString dataPath;
    };

}

#endif // MEGATRON_STRUCTS_H
