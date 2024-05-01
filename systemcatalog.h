#ifndef SYSTEMCATALOG_H
#define SYSTEMCATALOG_H

#include "megatron_types.h"

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringView>
#include <QFile>
#include <QMultiMap>
#include <QList>

// SystemCatalog will be a Singleton

class SystemCatalog : public QObject
{
    Q_OBJECT
public:
    static SystemCatalog& getInstance(const QString &dbDir = QString())
    {
        static SystemCatalog singleton(dbDir);
        return singleton;
    }
    struct attrMeta {
        QString attributeName;          // self explanatoty - 50bytes (limit)
        // QString tableName;              // self explanatory - 50bytes (limit)
        char type;                      // data type (int, char, etc) - 1byte
        int length;                     // applies only for char/varchar, max length permitted. 0 otherwise - 4bytes
        int position;                   // column position in table - 4bytes
    };                                  // 109 b total

    bool initSchema();

    Types::Return parseSchemaPath(const QString &, const QString&, const QString &);
    void insertTableMetadata(const QString&, const QString&, char, int, int);
    void insertTableMetadata(const QString&, const attrMeta&);
    void writeToSchema(const QString &);
    QString getSchemaPath() const;
    QString getDbDirPath() const;
    QList<SystemCatalog::attrMeta> values(const QString &);
    QMultiMap<QString, SystemCatalog::attrMeta>::iterator find(const QString &);
    QMultiMap<QString, SystemCatalog::attrMeta>::iterator end();

    QSet<QString> getTableNames() const;
    int getSize(const QString &);

private:
    SystemCatalog(const QString &dbDir = QString());
    // For retrieving multiple values with same key
    // <tableName, struct>
    QMultiMap<QString, attrMeta> tables;
    QDir dbDir;
    QString schemaPath;
    Q_DISABLE_COPY(SystemCatalog)
};

#endif // SYSTEMCATALOG_H
