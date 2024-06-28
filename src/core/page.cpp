#include "page.h"
#include "disk.h"
#include "record.h"

Core::Page::Page(QSharedPointer<Storage::Block> block)
{
    id = block->getBlockId();
}

Core::Page::Page(int pageId)
{
    id = pageId;
}

Core::Page::~Page() {}

int Core::Page::getId() const
{
    return id;
}

Core::DataPage::DataPage(QSharedPointer<Storage::Block> block)
    : Core::Page(block) {}

Core::DataPage::DataPage(int pageId) : Core::Page(pageId) {}

/// Unpacked Page Implementation (for fixed-legth records)

Core::UnpackedDataPage::UnpackedDataPage(QSharedPointer<Storage::Block> block/*, int recordSize = 0*/)
    : Core::DataPage(block)
{
    // retrieve block data
    QByteArray source = block->getData();
    QDataStream stream(source);

    // read number of slots, to determine page header start position on the byte array
    qint64 nOfSlotsPos = source.size() - sizeof(this->numberOfSlots);
    stream.device()->seek(nOfSlotsPos);
    stream >> this->numberOfSlots;

    // read recordSize (crucial, varies depending to the relation to which belongs)
    qint64 recordSizePos = nOfSlotsPos - sizeof(this->recordSize);
    stream.device()->seek(recordSizePos);
    stream >> this->recordSize;

    // read bitmap in temporal byte array
    int bitmapSize = qFloor((this->numberOfSlots + 7) / 8);
    qint64 bitmapPos = recordSizePos - bitmapSize;
    stream.device()->seek(bitmapPos);
    QByteArray bitArray(bitmapSize, 0);
    stream.readRawData(bitArray.data(), bitArray.size());
    // convert temporal byte array to bit array
    for (int i = 0; i < bitArray.size(); ++i) {
        bool bit = bitArray[i / 8] & (1 << (7 - (i % 8)));
        this->bitmap.setBit(i, bit);
    }
    // read page date from byte array start
    // bitmap position = page data size
    stream.device()->seek(0);
    stream.readRawData(this->data.data(), bitmapPos);
    // page data+header succesfully retrieved
}

Core::UnpackedDataPage::UnpackedDataPage(int pageId, int recordSize) : Core::DataPage(pageId)
{
    // first initialization of page
    // calculate number of records that can fit in the page
    // including page header overhead
    auto calculateNumberOfRecords = [=](int page_size, int record_size) {
        int num_records = 0;
        int header_size = 0;
        int total_size = 0;

        while (true) {
            // calculate header overhead (nSlots + size of bitmap in bytes)
            header_size = sizeof(this->numberOfSlots) + ((num_records + 7) / 8);
            // total size of page
            total_size = header_size + (num_records * record_size);
            // if the calculated size exceedes the real size of the page, break
            if (total_size > page_size) {
                break;
            }
            num_records++;
        }

        // decrease number of records since the last one caused the overage
        return num_records - 1;
    };
    // fixed number of slot
    this->numberOfSlots = calculateNumberOfRecords(Storage::blockSize - 1, recordSize);
    this->recordSize = recordSize;
    // bitmap fixed-size
    int bitmapSize = qFloor((this->numberOfSlots + 7) / 8);
    this->bitmap = QBitArray(this->numberOfSlots, false);
    // data size (not including header page bytes)
    this->data = QByteArray(Storage::blockSize
                                - 1                                     // block->getHeader()
                                - bitmapSize
                                - sizeof(this->numberOfSlots), '\0');
}

bool Core::UnpackedDataPage::addRecord(const Record &record)
{
    // check if there's any slot available
    for (int i = 0; i < this->numberOfSlots; i++) {
        if (!this->bitmap.testBit(i)) {
            // insert record to the data array, in its corresponding slot
            this->data.replace(i * this->recordSize, record.toBytes().size(), record.toBytes());
            // update bitmap
            this->bitmap.setBit(i, true);
            return true;
        }
    }
    // if there's no slot available in bitmap, page is full
    return false;
}

bool Core::UnpackedDataPage::deleteRecord(const quint16 &slot_id)
{
    // handle invalid slot_id
    if (slot_id >= this->numberOfSlots)
        return false;

    if (this->bitmap.testBit(slot_id))
        this->bitmap.setBit(slot_id, false);
    // no need to clear the old record
    return true;
}

QByteArray Core::UnpackedDataPage::findRecord(const quint16 &slot_id)
{
    // handle invalid slot_id
    if (slot_id >= this->numberOfSlots)
        return QByteArray(this->recordSize, '\0');

    bool state = this->bitmap.testBit(slot_id);
    if (state)
        return this->data.mid(slot_id * this->recordSize, recordSize);
    else
        return QByteArray(this->recordSize, '\0');
}

QSharedPointer<Storage::Block> Core::UnpackedDataPage::toBlock()
{
    QSharedPointer<Storage::Block> block;
    // set block id = page id
    block->setId(this->getId());

    // set block header (page/block type)
    block->setHeader(Storage::Block::Header::DataFixed);

    // set block data (page header + page data), init empty byte array
    // subtract block header size to data
    QByteArray data(Storage::blockSize - sizeof(block->getHeader()), '\0');
    QDataStream stream(&data, QIODevice::ReadWrite);
    // save page data from the start of the byte array
    stream.writeRawData(this->data.constData(), this->data.size());
    // page header at the footer
    // convert bitmap to bytes
    int byteCount = (bitmap.size() + 7) / 8;
    QByteArray byteArray(byteCount, 0);
    for (int i = 0; i < bitmap.size(); ++i) {
        byteArray[i / 8] |= (bitmap.testBit(i) ? 1 : 0) << (7 - (i % 8));
    }
    qint64 pageHeaderStartPos = data.size() - byteArray.size()
                                - sizeof(this->numberOfSlots);
    stream.device()->seek(pageHeaderStartPos);
    // page header grows backwards, write in reverse order
    stream << byteArray;
    stream << this->recordSize;
    stream << this->numberOfSlots;

    block->setData(data);
    return block;
}

/// Slotted Page Implementation (for variable-length records)

Core::SlottedPage::SlottedPage(QSharedPointer<Storage::Block> block)
    : Core::DataPage(block)
{
    // retrieve block data
    QByteArray source = block->getData();
    QDataStream stream(source);

    // read number of slots, to determine page header start position on the byte array
    qint64 nOfSlotsPos = source.size() - sizeof(this->numberOfSlots);
    stream.device()->seek(nOfSlotsPos);
    stream >> this->numberOfSlots;

    // read free space pointer
    qint64 fspPos = nOfSlotsPos - sizeof(this->freeSpacePointer);
    stream.device()->seek(fspPos);
    // stream >> this->freeSpacePointer.first >> this->freeSpacePointer.second;
    stream >> this->freeSpacePointer;

    // read slot array
    qint64 slotArrayPos = fspPos - (this->numberOfSlots * sizeof(slotEntry));
    stream.device()->seek(slotArrayPos);
    this->slotArray.resize(this->numberOfSlots);
    for (int i = 0; i < slotArray.size(); ++i) {
        stream >> this->slotArray[i].offset >> this->slotArray[i].length;
    }

    // read page date from byte array start
    // bitmap position = page data size
    stream.device()->seek(0);
    stream.readRawData(this->data.data(), slotArrayPos);
    // page data+header succesfully retrieved
}

Core::SlottedPage::SlottedPage(int pageId) : Core::DataPage(pageId)
{
    // first initialization of page
    this->numberOfSlots = 0;
    // fsp.first points to the beggining of free space in the data byte array
    this->freeSpacePointer.first = 0;
    // fsp.second points to the end of free space in data byte array =
    // start of header page
    this->freeSpacePointer.second = Storage::blockSize - 1                                     // block->getHeader()
                                    - sizeof(this->numberOfSlots)
                                    - sizeof(this->freeSpacePointer);
    // empty slot array
    this->slotArray = QList<slotEntry>();
    // data size (not including header page bytes)
    this->data = QByteArray(this->freeSpacePointer.second, '\0');
}

bool Core::SlottedPage::addRecord(const Record &record)
{
    // place record in free space on page (w/ freeSpaceSpointer)
    // if there's still space left
    // PCTFREE (Percentage Free): Reserved Free Space (threshold) in block for tuple updates only
    int PCTFREE = Storage::blockSize * 0.2f;
    quint16 freeSpace = this->freeSpacePointer.second - this->freeSpacePointer.first;
    if (freeSpace > PCTFREE) {
        // write record to data array
        qint16 index = this->freeSpacePointer.first;
        qint16 recordSize = record.toBytes().size();
        this->data.replace(index, recordSize, record.toBytes());
        // find empty slot in slot array (first ocurrence)
        int slot = slotArray.indexOf({0, 0});
        if (slot != -1) {
            this->slotArray[slot] = {index, recordSize};
            // update fsp
            this->freeSpacePointer.first += recordSize;
        } else {
            // add a new slot
            this->slotArray.append({index, recordSize});
            // update nos
            this->numberOfSlots++;
            // update fsp
            this->freeSpacePointer.first += recordSize;
            this->freeSpacePointer.second -= sizeof(slotEntry);
        }
        return true;
    } else {
        // no more insertions can be done in the current page
        return false;
    }
}

bool Core::SlottedPage::deleteRecord(const quint16 &slot_id)
{
    // handle invalid slot_id
    if (slot_id >= this->numberOfSlots)
        return false;
    // eager scheme: shift records to occupy free space, avoiding fragmentation
    int start = this->slotArray[slot_id].offset;
    int length = this->slotArray[slot_id].length;
    // shift subsequent entries in the byte array
    int shiftStart = start + length;
    int shiftLength = this->data.size() - shiftStart;
    memmove(this->data.data() + start, this->data.data() + shiftStart, shiftLength);

    // fill the end of the byte array with '\0' to maintain the original size
    this->data.append(length, '\0');

    // mark the entry as removed
    this->slotArray[slot_id].offset = 0;
    this->slotArray[slot_id].length = 0;

    // update offsets of subsequent slots
    for (int i = slot_id + 1; i < this->slotArray.size(); ++i) {
        if (this->slotArray[i].length > 0) {
            this->slotArray[i].offset -= length;
        }
    }
    return true;
}

QSharedPointer<Storage::Block> Core::SlottedPage::toBlock()
{
    QSharedPointer<Storage::Block> block;
    // set block id = page id
    block->setId(this->getId());

    // set block header (page/block type)
    block->setHeader(Storage::Block::Header::DataVariable);

    // set block data (page header + page data), init empty byte array
    // subtract block header size to data
    QByteArray data(Storage::blockSize - sizeof(block->getHeader()), '\0');
    QDataStream stream(&data, QIODevice::ReadWrite);
    // save page data from the start of the byte array
    stream.writeRawData(this->data.constData(), this->data.size());
    // page header at the footer
    qint64 pageHeaderStartPos = data.size() - sizeof(this->numberOfSlots)
                                - sizeof(this->freeSpacePointer)
                                - (this->slotArray.size() * sizeof(slotEntry));
    stream.device()->seek(pageHeaderStartPos);
    // header page grows backwards, but we can't write in a file backwards
    // write data in reverse order
    for (int i = 0; i < slotArray.size(); ++i) {
        stream << this->slotArray[i].offset << this->slotArray[i].length;
    }
    // stream << this->freeSpacePointer;
    stream << this->freeSpacePointer.first << this->freeSpacePointer.second;
    stream << this->numberOfSlots;

    block->setData(data);
    return block;
}


