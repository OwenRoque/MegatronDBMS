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
    class SystemCatalog : public QObject
    {
        Q_OBJECT
    public:
        static SystemCatalog& getInstance()
        {
            static SystemCatalog singleton;
            return singleton;
        }

        struct relationMeta
        {
            QString relationName;
            int numberOfAttributes;
            Types::FileOrganization fileOrganization;
            Types::RecordFormat recordFormat;
            // fileGroupId
            int location;
        };

        struct attributeMeta
        {
            QString attributeName;          // self explanatoty - 50bytes (limit)
            QString relationName;           // self explanatory - 50bytes (limit)
            Types::DataType type;           // data type (int, char, etc) - 1byte
            int length;                     // applies only for char/varchar, max length permitted. 0 otherwise - 4bytes
            int position;                   // column position in table - 4bytes
            bool isNull;                    // to allow null values or not - 1byte
            int autoIncrement;             // to autoincrement its value
        };                                  // 109 b total

        QMap<QString, relationMeta> getRelations() const;
        QMultiMap<QString, attributeMeta> getAttributes() const;


        bool initSchema();

        // Types::Return parseSchemaFile(const QString &, const QString&, const QString &);
        void insertRelationMetadata(const QString &, int, Types::FileOrganization, Types::RecordFormat, int);
        void insertAttributeMetadata(const QString &, const QString &, Types::DataType, int, int, bool, bool);


        void writeToSchema(const QString &);
        QString getSchemaPath() const;
        QString getDbDirPath() const;
        QSharedPointer<Storage::DiskController> getDiskController() const;
        QList<SystemCatalog::attributeMeta> values(const QString&);
        QMultiMap<QString, SystemCatalog::attributeMeta>::iterator findInAttributes(const QString &);

        QMultiMap<QString, SystemCatalog::attributeMeta>::iterator end();

        QSet<QString> getTableNames() const;
        int getSize(const QString &);
        void saveOnDisk();
        void readFromDisk();

    private:
        SystemCatalog();
        // For retrieving multiple values with same key
        // <tableName, struct>
        QMultiMap<QString, attributeMeta> attributes;
        QMap<QString, relationMeta> relations;
        QSharedPointer<Storage::DiskController> controller;
        // QString schemaPath;
        Q_DISABLE_COPY(SystemCatalog)
    };

}
#endif // SYSTEMCATALOG_H
