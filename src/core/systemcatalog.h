#ifndef SYSTEMCATALOG_H
#define SYSTEMCATALOG_H

#include "megatron_types.h"
#include <diskcontroller.h> // removal in the future, storageManager handles it

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringView>
#include <QFile>
#include <QMultiMap>
#include <QList>
#include <QSharedPointer>

// SystemCatalog will be a Singleton
namespace Core
{
    constexpr Types::Charset Default = Types::Charset::Utf16;

    class SystemCatalog : public QObject
    {
        Q_OBJECT
    public:
        static SystemCatalog& getInstance()
        {
            static SystemCatalog singleton;
            return singleton;
        }

        struct charsetMeta
        {
            Types::Charset charset;
            QString charsetName;
            QString description;
            quint8 maxlen;
        };

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
        };

        // ADD to relations/attributes
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
            QString referencedRelation;     // for foreign only
            QString referencedAttribute;    // for foreign only
        };

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

        auto insertAttribute(const attributeMeta&) -> QMultiMap<QString, attributeMeta>::iterator;
        auto deleteAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::size_type;
        auto findAttributesFor(const QString&) -> std::pair<QMultiMap<QString, attributeMeta>::iterator, QMultiMap<QString, attributeMeta>::iterator>;
        auto constFindAttributesFor(const QString&) const -> std::pair<QMultiMap<QString, attributeMeta>::const_iterator, QMultiMap<QString, attributeMeta>::const_iterator>;
        auto numberOfAttributes(const QString&) const -> QMultiMap<QString, attributeMeta>::size_type;
        auto findAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::iterator;
        auto constFindAttribute(const QString&, const QString&) -> QMultiMap<QString, attributeMeta>::const_iterator;
        void updateOrdinalPositions(const QString&);
        // QMultimap useful methods to access to attributes ...

        auto findCharset(const Types::Charset&) -> QMap<Types::Charset, charsetMeta>::iterator;
        auto constFindCharset(const Types::Charset&) const -> QMap<Types::Charset, charsetMeta>::const_iterator;
        // QMap useful methods to access to charsets ...

        // fix these and that's it
        bool initSchema();
        void writeToSchema(const QString &);
        QSharedPointer<Storage::DiskController> getDiskController() const;

        void saveOnDisk();
        void readFromDisk();

    private:
        SystemCatalog();
        // For retrieving multiple values with same key
        // <tableName, struct>
        QMultiMap<QString, attributeMeta> attributes;
        QMap<QString, relationMeta> relations;
        QMap<Types::Charset, charsetMeta> charsets;

        QSharedPointer<Storage::DiskController> controller;
        // QString schemaPath;
        Q_DISABLE_COPY(SystemCatalog)
    };

}
#endif // SYSTEMCATALOG_H
