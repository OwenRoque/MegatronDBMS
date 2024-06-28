#ifndef SYSTEMCATALOG_H
#define SYSTEMCATALOG_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringView>
#include <QFile>
#include <QMultiMap>
#include <QList>
#include <QSharedPointer>
#include "megatron_types.h"

// SystemCatalog will be a Singleton
namespace Core
{
    constexpr Types::Charset Default = Types::Charset::Utf16;

    class SystemCatalog : public QObject
    {
        Q_OBJECT
    public:
        static SystemCatalog& getInstance(const QString& catalogPath = QString())
        {
            static SystemCatalog singleton(catalogPath);
            return singleton;
        }

        struct relationMeta
        {
            QString relationName;
            quint8 numberOfAttributes;
            Types::FileOrganization fileOrganization;
            Types::RecordFormat recordFormat;
            Types::Charset charset;
            quint64 autoIncrement;
            // fileGroupId
            quint64 location;
            friend bool operator==(const relationMeta& a, const relationMeta& b)
            {
                return a.relationName == b.relationName;
            }
            friend QDataStream &operator<<(QDataStream &out, const relationMeta &meta)
            {
                out << meta.relationName
                    << meta.numberOfAttributes
                    << meta.fileOrganization
                    << meta.recordFormat
                    << meta.charset
                    << meta.autoIncrement
                    << meta.location;
                return out;
            }
            friend QDataStream &operator>>(QDataStream &in, relationMeta &meta)
            {
                in >> meta.relationName
                    >> meta.numberOfAttributes
                    >> meta.fileOrganization
                    >> meta.recordFormat
                    >> meta.charset
                    >> meta.autoIncrement
                    >> meta.location;
                return in;
            }
        };
        auto insertRelation(const relationMeta&) -> QMap<QString, relationMeta>::iterator;
        auto deleteRelation(const QString&) -> QMap<QString, relationMeta>::size_type;
        auto findRelation(const QString&) -> QMap<QString, relationMeta>::iterator;
        auto constFindRelation(const QString&) const -> QMap<QString, relationMeta>::const_iterator;
        bool relationExists(const QString&) const;
        auto relationSize() const -> QMap<QString, relationMeta>::size_type;
        auto listRelationKeys() const -> QList<QString>;
        auto listRelationValues() const -> QList<relationMeta>;
        // QMap useful methods to access to relations ...

        struct attributeMeta
        {
            QString attributeName;
            QString relationName;
            Types::DataType dataType;
            QString columnType;
            quint16 maxCharacterLength;
            quint32 maxByteLength;
            QString defaultValue;
            quint8 ordinalPosition;
            bool isNullable;
            bool isUnsigned;
            bool autoIncrement;
            Types::KeyConstraintType key;
            QString comment;
            friend bool operator==(const attributeMeta& a, const attributeMeta& b)
            {
                return (a.attributeName == b.attributeName) &&
                       (a.relationName == b.relationName);
            }
            friend bool operator<(const attributeMeta& a, const attributeMeta& b)
            {
                return a.ordinalPosition < b.ordinalPosition;
            }
            friend QDataStream &operator<<(QDataStream &out, const attributeMeta &meta)
            {
                out << meta.attributeName
                    << meta.relationName
                    << meta.dataType
                    << meta.columnType
                    << meta.maxCharacterLength
                    << meta.maxByteLength
                    << meta.defaultValue
                    << meta.ordinalPosition
                    << meta.isNullable
                    << meta.isUnsigned
                    << meta.autoIncrement
                    << meta.key
                    << meta.comment;
                return out;
            }
            friend QDataStream &operator>>(QDataStream &in, attributeMeta &meta)
            {
                in >> meta.attributeName
                    >> meta.relationName
                    >> meta.dataType
                    >> meta.columnType
                    >> meta.maxCharacterLength
                    >> meta.maxByteLength
                    >> meta.defaultValue
                    >> meta.ordinalPosition
                    >> meta.isNullable
                    >> meta.isUnsigned
                    >> meta.autoIncrement
                    >> meta.key
                    >> meta.comment;
                return in;
            }
        };
        auto insertAttribute(const attributeMeta&) -> QMultiMap<QString, attributeMeta>::iterator;
        auto deleteAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::size_type;
        auto findAttributesFor(const QString&) -> std::pair<QMultiMap<QString, attributeMeta>::iterator, QMultiMap<QString, attributeMeta>::iterator>;
        auto constFindAttributesFor(const QString&) const -> std::pair<QMultiMap<QString, attributeMeta>::const_iterator, QMultiMap<QString, attributeMeta>::const_iterator>;
        auto numberOfAttributes(const QString&) const -> QMultiMap<QString, attributeMeta>::size_type;
        auto findAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::iterator;
        auto constFindAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::const_iterator;
        void updateOrdinalPositions(const QString&);
        // QMultimap useful methods to access to attributes ...

        struct charsetMeta
        {
            Types::Charset charset;
            QString charsetName;
            QString description;
            quint8 maxlen;
            friend QDataStream &operator<<(QDataStream &out, const charsetMeta &meta)
            {
                out << meta.charset
                    << meta.charsetName
                    << meta.description
                    << meta.maxlen;
                return out;
            }
            friend QDataStream &operator>>(QDataStream &in, charsetMeta &meta)
            {
                in >> meta.charset
                    >> meta.charsetName
                    >> meta.description
                    >> meta.maxlen;
                return in;
            }
        };
        auto findCharset(const Types::Charset&) -> QMap<Types::Charset, charsetMeta>::iterator;
        auto constFindCharset(const Types::Charset&) const -> QMap<Types::Charset, charsetMeta>::const_iterator;
        // QMap useful methods to access to charsets ...

        struct indexMeta
        {
            QString indexName;
            QString relationName;
            QString attributeName;
            Types::IndexType indexType;
            bool isNonUnique;
            bool isNullable;
            bool isClustered;
            Types::Order ordering;
            quint8 ordinalPosition;
            QString comment;
            QString referencedRelation;
            QString referencedAttribute;
            bool operator==(const indexMeta& other) const {
                return (relationName == other.relationName) &&
                       (attributeName == other.attributeName) &&
                       (indexName == other.indexName) &&
                       (indexType == other.indexType);
            }
            // Compares if they are in the same relation and have the same name and index type, but different attributes.
            bool isPartOfCompositeIndex(const indexMeta& other) const {
                return (relationName == other.relationName) &&
                       (indexName == other.indexName) &&
                       (indexType == other.indexType) &&
                       (attributeName != other.attributeName);
            }
            bool operator<(const indexMeta& a) const {
                if (relationName == a.relationName) {
                    if (attributeName == a.attributeName) {
                        if (indexName == a.indexName) {
                            return indexType < a.indexType;
                        }
                        return indexName < a.indexName;
                    }
                    return attributeName < a.attributeName;
                }
                return relationName < a.relationName;
            }

            // comparison with strings
            static bool comp_index_str(const indexMeta& a, const QString& s) {
                return a.indexName < s;
            }
            static bool comp_str_index(const QString& s, const indexMeta& a) {
                return s < a.indexName;
            }
            friend QDataStream &operator<<(QDataStream &out, const indexMeta &meta)
            {
                out << meta.indexName
                    << meta.relationName
                    << meta.attributeName
                    << meta.indexType
                    << meta.isNonUnique
                    << meta.isNullable
                    << meta.isClustered
                    << meta.ordering
                    << meta.ordinalPosition
                    << meta.comment
                    << meta.referencedRelation
                    << meta.referencedAttribute;
                return out;
            }
            friend QDataStream &operator>>(QDataStream &in, indexMeta &meta)
            {
                in >> meta.indexName
                    >> meta.relationName
                    >> meta.attributeName
                    >> meta.indexType
                    >> meta.isNonUnique
                    >> meta.isNullable
                    >> meta.isClustered
                    >> meta.ordering
                    >> meta.ordinalPosition
                    >> meta.comment
                    >> meta.referencedRelation
                    >> meta.referencedAttribute;
                return in;
            }
        };
        auto insertIndex(indexMeta&) -> QMultiMap<QString, indexMeta>::iterator;
        auto deleteIndex(const QString&, const QString&) -> QMultiMap<QString, indexMeta>::size_type;
        auto findIndexes(const QString&, const QString&) -> std::pair<QMultiMap<QString, indexMeta>::iterator, QMultiMap<QString, indexMeta>::iterator>;
        auto constFindIndexes(const QString&, const QString&) const -> std::pair<QMultiMap<QString, indexMeta>::const_iterator, QMultiMap<QString, indexMeta>::const_iterator>;
        auto numberOfIndexes(const QString&) const -> QMultiMap<QString, indexMeta>::size_type;
        // QMultimap useful methods to access to indexes ...

        // fix these and that's it
        bool initSchema();
        void writeToSchema(const QString &);
        // persistence
        bool saveOnDisk();
        bool readFromDisk();

    private:
        SystemCatalog(const QString& catalogPath = QString());

        QMap<QString, relationMeta> relations;
        QMultiMap<QString, attributeMeta> attributes;
        QMap<Types::Charset, charsetMeta> charsets;
        QMultiMap<QString, indexMeta> indexes;
        QString catalogFile;

        // QString schemaPath;
        Q_DISABLE_COPY(SystemCatalog)
    };

}
#endif // SYSTEMCATALOG_H
