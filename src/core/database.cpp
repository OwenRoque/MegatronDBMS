#include "database.h"
#include "heapfile.h"

Core::Database::Database(QSharedPointer<Storage::DiskController> dc, const QString& storagePath,
                          const QString& catalogPath, const QString& replacerPolicy, int bufferSize, bool firstInit)
{
    dm = &DiskManager::getInstance(dc, storagePath, firstInit);
    sc = &SystemCatalog::getInstance(catalogPath, firstInit);
    Types::ReplacementPolicy policy;
    if (replacerPolicy == "LRU") {
        policy = Types::ReplacementPolicy::LRUPolicy;
    } else if (replacerPolicy == "MRU") {
        policy = Types::ReplacementPolicy::MRUPolicy;
    } else if (replacerPolicy == "Clock") {
        policy = Types::ReplacementPolicy::ClockPolicy;
    } else if (replacerPolicy == "Default") {
        policy = Types::ReplacementPolicy::Default;
    }
    bm = &Memory::BufferManager::getInstance(bufferSize, policy);
    // get systemCatalog data from disk, only if it's not first initialization
    if (firstInit == false)
    {
        dm->readFromDisk();
        sc->readFromDisk();
        // TODO: how to init & refill QList<QSharedPointer<File>> relations;
    }
}

Types::Return Core::Database::createRelation(Core::RelationInput response)
{
    // more validations, handle responses
    // checking duplicate relations
    if (sc->relationExists(response.relationName))
        return Types::Return::DuplicateError;

    // add relation to catalog
    quint64 autoIncValue = response.autoIncrementFieldExists();
    auto relation = sc->insertRelation({
        .relationName = response.relationName,
        .numberOfAttributes = 0,
        .fileOrganization = response.fileOrg,
        .recordFormat = response.recFormat,
        .charset = response.charset,
        .autoIncrement = autoIncValue,
        .location = dm->newFileGroup(response.fileOrg)
        // file will be immediately allocated on disk, with a default size
        // when file is full/near full, it will autogrow.
    });

    // add attributes to relation just created
    for (quint8 i = 0; i < response.attributes.size(); i++)
    {
        quint32 maxByteLen;
        Types::DataType dataType = std::get<1>(response.attributes.at(i));
        switch (dataType)
        {
        case Types::TinyInt:
        case Types::UTinyInt:
            maxByteLen = sizeof(qint8);
            break;
        case Types::SmallInt:
        case Types::USmallInt:
            maxByteLen = sizeof(qint16);
            break;
        case Types::Int:
        case Types::UInt:
            maxByteLen = sizeof(qint32);
            break;
        case Types::BigInt:
        case Types::UBigInt:
            maxByteLen = sizeof(qint64);
            break;
        case Types::Float:
            maxByteLen = sizeof(float);
            break;
        case Types::Double:
            maxByteLen = sizeof(double);
            break;
        case Types::Bool:
            maxByteLen = sizeof(bool);
            break;
        case Types::Enum:
            maxByteLen = sizeof(qint8);
            break;
        case Types::Char:
        case Types::Varchar:
            // as stated in the docs: "maxByteLength should be the same as maxCharLength, except for multibyte character sets."
            // only if charset == Latin1 they'll be equal, otherwise not.
            maxByteLen = std::get<3>(response.attributes.at(i)) *
                         sc->constFindCharset(relation->charset)->maxlen;
            break;
        }

        sc->insertAttribute({
            .attributeName = std::get<0>(response.attributes.at(i)),
            .relationName = response.relationName,
            .dataType = std::get<1>(response.attributes.at(i)),
            .columnType = std::get<2>(response.attributes.at(i)),
            .maxCharacterLength = std::get<3>(response.attributes.at(i)),
            .maxByteLength = maxByteLen,
            .defaultValue = std::get<4>(response.attributes.at(i)),
            .ordinalPosition = i,
            .isNullable = std::get<5>(response.attributes.at(i)),
            .isUnsigned = std::get<6>(response.attributes.at(i)),
            .autoIncrement = std::get<7>(response.attributes.at(i)),
            .key = std::get<8>(response.attributes.at(i)),
            .comment = std::get<9>(response.attributes.at(i))
        });
    }

    // add indexes to relation just created
    for (quint8 i = 0; i < response.indexes.size(); i++)
    {
        Core::SystemCatalog::indexMeta im = {
            .indexName = std::get<0>(response.indexes.at(i)),
            .relationName = relation->relationName,
            .attributeName = std::get<1>(response.indexes.at(i)),
            .indexType = std::get<2>(response.indexes.at(i)),
            .isNonUnique = std::get<3>(response.indexes.at(i)).isNonUnique,
            .isNullable = std::get<3>(response.indexes.at(i)).isNullable,
            .isClustered= std::get<3>(response.indexes.at(i)).isClustered,
            .ordering = std::get<3>(response.indexes.at(i)).order,
            .ordinalPosition = std::get<3>(response.indexes.at(i)).ordinalPosition,
            .comment = std::get<3>(response.indexes.at(i)).comment,
            // foreign keys are not defined inside CREATE TABLE statement
            .referencedRelation = "",
            .referencedAttribute = "",
        };
        sc->insertIndex(im);
    }

    QSharedPointer<Core::File> file;

    try {
        switch (response.fileOrg)
        {
        case Types::FileOrganization::Heap:
            file = this->relations.emplaceBack(QSharedPointer<Core::HeapFile>::create(relation->relationName));
            break;
        // TODO
        case Types::FileOrganization::Sequential:
        case Types::FileOrganization::Hash:
        case Types::FileOrganization::BPlusTree:
            break;
        }
    }
    catch (const std::exception& e) {
        qCritical() << "Exception caught:" << e.what();
    }

    // Check if a bulk insert operation will be done after creating the relation
    if (!response.dataPath.isEmpty())
    {
        Types::Return ret = file->bulkInsertRecords(response.dataPath);
        return ret;
    }

    // TODO: implement SaveToDisk in each FileOrganization
    // only when pressed 'save' button, CTRL+S

    dm->saveToDisk();
    sc->saveToDisk();
    return Types::Return::Success;
}
