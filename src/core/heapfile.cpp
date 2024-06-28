#include "heapfile.h"
#include "systemcatalog.h"
#include "diskmanager.h"
#include "record.h"

Core::HeapFile::HeapFile(const QString& relationName) : Core::File(relationName)
{

}

Types::Return Core::HeapFile::insertRecord()
{
    // TODO: validate record data according to constraints - relation.insertRecord method
    return Types::Return::Success;
}

Types::Return Core::HeapFile::bulkInsertRecords(const QString &dataPath)
{
    // use ::getInstance();
    // QQueue<QPointer<Core::Record>> records;
    // Extract records and calculate fileSize (overhead included)
    QList<QStringList> records;
    QFile newData(dataPath);
    if (newData.open(QIODevice::ReadOnly))
    {
        QTextStream in(&newData);
        // ignore header
        QString line = in.readLine();
        while (!in.atEnd())
        {
            line = in.readLine();
            // parse record algorithm
            std::stringstream inLine(line.toStdString());
            QStringList record;
            bool insideQuotes = false;
            QString word;
            char c;
            while (inLine.get(c))
            {
                // qDebug() << word;
                if (c == ',') {
                    // If no content
                    if (word.isEmpty()) {
                        // qDebug() << "Nan";
                        record.append("");
                    }
                    else if (insideQuotes)
                        word+=c;
                    else {
                        record.append(word);
                        word.clear();
                    }
                }
                // not so sure about some (unlikely) cases like  ""hello", he said"
                else if (c == '"') {
                    if (word.isEmpty())
                        insideQuotes = true;
                    else {
                        char p = inLine.peek();
                        // If data ends
                        if (p == ',') {
                            record.append(word);
                            word.clear();
                            insideQuotes = false;
                            inLine.seekg(1, std::ios_base::cur);
                        }
                        // Then it's a '"' inside commillas
                        else
                            word+=c;
                    }
                }
                else word+=c;
            }
            // handle last field
            if (!word.isEmpty())
                record.append(word);

            records.append(record);
        }
        newData.close();
    }
    else return Types::Return::OpenError;

    // list of records has been collected, proceed to calculate the total size of the dataset
    // retrieve relation's record format
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->findRelation(relationName);

    // convert stringlist raw record to record pointers, store them temporaly in a container
    QList<Record*> recordList;
    int fileSize = 0;
    for (const auto& stringRecord : records)
    {
        Record* record;
        switch (relation->recordFormat)
        {
        case Types::Fixed:
            record = new FLRecord(relationName, stringRecord);
            break;
        case Types::Variable:
            record = new VLRecord(relationName, stringRecord);
            break;
        }
        // get size of Record, and increment fileSize
        fileSize += record->toBytes().size();
        recordList.append(record);
        // not considering page header overhead, varies according to n° of pages
    }

    // call Disk Manager to allocate space on disk for the File
    Core::DiskManager* dm = &Core::DiskManager::getInstance();
    qint64 fileGroupId = dm->newFileGroup(relation->fileOrganization, fileSize);

    // update 'location' field in the catalog
    relation->location = fileGroupId;

    // when BufferManager is full, write to disk evicted pages

    return Types::Return::Success;

    // TODO: validate record data according to constraints - relation.bulkInsert method
    // only insert the valid ones, reject the invalid



}

Types::Return Core::HeapFile::deleteRecord()
{
    return Types::Return::Success;
}

