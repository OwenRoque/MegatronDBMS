#include "heapfile.h"
#include "systemcatalog.h"
#include "diskmanager.h"
#include "pagefactory.h"
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

    // get free page from fsm
    QList<int> blocksFSM;
    QVariant fileGroupVariant = dm->locateFileGroup(fileGroupId);
    if (!fileGroupVariant.isValid()) {
        qWarning() << "File group not found!";
        return Types::Return::OpenError; // or throw an exception
    }
    if (fileGroupVariant.canConvert<HeapGroup>()) {
        HeapGroup heapGroup = fileGroupVariant.value<HeapGroup>();
        blocksFSM = heapGroup.freeSpace.blocks;
    }
    for (const int i : blocksFSM)
        freeSpace.insert(i , 8);    // Default fs fraction, 8/8 free space

    while (!recordList.empty())
    {
        // retrieve block with more free space
        quint64 target = freeSpace.getBlockWithMoreFreeSpace();
        // read block from disk, store its contents in the byte array
        QByteArray blockContent;
        dm->readBlock(target, blockContent);
        // convert byte array to Block instance
        QSharedPointer<Storage::Block> block = QSharedPointer<Storage::Block>::create(target, blockContent);
        // set block header according to datapage required
        // & construct Page equivalent to Block
        Core::DataPageFactory factory;
        QSharedPointer<Core::DataPage> targetPage;
        if (relation->recordFormat == Types::RecordFormat::Fixed)
        {
            block->setHeader(Storage::Block::Header::DataFixed);
            // calculate record size for fixed-length page
            auto [beg, it] = sc->constFindAttributesFor(relationName);
            int recordLength = 0;
            while (it != beg)
            {
                --it;
                switch (it->dataType)
                {
                case Types::DataType::TinyInt:
                case Types::DataType::UTinyInt:
                case Types::DataType::Enum:
                    recordLength += sizeof(qint8);
                    break;
                case Types::DataType::SmallInt:
                case Types::DataType::USmallInt:
                    recordLength += sizeof(qint16);
                    break;
                case Types::DataType::Int:
                case Types::DataType::UInt:
                    recordLength += sizeof(qint32);
                    break;
                case Types::DataType::BigInt:
                case Types::DataType::UBigInt:
                    recordLength += sizeof(qint64);
                    break;
                case Types::DataType::Float:
                    recordLength += sizeof(float);
                    break;
                case Types::DataType::Double:
                    recordLength += sizeof(double);
                    break;
                case Types::DataType::Bool:
                    recordLength += sizeof(bool);
                    break;
                case Types::DataType::Char:
                    recordLength += it->maxByteLength;
                    break;
                case Types::DataType::Varchar:
                    // Not Allowed, validation already done in interface
                    break;
                }
            }
            targetPage =  qSharedPointerCast<Core::DataPage>(factory.createPage(Storage::Block::Header::DataFixed, block->getBlockId(), recordLength));
        }
        else if (relation->recordFormat == Types::RecordFormat::Variable)
        {
            block->setHeader(Storage::Block::Header::DataVariable);
            targetPage =  qSharedPointerCast<Core::DataPage>(factory.createPage(Storage::Block::Header::DataFixed, block->getBlockId()));
        }
        // insert Record object into target page
        bool insertOk = false;
        do {
            // verify if there are still records to pop from temporal list

            if (!records.isEmpty()) {
                Core::Record* rec = recordList.takeFirst();
                insertOk = targetPage->addRecord(*rec);
                delete rec;
            }
            // test
            qDebug() << targetPage->findRecord(0);
            qDebug() << targetPage->findRecord(1);
            qDebug() << targetPage->findRecord(3);
        }
        while (!recordList.isEmpty() || !insertOk);

    }
    // when BufferManager is full, write to disk evicted pages
    // TODO: add target page to buffer before inserting

    return Types::Return::Success;
    // TODO: validate record data according to constraints - relation.bulkInsert method
    // only insert the valid ones, reject the invalid
}

Types::Return Core::HeapFile::deleteRecord()
{
    return Types::Return::Success;
}

