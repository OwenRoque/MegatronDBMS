#include "page.h"
#include "disk.h"
#include "record.h"
#include "generic_key.h"
#include "generic_comparator.h"
#include <buffermanager.h>

Core::Page::Page(QSharedPointer<Storage::Block> block)
{
    id = block->getId();
    switch (block->getHeader()) {
        case Storage::BlockType::FreePage:
            // this cases shouldn't ever happen
            break;
        case Storage::BlockType::InvalidType:
            qWarning() << "Invalid Type just created, invalid types need to be solved.";
            break;
        case Storage::BlockType::UnpackedPage:
            pageType = Types::PageType::UnpackedPage;
            break;
        case Storage::BlockType::SlottedPage:
            pageType = Types::PageType::SlottedPage;
            break;
        case Storage::BlockType::BPT_InternalPage:
            pageType = Types::PageType::BPT_InternalPage;
            break;
        case Storage::BlockType::BPT_LeafPage:
            pageType = Types::PageType::BPT_LeafPage;
            break;
        break;
    }
}

Core::Page::Page(page_id_t pageId)
{
    id = pageId;
}

Core::Page::~Page() {}

Core::page_id_t Core::Page::getId() const
{
    return id;
}

/// DataPage implementations

Core::DataPage::DataPage(QSharedPointer<Storage::Block> block)
    : Core::Page(block) {}

Core::DataPage::DataPage(page_id_t pageId) : Core::Page(pageId) {}

/// Unpacked Page Implementation (for fixed-legth records)

Core::UnpackedDataPage::UnpackedDataPage(QSharedPointer<Storage::Block> block)
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

Core::UnpackedDataPage::UnpackedDataPage(page_id_t pageId, int recordSize) : Core::DataPage(pageId)
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

bool Core::UnpackedDataPage::deleteRecord(const slot_id_t &slot_id)
{
    // handle invalid slot_id
    if (slot_id >= this->numberOfSlots)
        return false;

    if (this->bitmap.testBit(slot_id))
        this->bitmap.setBit(slot_id, false);
    // no need to clear the old record
    return true;
}

QByteArray Core::UnpackedDataPage::findRecord(const slot_id_t &slot_id)
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

quint8 Core::UnpackedDataPage::getFreeSpace() const
{
    // get number of empty slots
    int emptySlots = this->bitmap.count(false);
    int freeBytes = emptySlots * this->recordSize;
    // calculate free space fraction (n/256)
    // return n
    double frac = static_cast<double>(freeBytes) / static_cast<double>(Storage::blockSize);
    return static_cast<int>(frac * 256);

}

QSharedPointer<Storage::Block> Core::UnpackedDataPage::toBlock()
{
    QSharedPointer<Storage::Block> block(new Storage::Block());
    // set block id = page id
    block->setId(this->getId());

    // set block header (page/block type)
    block->setHeader(Storage::BlockType::UnpackedPage);

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

Core::SlottedPage::SlottedPage(page_id_t pageId) : Core::DataPage(pageId)
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

bool Core::SlottedPage::deleteRecord(const slot_id_t &slot_id)
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

// TODO:
QByteArray Core::SlottedPage::findRecord(const slot_id_t &slot_id)
{
    QString ret = "find " + QString::number(slot_id) + " record.";
    return ret.toUtf8();
}

quint8 Core::SlottedPage::getFreeSpace() const
{
    // calculate freeBytes with freeSpacePointer
    int freeBytes = this->freeSpacePointer.second - this->freeSpacePointer.first;
    // calculate free space fraction (n/256)
    // return n
    double frac = static_cast<double>(freeBytes) / static_cast<double>(Storage::blockSize);
    return static_cast<int>(frac * 256);
}

QSharedPointer<Storage::Block> Core::SlottedPage::toBlock()
{
    QSharedPointer<Storage::Block> block(new Storage::Block());
    // set block id = page id
    block->setId(this->getId());

    // set block header (page/block type)
    block->setHeader(Storage::BlockType::SlottedPage);

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
    stream << this->freeSpacePointer.first << this->freeSpacePointer.second;
    stream << this->numberOfSlots;

    block->setData(data);
    return block;
}

/// BPlusTreeIndexPage Implementations

Core::BPTIndexPage::BPTIndexPage(QSharedPointer<Storage::Block> block) : Core::Page(block) {}

Core::BPTIndexPage::BPTIndexPage(page_id_t page_id) : Core::Page(page_id) {}

bool Core::BPTIndexPage::isLeafPage() const
{
    return pageType == Types::PageType::BPT_LeafPage;
}

bool Core::BPTIndexPage::isRootPage() const
{
    return parentPageId == INVALID_PAGE_ID;
}

void Core::BPTIndexPage::setIndexPageType(Types::PageType type)
{
    pageType = type;
}

int Core::BPTIndexPage::getSize() const
{
    return size;
}

void Core::BPTIndexPage::setSize(int size_)
{
    size = size_;
}

void Core::BPTIndexPage::increaseSize(int amount)
{
    size += amount;
}

int Core::BPTIndexPage::getMaxSize() const
{
    return maxSize;
}

void Core::BPTIndexPage::setMaxSize(int max_size)
{
    maxSize = max_size;
}

int Core::BPTIndexPage::getMinSize() const
{
    if (isRootPage()) {
        /**
         * When the node is the root and also a leaf page, it must contain at least one key
         * for the tree to make sense.
         * case: When the tree consists of a single node, it must have at leasat one key to represent data
         */
        if (isLeafPage())
            return 1;
        /**
         * When the node is the root and a internal page, it must have at least 2 children
         * (at least one key to separate these children)
         * case: When the tree splits (overflow) from a single node, increasing the height of the tree
         */
        else
            return 2;
    }
    if (!isLeafPage()) {
        /**
         * minimum number of key-value pairs for internal nodes
         * is ⌈(maxSize - 1 - 1)/2⌉ + 1
         * first -1 subtracts 1st invalid key
         * (leftmost children/pointer doesn't have any value,
         * so it's considered 'invalid')
         * second -1 for balancing
        */
        return qCeil((maxSize - 2) / 2.0) + 1;
    }
    /**
     * minimum number of key-value pairs for leaf nodes
     * is ⌈(maxSize - 1)/2⌉
     * -1 for balancing, number of index entries == n. of values
    */
    return qCeil(maxSize / 2.0);
}

Core::page_id_t Core::BPTIndexPage::getParentPageId() const
{
    return parentPageId;
}

void Core::BPTIndexPage::setParentPageId(page_id_t page_id)
{
    parentPageId = page_id;
}

/// BPlusTreeInternalPage Implementation

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::init(page_id_t parentId, int max_size)
{
    setIndexPageType(Types::PageType::BPT_InternalPage);
    setSize(0);
    setParentPageId(parentId);
    setMaxSize(max_size);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
KeyType Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::keyAt(int index) const
{
    return array[index].first;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::setKeyAt(int index, const KeyType &key)
{
    array[index].first = key;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
int Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::valueIndex(const ValueType &value) const
{
    for (int i = 0; i < getSize(); ++i) {
        if (valueAt(i) == value) {
            return i;
        }
    }
    return -1;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
ValueType Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::valueAt(int index) const
{
    return array[index].second;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
ValueType Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::lookup(const KeyType &key, const KeyComparator &comparator) const
{
    int left = 0, right = getSize();
    while (left + 1 < right) {
        int mid = left + (right - left) / 2;
        if (comparator(keyAt(mid), key) <= 0) {
            left = mid;
        } else {
            right = mid;
        }
    }
    return valueAt(left);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::populateNewRoot(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value)
{
    array[0].second = old_value;
    array[1] = {new_key, new_value};
    setSize(2);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
int Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::insertNodeAfter(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value)
{
    // find the old_value index and set it next to it
    auto index = valueIndex(old_value) + 1;
    // insert the new pair next to the old_value
    array.insert(index, {new_key, new_value});
    // increase number of index entries
    increaseSize(1);
    // ret. n. of index entries
    return getSize();
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::remove(int index)
{
    // delete the element at the specified index
    array.removeAt(index);
    // decrease size
    increaseSize(-1);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
ValueType Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::removeAndReturnOnlyChild()
{
    setSize(0);
    return valueAt(0);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::moveAllTo(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key)
{
    setKeyAt(0, middle_key);
    recipient->copyNFrom(array, getSize());
    setSize(0);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::moveHalfTo(QSharedPointer<BPTInternalIndexPage> recipient)
{
    // The first key is never used in the first place, so it doesn't matter if
    // we just copy it.
    auto start = getMinSize();
    int N = getSize() - start;
    QList<MappingType> itemsToMove = array.mid(start, N);
    recipient->copyNFrom(itemsToMove, N);
    array.erase(array.begin() + start, array.begin() + start + N);
    increaseSize(-N);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::moveFirstToEndOf(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key)
{
    recipient->copyLastFrom(array[0]);
    // use the parent node's key as the last key
    recipient->setKeyAt(recipient->getSize() - 1, middle_key);
    remove(0);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::moveLastToFrontOf(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key)
{
    recipient->copyFirstFrom(array[getSize() - 1]);
    // Use the parent's key as the first valid key
    recipient->setKeyAt(1, middle_key);
    array.pop_back();
    increaseSize(-1);
}

// private helper methods to copy-move index entries between pages

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::copyNFrom(const QList<MappingType> &items, int size)
{
    array.append(items.mid(0, size));

    // update the parent id of the child nodex
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    for (int i = 0; i < size; ++i) {
        QSharedPointer<Memory::Frame> frame = bm->fetchPage(items[i].second);
        QSharedPointer<Core::Page> page = frame->getPage();
        // test if changes are reflected in page interface held by the frame
        auto treePage = qSharedPointerDynamicCast<Core::BPTIndexPage>(page);
        treePage->setParentPageId(this->getId());
        bm->unpinPage(page->getId(), true);
    }

    increaseSize(size);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::copyLastFrom(const QPair<KeyType, ValueType> &pair)
{
    array[getSize()] = pair;

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    QSharedPointer<Memory::Frame> frame = bm->fetchPage(pair.second);
    QSharedPointer<Core::Page> page = frame->getPage();
    // test
    auto child = qSharedPointerDynamicCast<Core::BPTIndexPage>(page);
    child->setParentPageId(this->getId());
    bm->unpinPage(page->getId(), true);

    increaseSize(1);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>::copyFirstFrom(const QPair<KeyType, ValueType> &pair)
{
    array.insert(0, pair);

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    QSharedPointer<Memory::Frame> frame = bm->fetchPage(pair.second);
    QSharedPointer<Core::Page> page = frame->getPage();
    auto child = qSharedPointerDynamicCast<Core::BPTIndexPage>(page);
    child->setParentPageId(this->getId());
    bm->unpinPage(page->getId(), true);

    increaseSize(1);
}

template class Core::BPTInternalIndexPage<Core::GenericKey<Types::DataType::TinyInt>, Core::page_id_t, Core::GenericComparator<Types::DataType::TinyInt>>;
template class Core::BPTInternalIndexPage<Core::GenericKey<Types::DataType::SmallInt>, Core::page_id_t, Core::GenericComparator<Types::DataType::SmallInt>>;
template class Core::BPTInternalIndexPage<Core::GenericKey<Types::DataType::Int>, Core::page_id_t, Core::GenericComparator<Types::DataType::Int>>;
template class Core::BPTInternalIndexPage<Core::GenericKey<Types::DataType::BigInt>, Core::page_id_t, Core::GenericComparator<Types::DataType::BigInt>>;

/// BPlusTreeLeafPage Implementation

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::init(page_id_t parentId, int max_size)
{
    setIndexPageType(Types::PageType::BPT_LeafPage);
    setSize(0);
    setParentPageId(parentId);
    setNextPageId(INVALID_PAGE_ID);
    setMaxSize(max_size);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
Core::page_id_t Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::getNextPageId() const
{
    return nextPageId;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::setNextPageId(page_id_t next_page_id)
{
    nextPageId = next_page_id;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
KeyType Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::keyAt(int index) const
{
    return array[index].first;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
int Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::keyIndex(const KeyType &key, const KeyComparator &comparator) const
{
    int left = -1, right = getSize();
    while (left + 1 < right) {
        int mid = left + (right - left) / 2;
        if (comparator(keyAt(mid), key) < 0) {
            left = mid;
        } else {
            right = mid;
        }
    }
    return right;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
const QPair<KeyType, ValueType> &Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::getItem(int index)
{
    return array[index];
}

template<typename KeyType, typename ValueType, typename KeyComparator>
int Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator)
{
    // get the first index greater than or equal to the key and shift the subsequent key-value pairs to the right
    auto index = keyIndex(key, comparator);
    array.insert(index, {key, value});
    increaseSize(1);
    return getSize();
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const
{
    int left = 0, right = getSize() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = comparator(keyAt(mid), key);
        if (cmp == 0) {
            *value = array[mid].second;
            return true;
        } else if (cmp > 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return false;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
int Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::removeAndDeleteRecord(const KeyType &key, const KeyComparator &comparator)
{
    // Find the first subscript greater than or equal to the key
    auto index = keyIndex(key, comparator);
    // If the key isn't found, just return it.
    if (index < 0 || index == getSize() || comparator(keyAt(index), key) != 0) {
        return getSize();
    }
    // otherwise, remove the key-value pair, QList will automatically move the following elements forward.
    array.removeAt(index);
    increaseSize(-1);
    array.pop_back();
    return getSize();
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::moveHalfTo(QSharedPointer<BPTLeafIndexPage> recipient)
{
    // After the root node splits it becomes a leaf node
    int start = !isRootPage() ? getMinSize() : getMaxSize() / 2;
    int N = getSize() - start;
    recipient->copyNFrom(array + start, N);

    // update next page id
    recipient->setNextPageId(getNextPageId());
    setNextPageId(recipient->getPageId());

    increaseSize(-N);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::moveAllTo(QSharedPointer<BPTLeafIndexPage> recipient)
{
    recipient->copyNFrom(array, getSize());
    recipient->setNextPageId(getNextPageId());
    setSize(0);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::moveFirstToEndOf(QSharedPointer<BPTLeafIndexPage> recipient)
{
    recipient->copyLastFrom(getItem(0));
    array.removeAt(0);
    increaseSize(-1);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::moveLastToFrontOf(QSharedPointer<BPTLeafIndexPage> recipient)
{
    recipient->copyFirstFrom(getItem(getSize() - 1));
    increaseSize(-1);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::copyNFrom(const QList<MappingType>& items, int size)
{
    for (int i = 0; i < size; ++i) {
        array.append(items[i]);
    }
    increaseSize(size);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::copyLastFrom(const QPair<KeyType, ValueType> &item)
{
    array.append(item);
    increaseSize(1);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>::copyFirstFrom(const QPair<KeyType, ValueType> &item)
{
    array.insert(0, item);
    increaseSize(1);
}








