#include "database.h"
#include "heapfile.h"
#include "record.h"
#include <QPointer>

Core::Database::Database(QSharedPointer<Storage::DiskController> dc, QString storageFile, bool firstInit)
{
    dm = &DiskManager::getInstance(dc, storageFile);
    sc = &SystemCatalog::getInstance();
    // get systemCatalog data from disk, only if it's not first initialization
    if (firstInit == false)
    {
        dm->readFromDisk();     // test
        sc->readFromDisk();     // todo
    }

}

Types::Return Core::Database::createRelation(Core::RelationInput response)
{
    // more validations, handle responses
    // checking duplicate relations
    if (sc->relationExists(response.relationName))
        return Types::Return::DuplicateError;

    // add relation to catalog
    auto relation = sc->insertRelation({
        .relationName = response.relationName,
        .numberOfAttributes = static_cast<quint8>(response.attributes.size()),
        .fileOrganization = response.fileOrg,
        .recordFormat = response.recFormat,
        .charset = response.charset,
        .autoIncrement = 1,
        .location = 0
        // location 0 represents that it's not allocated on disk still (undef. loc)
        // will be defined whether a bulk insert is made or not
        // dm->newFileGroup(response.fileOrg, fileSize)
    });

    // add attributes to relation just created
    for (quint8 i = 0; i < response.attributes.size(); i++)
    {
        quint32 maxByteLen;
        switch (relation->recordFormat)
        {
        case Types::RecordFormat::Fixed:
        case Types::RecordFormat::Variable:
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

    QSharedPointer<Core::File> file;

    switch (response.fileOrg)
    {
    case Types::FileOrganization::Heap:
        file = this->relations.emplaceBack(QSharedPointer<Core::HeapFile>::create());
        break;
    case Types::FileOrganization::Sequential:
    case Types::FileOrganization::Hash:
    case Types::FileOrganization::BPlusTree:
        break;
    }

    QQueue<QPointer<Core::Record>> records;
    // Extract records and calculate fileSize (overhead included)
    int fileSize = 0;
    // Check if a bulk insert operation will be done after creating the relation
    if (!response.dataPath.isEmpty())
    {
        file->bulkInsertRecords(response.dataPath);
        // QFile newData(response.dataPath);
        // if (newData.open(QIODevice::ReadOnly))
        // {
        //     QTextStream in(&newData);
        //     // ignore header
        //     QString line = in.readLine();
        //     while (!in.atEnd())
        //     {
        //         QString line = in.readLine();
        //         // parse record algorithm
        //         std::stringstream inLine(line.toStdString());
        //         QStringList values;
        //         bool insideQuotes = false;
        //         QString word;
        //         char c;
        //         while (inLine.get(c))
        //         {
        //             // qDebug() << word;
        //             if (c == ',') {
        //                 // If no content
        //                 if (word.isEmpty()) {
        //                     // qDebug() << "Nan";
        //                     values.append("");
        //                 }
        //                 else if (insideQuotes)
        //                     word+=c;
        //                 else {
        //                     values.append(word);
        //                     word.clear();
        //                 }
        //             }
        //             // not so sure about some (unlikely) cases like  ""hello", he said"
        //             else if (c == '"') {
        //                 if (word.isEmpty())
        //                     insideQuotes = true;
        //                 else {
        //                     char p = inLine.peek();
        //                     // If data ends
        //                     if (p == ',') {
        //                         values.append(word);
        //                         word.clear();
        //                         insideQuotes = false;
        //                         inLine.seekg(1, std::ios_base::cur);
        //                     }
        //                     // Then it's a '"' inside commillas
        //                     else
        //                         word+=c;
        //                 }
        //             }
        //             else word+=c;
        //         }
        //         // handle last field
        //         if (!word.isEmpty())
        //             values.append(word);

        //         for (qsizetype i = 0; i < values.size(); ++i)
        //         {
        //             const auto &w = values.at(i);
        //             // create RECORDD
        //             qDebug() << "Creating record...";
        //             // add to queue, get size with .head() method
        //         }
        //     }
        //     // run table.bulk<insert
        //     // validate record data according to constraints - relation.bulkInsert method
        //     // also validate when inserting single record - relation.Insert method
        //     // only insert the valid ones, reject the invalid
        //     // as records are created as objects (record.h), fill them one by one
        //     // when BufferManager is full, write to disk evicted pages
        //     newData.close();
        // }
        // else return Types::Return::OpenError;
    }







    // TODO: implement SaveToDisk in each FileOrganization
    // only when pressed 'save' button, CTRL+S



    return Types::Return::Success;
}
