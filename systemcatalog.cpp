#include "systemcatalog.h"

SystemCatalog::SystemCatalog(const QString &path)
    : dbDir(path)
{
    schemaPath = dbDir.filePath("schema.txt");
}

bool SystemCatalog::initSchema()
{
    // Read schema and load 'tables' if any
    QFile schema(schemaPath);
    if (schema.open(QIODevice::ReadOnly | QIODevice::Text) && schema.size() != 0) {
        QTextStream in(&schema);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split('#');
            QString tableName = parts.takeFirst();
            int pos = 0;
            for (int i = 0; i < parts.size(); i += 2) {
                attrMeta meta;
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
                tables.insert(tableName, meta);
                pos++;
            }
        }
        return true;
    }
    return false;
}

Types::Return SystemCatalog::parseSchemaPath(const QString &relName,
    const QString &header, const QString &schemaFile)
{
    // Parse newSchemaFile
    QFile newSchema(schemaFile);
    if (!newSchema.open(QIODevice::ReadOnly | QIODevice::Text))
        return Types::OpenError;
    QTextStream in(&newSchema);

    QString token;
    QStringList attrNames = header.split(",");
    for (auto& i : attrNames) {
        i.replace('"', QString());
        // qDebug() << i;
    }

    char c;
    char t;
    int l = 0;
    int p = 0;
    while (!in.atEnd())
    {
        in >> c;
        if (c == ',')
        {
            // p++;
            if (token == "int")             t = 'i';
            else if (token == "float")      t = 'f';
            else if (token == "double")     t = 'd';
            else if (token == "boolean" ||
                     token == "bool")       t = 'b';
            else if (token == "tinyint")    t = 't';
            else if (token == "char")       t = 'c';
            else if (token == "varchar")    t = 'v';
            else return Types::ParseError;
            // Comma is the main delimiter between data types, so insert metadata
            // int index = p - 1;
            QString attrName = attrNames.at(p);
            insertTableMetadata(attrName, relName, t, l, p);
            l = 0; p++;
            token.clear();
        }
        else if (c == '(')
        {
            QString length;
            in >> c;
            while (c != ')') {
                length += c;
                in >> c;
            }
            l = length.toInt();
        }
        else
            token += c;
    }
    // Handle last token
    if (!token.isEmpty()) {
        // p++;
        if (token == "int")             t = 'i';
        else if (token == "float")      t = 'f';
        else if (token == "double")     t = 'd';
        else if (token == "boolean" ||
                 token == "bool")       t = 'b';
        else if (token == "tinyint")    t = 't';
        else if (token == "char")       t = 'c';
        else if (token == "varchar")    t = 'v';
        else return Types::ParseError;
        // Comma is the main delimiter between data types, so insert metadata
        // int index = p - 1;
        QString attrName = attrNames.at(p);
        insertTableMetadata(attrName, relName, t, l, p);
        p++;
    }
    newSchema.close();
    // End of Parse
    return Types::Success;
}

void SystemCatalog::insertTableMetadata(const QString &an, const QString &tn, char t, int l, int p)
{
    attrMeta tm = { .attributeName = an, .type = t, .length = l, .position = p};
    // qDebug() << tm.attributeName << tm.type << tm.length << tm.position;
    tables.insert(tn, tm);
}

void SystemCatalog::insertTableMetadata(const QString &tn, const attrMeta &tm)
{
    tables.insert(tn, tm);
}

void SystemCatalog::writeToSchema(const QString &relName)
{
    QFile schema(schemaPath);
    schema.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&schema);
    out << relName;
    QList<attrMeta> values = tables.values(relName);
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

QString SystemCatalog::getSchemaPath() const
{
    return schemaPath;
}

QString SystemCatalog::getDbDirPath() const
{
    return dbDir.absolutePath();
}

QList<SystemCatalog::attrMeta> SystemCatalog::values(const QString &str)
{
    return tables.values(str);
}

QMultiMap<QString, SystemCatalog::attrMeta>::iterator SystemCatalog::find(const QString &str)
{
    return tables.find(str);
}

QMultiMap<QString, SystemCatalog::attrMeta>::iterator SystemCatalog::end()
{
    return tables.end();
}

QSet<QString> SystemCatalog::getTableNames() const
{
    QSet<QString> tableSet;
    for (const QString& table : tables.keys())
        if (!tableSet.contains(table))
            tableSet.insert(table);
    return tableSet;
}

int SystemCatalog::getSize(const QString &tableName)
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
