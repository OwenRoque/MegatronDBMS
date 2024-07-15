#include "heapfile.h"
#include "systemcatalog.h"
#include "diskmanager.h"
#include <buffermanager.h>
#include "pagefactory.h"
#include "record.h"

Core::HeapFile::HeapFile(const QString& relationName) : Core::File(relationName)
{
    // retrieve relation's metadata
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->findRelation(relationName);
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    // initialize free space map, by retrieving relations' storage info
    QList<int> dataBlocks;

    QVariant fileGroupVariant = dm->locateFileGroup(relation->location);
    if (!fileGroupVariant.isValid()) {
        qWarning() << "File group not found!";
        // throw an exception
        throw std::runtime_error("File group not found!");
    }
    if (!fileGroupVariant.canConvert<HeapGroup>()) {
        qWarning() << "File group is not a HeapGroup!";
        throw std::runtime_error("File group is not a HeapGroup!");
    }
    HeapGroup heapGroup = fileGroupVariant.value<HeapGroup>();
    dataBlocks = heapGroup.data.blocks;

    for (const int i : dataBlocks)
        // Default fs fraction, 255/256 free space
        freeSpace.insert(i , 255);
}

Types::Return Core::HeapFile::insertRecord()
{
    // TODO: validate record data according to constraints - relation.insertRecord method
    return Types::Return::Success;
}

Types::Return Core::HeapFile::bulkInsertRecords(const QString &dataPath)
{
    // parse CSV File algorithm
    auto parseCSVLine = [](const QString& line) {
        QStringList fields;
        bool inQuotes = false;
        QString currentField;

        for (int i = 0; i < line.length(); ++i) {
            QChar ch = line[i];
            if (ch == '\"') {
                inQuotes = !inQuotes; // Toggle the inQuotes flag
            } else if (ch == ',' && !inQuotes) {
                fields.append(currentField.trimmed());
                currentField.clear();
            } else {
                currentField.append(ch);
            }
        }
        fields.append(currentField.trimmed()); // Add the last field

        return fields;
    };

    QList<QStringList> allRecords;
    QFile newData(dataPath);
    if (newData.open(QIODevice::ReadOnly))
    {
        QTextStream in(&newData);
        // omit first row
        QString line = in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            if (!line.isEmpty()) {
                QStringList fields = parseCSVLine(line);
                allRecords.append(fields);
            }
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
    for (const auto& stringRecord : allRecords)
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
        recordList.append(record);
    }

    // add records to file
    Core::DiskManager* dm = &Core::DiskManager::getInstance();
    // no of blocks left to process
    int nBlocks = freeSpace.size();

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    while (!recordList.empty())
    {
        // when there are no more pages with free space available for
        // new records, file needs to grow in size
        if (nBlocks == 0) {
            // call autogrow method
            if (this->autogrow()) {
                // add no of blocks added to the file, this number is known
                nBlocks += Core::AutoGrowthFactor;
                continue;
            }
            else
                return Types::Return::RuntimeError;
        }
        // retrieve block with more free space (pops it from queue)
        block_id_t target = freeSpace.getBlockWithMoreFreeSpace();
        // buffer pool request: fetch operation (add target page to buffer before inserting)
        QSharedPointer<Memory::Frame> frame = bm->fetchPage(target);
        // access the frame which holds the desired page, we don't know which type it is
        QSharedPointer<Core::Page> page = frame->getPage();
        // dataPage object, this is the type we need to store data
        QSharedPointer<Core::DataPage> targetPage;

        if (relation->recordFormat == Types::RecordFormat::Fixed)
        {
            // calculate record size for fixed-length page
            auto [beg, it] = sc->constFindAttributesFor(relationName);
            int recordLength = 0;
            if (it != beg)
                --it;
            while (true)
            {
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
                if (it == beg)
                    break;
                --it;
            }
            // create new data page: replace free page object
            if (qSharedPointerDynamicCast<FreePage>(page)) {
                targetPage = Core::PageFactory::createUnpackedPage(page->getId(), recordLength).staticCast<Core::DataPage>();
                frame->setPage(targetPage);
            }
            else {
                // cast the page to a DataPage, this is the one we need to store data
                targetPage = page.staticCast<Core::DataPage>();
            }
        }
        else if (relation->recordFormat == Types::RecordFormat::Variable)
        {
            // create new data page: replace free page object
            if (qSharedPointerDynamicCast<FreePage>(page)) {
                targetPage = Core::PageFactory::createSlottedPage(page->getId()).staticCast<Core::DataPage>();
                frame->setPage(targetPage);
            }
            else {
                // cast the page to a DataPage, this is the one we need to store data
                targetPage = page.staticCast<Core::DataPage>();
            }
        }

        // insert Record object into target page
        bool insertOk = false;
        do {          
            Core::Record* rec = recordList.takeFirst();
            insertOk = targetPage->addRecord(*rec);
            // delete record object only if it got inserted succesfully
            if (insertOk)
                delete rec;
            // otherwise reinsert it to the container
            else
                recordList.prepend(rec);
        }
        while (!recordList.isEmpty() && insertOk);
        // mark current frame as dirty, since changes were made
        frame->setDirty(true);
        // update page free space, after no more operations are made on it
        freeSpace.insert(targetPage->getId(), targetPage->getFreeSpace());
        // decrease no of blocks processed
        nBlocks--;
    }

    // save data to disk
    bm->flushAllPages();

    return Types::Return::Success;
    // TODO: validate record data according to constraints - relation.bulkInsert method
    // only insert the valid ones, reject the invalid
    // will be done with the help of index structure
}

Types::Return Core::HeapFile::deleteRecord()
{
    return Types::Return::Success;
}

bool Core::HeapFile::autogrow()
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->findRelation(relationName);
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    QVariant fileGroupVariant = dm->locateFileGroup(relation->location);
    HeapGroup heapGroup = fileGroupVariant.value<HeapGroup>();
    int start = 0;
    bool ret = dm->autogrowFileNode(heapGroup.data, start);
    // update free-space-map, add newly added block addresses
    if (ret) {
        // blocks allocated are inserted at the end of the list
        for (; start < heapGroup.data.blocks.size(); ++start)
            freeSpace.insert(heapGroup.data.blocks[start], 255);
    }
    return ret;
}

