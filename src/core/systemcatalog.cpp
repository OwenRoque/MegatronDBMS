#include "systemcatalog.h"

Core::SystemCatalog::SystemCatalog()
{
    // init tables/maps with self-data description
    // this data won't be stored on disk, but in the database code itself (pg. 604)
    // new rows WILL be stored on disk, in a specific FileGroup
    // relations
    relationMeta relations {
        .relationName = "relations",
        .numberOfAttributes = 6,
        .fileOrganization = Types::FileOrganization::Heap,
        .recordFormat = Types::RecordFormat::Variable,
        .charset = Default,
        .location = 1
    };
    this->relations.insert("relations", relations);
    relationMeta attributes {
        .relationName = "attributes",
        .numberOfAttributes = 12,
        .fileOrganization = Types::FileOrganization::Heap,
        .recordFormat = Types::RecordFormat::Variable,
        .charset = Default,
        .location = 2
    };
    this->relations.insert("attributes", attributes);
    relationMeta charsets {
        .relationName = "charsets",
        .numberOfAttributes = 4,
        .fileOrganization = Types::FileOrganization::Heap,
        .recordFormat = Types::RecordFormat::Variable,
        .charset = Default,
        .location = 3
    };
    this->relations.insert("charsets", charsets);

    // charsets
    charsetMeta latin1 {
        .charset = Types::Charset::Latin1,
        .charsetName = "Latin1",
        .description = "The charset Latin1, or ISO-8859-1, is a character encoding that encompasses "
                       "basic Latin characters, including letters, numbers and Western European symbols.",
        .maxlen = 1
    };
    this->charsets.insert(Types::Charset::Latin1, latin1);
    charsetMeta utf8 {
        .charset = Types::Charset::Utf8,
        .charsetName = "UTF-8",
        .description = "UTF-8 is a variable-width character encoding that can represent every character "
                       "in the Unicode standard, supporting global text representation.",
        .maxlen = 4
    };
    this->charsets.insert(Types::Charset::Utf8, utf8);
    charsetMeta utf16 {
        .charset = Types::Charset::Utf16,
        .charsetName = "UTF-16",
        .description = "UTF-16 is a variable-length encoding that uses 16-bit code units to represent "
                       "characters from the Unicode standard, supporting extensive multilingual text.",
        .maxlen = 4
    };
    this->charsets.insert(Types::Charset::Utf16, utf16);
    charsetMeta utf32 {
        .charset = Types::Charset::Utf32,
        .charsetName = "UTF-32",
        .description = "UTF-32 is a fixed-length encoding that uses 32-bit code units to represent each "
                       "character in the Unicode standard, ensuring straightforward character representation.",
        .maxlen = 4
    };
    this->charsets.insert(Types::Charset::Utf32, utf32);

    // attributes (a lot)
    this->attributes.insert("relations", {
        .attributeName = "relationName",
        .relationName = "relations",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(512 * this->charsets.find(Default)->maxlen),
        .ordinalPosition = 1,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::Primary,
        .comment = "The name of the table/relation."
    });
    this->attributes.insert("relations", {
        .attributeName = "numberOfAttributes",
        .relationName = "relations",
        .dataType = Types::DataType::UTinyInt,
        .columnType = "tinyint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 2,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Number of attributes in the table/relation."
    });
    this->attributes.insert("relations", {
        .attributeName = "fileOrganization",
        .relationName = "relations",
        .dataType = Types::DataType::Enum,
        .columnType = "Heap, Sequential, Hash, BPlusTree",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 3,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "File organization on disk."
    });
    this->attributes.insert("relations", {
        .attributeName = "recordFormat",
        .relationName = "relations",
        .dataType = Types::DataType::Enum,
        .columnType = "Fixed, Variable",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 4,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Row/record storage format."
    });
    this->attributes.insert("relations", {
        .attributeName = "charset",
        .relationName = "relations",
        .dataType = Types::DataType::Enum,
        .columnType = "Latin1, Utf8, Utf16, Utf32",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 5,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::Foreign,
        .comment = "Character set of the table/relation."
    });
    this->attributes.insert("relations", {
        .attributeName = "autoIncrement",
        .relationName = "relations",
        .dataType = Types::DataType::UBigInt,
        .columnType = "bigint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint64),
        .ordinalPosition = 6,
        .isNullable = true,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "FileGroup ID to lookup in Disk Manager."
     });
    this->attributes.insert("relations", {
        .attributeName = "location",
        .relationName = "relations",
        .dataType = Types::DataType::UBigInt,
        .columnType = "bigint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint64),
        .ordinalPosition = 7,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "FileGroup ID to lookup in Disk Manager."
    });
    this->attributes.insert("attributes", {
        .attributeName = "attributeName",
        .relationName = "attributes",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(512 * this->charsets.find(Default)->maxlen),
        .ordinalPosition = 1,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Name of the column/attribute."
    });
    this->attributes.insert("attributes", {
        .attributeName = "relationName",
        .relationName = "attributes",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(512 * this->charsets.find(Default)->maxlen),
        .ordinalPosition = 2,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::Index,
        .comment = "The name of the table/relation containing the column/attribute."
    });
    this->attributes.insert("attributes", {
        .attributeName = "dataType",
        .relationName = "attributes",
        .dataType = Types::DataType::Enum,
        .columnType = "TinyInt, UTinyInt, SmallInt, USmallInt, Int, UInt, "
                      "BigInt, UBigInt, Float, Double, Bool, Enum, Char, Varchar",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 3,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The column/attribute data type."
    });
    this->attributes.insert("attributes", {
        .attributeName = "columnType",
        .relationName = "attributes",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(512 * this->charsets.find(Default)->maxlen),
        .ordinalPosition = 4,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The column/attribute data type."
    });
    this->attributes.insert("attributes", {
        .attributeName = "maxCharacterLength",
        .relationName = "attributes",
        .dataType = Types::DataType::USmallInt,
        .columnType = "smallint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint16),
        .ordinalPosition = 5,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "For string columns, the maximum length in characters."
    });
    this->attributes.insert("attributes", {
        .attributeName = "maxByteLength",
        .relationName = "attributes",
        .dataType = Types::DataType::UInt,
        .columnType = "int",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint32),
        .ordinalPosition = 6,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "For string columns, the maximum length in bytes."
    });
    this->attributes.insert("attributes", {
        .attributeName = "defaultValue",
        .relationName = "attributes",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(512 * this->charsets.find(Default)->maxlen),
        .ordinalPosition = 7,
        .isNullable = true,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Default value for this field."
    });
    this->attributes.insert("attributes", {
        .attributeName = "ordinalPosition",
        .relationName = "attributes",
        .dataType = Types::DataType::UTinyInt,
        .columnType = "tinyint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 8,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The position of the column within the table."
    });
    this->attributes.insert("attributes", {
        .attributeName = "isNullable",
        .relationName = "attributes",
        .dataType = Types::DataType::Bool,
        .columnType = "bool",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(bool),
        .ordinalPosition = 9,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The column nullability. The value is 'true' if NULL values can be stored in the column, 'false' if not."
    });
    this->attributes.insert("attributes", {
        .attributeName = "isUnsigned",
        .relationName = "attributes",
        .dataType = Types::DataType::Bool,
        .columnType = "bool",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(bool),
        .ordinalPosition = 10,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "If specified, disallows negative values. Deprecated for DECIMAL types (float, double)."
    });
    this->attributes.insert("attributes", {
        .attributeName = "autoIncrement",
        .relationName = "attributes",
        .dataType = Types::DataType::Bool,
        .columnType = "bool",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(bool),
        .ordinalPosition = 11,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "It can be used to generate a unique identity for new rows. "
                   "Only available for PRIMARY KEY or UNIQUE indexes."
    });
    this->attributes.insert("attributes", {
        .attributeName = "key",
        .relationName = "attributes",
        .dataType = Types::DataType::Enum,
        .columnType = "Primary, Unique, Index, Foreign, None",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 12,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Whether the column is indexed."
    });
    this->attributes.insert("attributes", {
        .attributeName = "comment",
        .relationName = "attributes",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(QString("comment").toUtf8().size() + 4),
        .ordinalPosition = 13,
        .isNullable = true,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "Any comment included in the column/attribute definition."
    });
    this->attributes.insert("charsets", {
        .attributeName = "charset",
        .relationName = "charsets",
        .dataType = Types::DataType::Enum,
        .columnType = "Latin1, Utf8, Utf16, Utf32",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 1,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::Primary,
        .comment = "Charset ID."
    });
    this->attributes.insert("charsets", {
        .attributeName = "charsetName",
        .relationName = "charsets",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(QString("charsetName").toUtf8().size() + 4),
        .ordinalPosition = 2,
        .isNullable = false,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The character set name."
    });
    this->attributes.insert("charsets", {
        .attributeName = "description",
        .relationName = "charsets",
        .dataType = Types::DataType::Varchar,
        .columnType = "varchar(512)",
        .maxCharacterLength = 512,
        .maxByteLength = static_cast<quint32>(QString("description").toUtf8().size() + 4),
        .ordinalPosition = 3,
        .isNullable = true,
        .isUnsigned = false,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "A description of the character set."
    });
    this->attributes.insert("charsets", {
        .attributeName = "maxlen",
        .relationName = "charsets",
        .dataType = Types::DataType::UTinyInt,
        .columnType = "tinyint",
        .maxCharacterLength = 0,
        .maxByteLength = sizeof(quint8),
        .ordinalPosition = 4,
        .isNullable = false,
        .isUnsigned = true,
        .autoIncrement = false,
        .key = Types::KeyConstraintType::None,
        .comment = "The maximum number of bytes required to store one character."
    });
}

auto Core::SystemCatalog::insertRelation(const Core::SystemCatalog::relationMeta& rm)
    -> QMap<QString, relationMeta>::iterator
{
    // since duplicate keys insertion is handled differently (erase old duplicate)
    // if a duplicate key is found, any value won't be inserted
    if (relationExists(rm.relationName))
        return relations.end();
    return relations.insert(rm.relationName, rm);
}

auto Core::SystemCatalog::deleteRelation(const QString &relationName)
    -> QMap<QString, relationMeta>::size_type
{
    return relations.remove(relationName);
}

auto Core::SystemCatalog::findRelation(const QString &relationName)
    -> QMap<QString, relationMeta>::iterator
{
    return relations.find(relationName);
}

auto Core::SystemCatalog::constFindRelation(const QString &relationName) const
    -> QMap<QString, relationMeta>::const_iterator
{
    return relations.constFind(relationName);
}

bool Core::SystemCatalog::relationExists(const QString &relationName) const
{
    return relations.contains(relationName);
}

auto Core::SystemCatalog::relationSize() const
    -> QMap<QString, relationMeta>::size_type
{
    return relations.size();
}

auto Core::SystemCatalog::listRelationKeys() const
    -> QList<QString>
{
    return relations.keys();
}

auto Core::SystemCatalog::listRelationValues() const
    -> QList<relationMeta>
{
    return relations.values();
}

auto Core::SystemCatalog::insertAttribute(const Core::SystemCatalog::attributeMeta &am)
    -> QMultiMap<QString, Core::SystemCatalog::attributeMeta>::iterator
{
    // attribute insertion is generally done in order, managed by the database,
    // so there's no need to sort them or update their positions
    auto attribute = findAttribute(am.relationName, am.attributeName);

    if (attribute == attributes.end()) {
        findRelation(am.relationName)->numberOfAttributes++;
        return attributes.insert(am.relationName, am);
    }
    else return attribute;
}

auto Core::SystemCatalog::deleteAttribute(const QString &relationName, const QString &attributeName)
    -> QMultiMap<QString, attributeMeta>::size_type
{
    auto predicate = [&relationName, &attributeName](const auto &value) {
        return value.key() == relationName && value.value().attributeName == attributeName;
    };
    qsizetype itemsDeleted = erase_if(attributes, predicate);
    // attribute deletion alters ordinalPosition of other attributes
    // position update & sorting are required
    updateOrdinalPositions(relationName);
    findRelation(relationName)->numberOfAttributes--;
    return itemsDeleted;
}

auto Core::SystemCatalog::findAttributesFor(const QString &relationName)
    -> std::pair<QMultiMap<QString, attributeMeta>::iterator, QMultiMap<QString, attributeMeta>::iterator>
{
    return attributes.equal_range(relationName);
}

auto Core::SystemCatalog::constFindAttributesFor(const QString &relationName) const
    -> std::pair<QMultiMap<QString, attributeMeta>::const_iterator, QMultiMap<QString, attributeMeta>::const_iterator>
{
    return attributes.equal_range(relationName);
}

auto Core::SystemCatalog::numberOfAttributes(const QString &relationName) const
    -> QMultiMap<QString, attributeMeta>::size_type
{
    return attributes.count(relationName);
}

auto Core::SystemCatalog::findAttribute(const QString &relationName, const QString &attributeName)
    -> QMultiMap<QString, attributeMeta>::iterator
{
    auto range = findAttributesFor(relationName);
    auto i = std::find_if(range.first, range.second, [&attributeName](auto &v) {
        return v.attributeName == attributeName; // v.second
    });
    return (i != range.second) ? i : attributes.end();
}

auto Core::SystemCatalog::constFindAttribute(const QString &relationName, const QString &attributeName)
    -> QMultiMap<QString, attributeMeta>::const_iterator
{
    auto range = constFindAttributesFor(relationName);
    auto i = std::find_if(range.first, range.second, [&attributeName](const auto &v) {
        return v.attributeName == attributeName;
    });
    return (i != range.second) ? i : attributes.end();
}

void Core::SystemCatalog::updateOrdinalPositions(const QString &relationName)
{
    auto range = findAttributesFor(relationName);

    QList<QMultiMap<QString, attributeMeta>::iterator> iterators;
    for (auto it = range.first; it != range.second; ++it) {
        iterators.push_back(it);
    }

    // Sort the vector by the position field using the iterators
    std::sort(iterators.begin(), iterators.end(), [](const auto &a, const auto &b) {
        return a.value().ordinalPosition < b.value().ordinalPosition;
    });

    // Update ordinalPosition based on order
    int ordinal = 1;
    for (auto &it : iterators) {
        it->ordinalPosition = ordinal++;
    }
}

auto Core::SystemCatalog::findCharset(const Types::Charset& charset)
    -> QMap<Types::Charset, charsetMeta>::iterator
{
    return charsets.find(charset);
}

auto Core::SystemCatalog::constFindCharset(const Types::Charset& charset) const
    -> QMap<Types::Charset, charsetMeta>::const_iterator
{
    return charsets.find(charset);
}

bool Core::SystemCatalog::initSchema()
{
    // Read schema and load 'tables' if any
    // create serialize/deserialize and call them, this method is deprecated!!
    // QFile schema(schemaPath);
    // if (schema.open(QIODevice::ReadOnly | QIODevice::Text) && schema.size() != 0) {
    //     QTextStream in(&schema);
    //     while (!in.atEnd()) {
    //         QString line = in.readLine();
    //         QStringList parts = line.split('#');
    //         QString tableName = parts.takeFirst();
    //         int pos = 0;
    //         for (int i = 0; i < parts.size(); i += 2) {
    //             attributeMeta meta;
    //             // meta.tableName = tableName;
    //             meta.attributeName = parts.at(i);
    //             meta.type = parts.at(i + 1).at(0).toLatin1();
    //             meta.position = pos;
    //             meta.length = 0;
    //             if (meta.type == 'c' || meta.type == 'v') {
    //                 int start = parts[i + 1].indexOf('(') + 1;
    //                 int end = parts[i + 1].indexOf(')');
    //                 meta.length = QStringView{parts[i + 1]}.mid(start, end - start).toInt();
    //             }
    //             attributes.insert(tableName, meta);
    //             pos++;
    //         }
    //     }
    //     return true;
    // }
    return false;
}


// When pressed 'save' button
void Core::SystemCatalog::writeToSchema(const QString &relName)
{
    // // write to catalog.bin
    // QFile schema(schemaPath);
    // schema.open(QIODevice::Append | QIODevice::Text);
    // QTextStream out(&schema);
    // out << relName;
    // QList<attributeMeta> values = attributes.values(relName);
    // // sort by position before writing
    // // (even if previously inserted in order, they are not guaranteed
    // // to be retrieved in the same order)
    // // std::sort(values.begin(), values.end(), [](const attrMeta &a, const attrMeta &b) {
    // //     return a.position < b.position;
    // // });
    // std::reverse(values.begin(), values.end());
    // QMap<char, QString> datatypeNames;
    // datatypeNames['i'] = "int";
    // datatypeNames['f'] = "float";
    // datatypeNames['d'] = "double";
    // datatypeNames['b'] = "bool";
    // datatypeNames['t'] = "tinyint";
    // datatypeNames['c'] = "char";
    // datatypeNames['v'] = "varchar";
    // for (const auto& i : std::as_const(values)) {
    //     QString typeName = datatypeNames.value(i.type);
    //     QString line;
    //     if (typeName == "char" || typeName == "varchar")
    //         line = QString("#%1#%2(%3)").arg(i.attributeName, typeName).arg(i.length);
    //     else
    //         line = QString("#%1#%2").arg(i.attributeName, typeName);
    //     out << line;
    // }
    // out << Qt::endl;
    // schema.close();
}

QSharedPointer<Storage::DiskController> Core::SystemCatalog::getDiskController() const
{
    return controller;
}

void Core::SystemCatalog::saveOnDisk()
{
    //
    // QFile file(catalogFile);
    // if (!file.open(QIODevice::WriteOnly))
    //     return;
    // QDataStream out(&file);
    // // out << attributes;
    // // out << cylinderGroups;
    // file.close();
}

void Core::SystemCatalog::readFromDisk()
{
    // QFile file(catalogFile);
    // if (!file.open(QIODevice::ReadOnly))
    //     return;
    // // Clear current default values
    // // cylinderGroups.clear();
    // QDataStream in(&file);
    // // in >> sib;
    // // in >> cylinderGroups;
    // file.close();
}
