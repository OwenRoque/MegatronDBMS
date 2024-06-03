#include "systemcatalog.h"

Core::SystemCatalog::SystemCatalog() {}

QMap<QString, Core::SystemCatalog::relationMeta> Core::SystemCatalog::getRelations() const
{
    return relations;
}

QMultiMap<QString, Core::SystemCatalog::attributeMeta> Core::SystemCatalog::getAttributes() const
{
    return attributes;
}

bool Core::SystemCatalog::initSchema()
{
    // Read schema and load 'tables' if any
    // create serialize/deserialize and call them, this method is deprecated!!
    QFile schema(schemaPath);
    if (schema.open(QIODevice::ReadOnly | QIODevice::Text) && schema.size() != 0) {
        QTextStream in(&schema);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split('#');
            QString tableName = parts.takeFirst();
            int pos = 0;
            for (int i = 0; i < parts.size(); i += 2) {
                attributeMeta meta;
                // meta.tableName = tableName;
                meta.attributeName = parts.at(i);
                meta.type = parts.at(i + 1).at(0).toLatin1();
                meta.position = pos;
                meta.length = 0;
                if (meta.type == 'c' || meta.type == 'v') {
                    int start = parts[i + 1].indexOf('(') + 1;
                    int end = parts[i + 1].indexOf(')');
                    meta.length = QStringView{parts[i + 1]}.mid(start, end - start).toInt();
                }
                attributes.insert(tableName, meta);
                pos++;
            }
        }
        return true;
    }
    return false;
}

void Core::SystemCatalog::insertRelationMetadata(const QString &rn, int n, Types::FileOrganization fo, Types::RecordFormat rf, int loc)
{
    relationMeta rm =
    {
        .relationName = rn,
        .numberOfAttributes = n,
        .fileOrganization = fo,
        .recordFormat = rf,
        .location = loc
    };
    relations.insert(rn, rm);
}

void Core::SystemCatalog::insertAttributeMetadata(const QString &rn, const QString &an, Types::DataType t, int len, int pos, bool null, bool ai)
{
    attributeMeta tm =
    {
        .attributeName = an,
        .relationName = rn,
        .type = t,
        .length = len,
        .position = pos,
        .isNull = null,
        .autoIncrement = ai
    };
    // qDebug() << tm.attributeName << tm.type << tm.length << tm.position;
    attributes.insert(rn, tm);
}

// When pressed 'save' button
void Core::SystemCatalog::writeToSchema(const QString &relName)
{
    // write to catalog.bin
    QFile schema(schemaPath);
    schema.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&schema);
    out << relName;
    QList<attributeMeta> values = attributes.values(relName);
    // sort by position before writing
    // (even if previously inserted in order, they are not guaranteed
    // to be retrieved in the same order)
    // std::sort(values.begin(), values.end(), [](const attrMeta &a, const attrMeta &b) {
    //     return a.position < b.position;
    // });
    std::reverse(values.begin(), values.end());
    QMap<char, QString> datatypeNames;
    datatypeNames['i'] = "int";
    datatypeNames['f'] = "float";
    datatypeNames['d'] = "double";
    datatypeNames['b'] = "bool";
    datatypeNames['t'] = "tinyint";
    datatypeNames['c'] = "char";
    datatypeNames['v'] = "varchar";
    for (const auto& i : std::as_const(values)) {
        QString typeName = datatypeNames.value(i.type);
        QString line;
        if (typeName == "char" || typeName == "varchar")
            line = QString("#%1#%2(%3)").arg(i.attributeName, typeName).arg(i.length);
        else
            line = QString("#%1#%2").arg(i.attributeName, typeName);
        out << line;
    }
    out << Qt::endl;
    schema.close();
}

QSharedPointer<Storage::DiskController> Core::SystemCatalog::getDiskController() const
{
    return controller;
}

QList<Core::SystemCatalog::attributeMeta> Core::SystemCatalog::values(const QString &str)
{
    return attributes.values(str);
}

QMultiMap<QString, Core::SystemCatalog::attributeMeta>::iterator Core::SystemCatalog::findInAttributes(const QString &value)
{
    return attributes.find(value);
}

QMap<QString, Core::SystemCatalog::relationMeta>::const_iterator Core::SystemCatalog::relationConstFind(const QString &value) const
{
    return relations.find(value);
}

QMultiMap<QString, Core::SystemCatalog::attributeMeta>::iterator Core::SystemCatalog::end()
{
    return attributes.end();
}

QSet<QString> Core::SystemCatalog::getTableNames() const
{
    QSet<QString> tableSet;
    for (const QString& table : attributes.keys())
        if (!tableSet.contains(table))
            tableSet.insert(table);
    return tableSet;
}

int Core::SystemCatalog::getSize(const QString &tableName)
{
    // QList<attrMeta> values = tables.values(tableName);
    // for (const auto& i : std::as_const(values)) {
    //     switch (i.type) {
    //     case 'i': case 'f':
    //         break;
    //     }
    // }
    int size = 0;
    // QFile file(dbDir.filePath(tableName));
    return size;
}

void Core::SystemCatalog::saveOnDisk()
{
    QFile file(catalogFile);
    if (!file.open(QIODevice::WriteOnly))
        return;
    QDataStream out(&file);
    // out << attributes;
    // out << cylinderGroups;
    file.close();
}

void Core::SystemCatalog::readFromDisk()
{
    QFile file(catalogFile);
    if (!file.open(QIODevice::ReadOnly))
        return;
    // Clear current default values
    // cylinderGroups.clear();
    QDataStream in(&file);
    // in >> sib;
    // in >> cylinderGroups;
    file.close();
}
