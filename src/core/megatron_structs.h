#ifndef MEGATRON_STRUCTS_H
#define MEGATRON_STRUCTS_H

#include <QString>
#include <QList>
#include <tuple>
#include <QDebug>
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
        void print() {
            qDebug() << relationName;
            qDebug() << "Attributes:";
            for (const auto& i : attributes) {
                QString attrName;
                Types::DataType type;
                QString columnType;
                quint16 maxCharLength;
                QString defaultValue;
                bool unsignedValue;
                bool nullableValue;
                bool autoIncrementValue;
                Types::KeyConstraintType keyConstraint;
                QString comment;
                std::tie(attrName, type, columnType, maxCharLength, defaultValue, unsignedValue,
                         nullableValue, autoIncrementValue, keyConstraint, comment) = i;

                qDebug() << attrName << columnType << maxCharLength << unsignedValue
                         << nullableValue << autoIncrementValue << keyConstraint << comment;
            }
            qDebug() << "Indexes:";
            for (const auto& i : indexes) {
                QString indexName;
                QString attributeName;
                Types::IndexType indexType;
                Core::IndexProperties idxProperties;

                // Desempaquetar la tupla en variables individuales
                std::tie(indexName, attributeName, indexType, idxProperties) = i;
                qDebug() << indexName << attributeName << indexType << idxProperties.isNullable
                         << idxProperties.isClustered << idxProperties.isNonUnique;
            }
        }
    };

}

#endif // MEGATRON_STRUCTS_H
