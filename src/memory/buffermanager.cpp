#include "buffermanager.h"
#include "lrureplacer.h"
#include "mrureplacer.h"
#include "clockreplacer.h"
#include "diskmanager.h"
#include "pagefactory.h"

Memory::BufferManager::BufferManager(size_t pool_size, const Types::ReplacementPolicy& policy)
    : bufferSize(pool_size), policy(policy)
{
    // set policy & alloc consecutive space in memory for the buffer pool, according to policy
    switch (policy) {
        case Types::Default:
        case Types::LRUPolicy:
        {
            replacer = QSharedPointer<LRUReplacer>::create(pool_size);
            // for (size_t i = 0; i < pool_size; ++i) {
            //     frames.append(QSharedPointer<Memory::LRUFrame>::create());
            // }
            break;
        }
        case Types::MRUPolicy:
        {
            replacer = QSharedPointer<MRUReplacer>::create(pool_size);
            // for (size_t i = 0; i < pool_size; ++i)
            //     frames.append(QSharedPointer<Memory::MRUFrame>::create());
            break;
        }
        case Types::ClockPolicy:
        {
            replacer = QSharedPointer<ClockReplacer>::create(pool_size);
            // for (size_t i = 0; i < pool_size; ++i)
            //     frames.append(QSharedPointer<Memory::ClockFrame>::create());
            break;
        }
    }

    for (size_t i = 0; i < pool_size; ++i) {
        frames.append(QSharedPointer<Memory::Frame>::create());
    }

    // initially, every page is in the free list
    for (frame_id_t i = 0; i < pool_size; ++i) {
        freeList.emplace_back(i);
    }
}

size_t Memory::BufferManager::getPoolSize()
{
    return bufferSize;
}

QList<QSharedPointer<Memory::Frame>> Memory::BufferManager::getFrames()
{
    return frames;
}

Memory::frame_id_t Memory::BufferManager::getVictimFrameId()
{
    frame_id_t frame_id = Core::INVALID_PAGE_ID;
    // if there are still some free frames
    if (!freeList.empty()) {
        // get the first one from the list
        frame_id = freeList.front();
        // delete the chosen frame_id from the list
        freeList.pop_front();
    }
    // if not, our replacer needs to choose a frame (victim frame) to evict
    else {
        replacer->victim(&frame_id);
    }
    return frame_id;
}

QSharedPointer<Memory::Frame> Memory::BufferManager::newPage(page_id_t* page_id = (page_id_t*)Core::INVALID_PAGE_ID)
{
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    // pick the replacement frame from either the free list or the replacer (always find from the free list first)
    frame_id_t replacement_frame = getVictimFrameId();
    // if all the pages in the buffer pool were pinned, our frame will have an invalid value, return nullptr
    if (replacement_frame == Core::INVALID_PAGE_ID) {
        return nullptr;
    }

    // pick the victim frame from the list
    auto frame = frames[replacement_frame];
    // retrieve page contained in victim frame
    auto page = frame->getPage();
    // handle dirty pages, write them back to disk
    if (frame->isDirty()) {
        QSharedPointer<Storage::Block> block = page->toBlock();
        dm->writeBlock(block->getId(), block);
    }

    // allocate new page from disk
    *page_id = dm->allocateBlock();
    // if there's no space on disk, the new page can't be created
    if (*page_id == Core::INVALID_PAGE_ID) {
        return nullptr;
    }
    QSharedPointer<Storage::Block> block = dm->readBlock(*page_id);
    auto newPage = Core::PageFactory::createPage(block);

    // update page table
    if (page != nullptr) {
        pageTable.remove(page->getId());
    }
    pageTable[*page_id] = replacement_frame;

    // update frame metadata
    UpdateParams param;
    switch (policy) {
        case Types::ReplacementPolicy::Default:
        case Types::ReplacementPolicy::LRUPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            break;
        }
        case Types::ReplacementPolicy::MRUPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            break;
        }
        case Types::ReplacementPolicy::ClockPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            param.refBit = true;
            break;
        }
    }
    frame->update(param);

    // pin the replaced frame
    replacer->pin(replacement_frame);

    // increase fault counter: creating a page in the buffer can be considered a page fault
    // since it is being brought to memory from disk
    pageFaultCounter++;

    // finally, return pointer to the new page
    return frame;
}

QSharedPointer<Memory::Frame> Memory::BufferManager::fetchPage(page_id_t page_id)
{
    Core::DiskManager* dm = &Core::DiskManager::getInstance();
    QSharedPointer<Core::Page> page;
    // search in the page table for the requested page
    auto it = pageTable.find(page_id);

    // if page exists, pin & return it
    if (it != pageTable.end()) {
        frame_id_t frame_id = it.value();
        auto frame = frames[frame_id];
        page = frame->getPage();
        replacer->pin(frame_id);
        frames[frame_id]->increasePinCount();
        // increase hit counter
        pageHitCounter++;
        return frame;
    }

    // if page isn't present in page table, we need to find a replacement page,
    // from either the free list or the replacer (find from the free list first always)
    frame_id_t frame_id = getVictimFrameId();
    // if all the pages in the buffer pool were pinned, our frame will have an invalid value, return nullptr
    if (frame_id == Core::INVALID_PAGE_ID) {
        return nullptr;
    }

    // if the page is dirty, write it back to disk
    auto frame = frames[frame_id];
    page = frame->getPage();
    if (frame->isDirty()) {
        QSharedPointer<Storage::Block> block = page->toBlock();
        dm->writeBlock(block->getId(), block);
    }

    // update page table
    if (page != nullptr) {
        pageTable.remove(page->getId());
    }
    pageTable[page_id] = frame_id;

    // read new page from disk
    QSharedPointer<Storage::Block> block = dm->readBlock(page_id);
    auto newPage = Core::PageFactory::createPage(block);

    // update frame metadata
    UpdateParams param;
    switch (policy) {
        case Types::ReplacementPolicy::Default:
        case Types::ReplacementPolicy::LRUPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            break;
        }
        case Types::ReplacementPolicy::MRUPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            break;
        }
        case Types::ReplacementPolicy::ClockPolicy:
        {
            param.page = newPage;
            param.pinCount = 1;
            param.dirtyBit = false;
            param.refBit = true;
            break;
        }
    }
    frame->update(param);

    // pin the replaced frame
    replacer->pin(frame_id);

    // increase fault counter
    pageFaultCounter++;

    // finally, return pointer to the page
    return frame;
}

bool Memory::BufferManager::unpinPage(page_id_t page_id, bool is_dirty)
{
    // if target page is not in memory, there's nothing to do
    auto it = pageTable.find(page_id);
    if (it == pageTable.end()) {
        return false;
    }

    // get frame which holds target page
    frame_id_t frame_id = it.value();
    auto frame = frames[frame_id];

    // if target frame doesn't have any pins left
    if (frame->getPinCount() <= 0) {
        return false;
    }

    // mark as dirty page only if is_dirty == true
    if (is_dirty) {
        frame->setDirty(true);
    }
    // decrement pin count
    frame->decreasePinCount();

    // if there are no pins left in the frame, it can be victimized
    if (frame->getPinCount() == 0) {
        replacer->unpin(frame_id);
    }

    return true;
}

bool Memory::BufferManager::flushPage(page_id_t page_id)
{
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    // target page was not in memory, nothing to flush
    auto it = pageTable.find(page_id);
    if (it == pageTable.end()) {
        return false;
    }

    // retrieve target page to be written to disk
    // the page is NOT evicted from the pool
    frame_id_t frame_id = it.value();
    auto frame = frames[frame_id];
    auto page = frame->getPage();

    // write to disk regardless of its dirty bit status
    QSharedPointer<Storage::Block> block = page->toBlock();
    dm->writeBlock(block->getId(), block);
    frame->setDirty(false);

    return true;
}

bool Memory::BufferManager::deletePage(page_id_t page_id)
{
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    // search target page in the page table
    auto it = pageTable.find(page_id);
    // if it isn't in memory, there's nothing to do
    if (it == pageTable.end()) {
        return true;
    }

    // if it exists but it's pinned, we can't delete the page since it's being used
    frame_id_t frame_id = it.value();
    auto frame = frames[frame_id];
    auto page = frame->getPage();
    if (frame->getPinCount() > 0) {
        return false;
    }

    // otherwise, target page can be deleted
    QSharedPointer<Storage::Block> block = page->toBlock();
    // there's no need to write back to disk, since it will be deleted anyway
    dm->deallocateBlock(block->getId());
    // remove it from the page table
    if (page != nullptr) {
        pageTable.remove(page->getId());
    }
    // add frame target back to free list
    freeList.push_back(frame_id);
    // update frame metadata, default (empty) values
    UpdateParams param;
    frame->update(param);

    return true;
}

void Memory::BufferManager::flushAllPages()
{
    Core::DiskManager* dm = &Core::DiskManager::getInstance();

    // write all pages back to disk, without retiring them from the pool, if they're dirty
    for (auto [page_id, frame_id] : pageTable.asKeyValueRange()) {
        auto frame = frames[frame_id];
        auto page = frame->getPage();
        if (frame->isDirty()) {
            QSharedPointer<Storage::Block> block = page->toBlock();
            dm->writeBlock(block->getId(), block);
            frame->setDirty(false);
        }
    }
}

int Memory::BufferManager::getPFC() const
{
    return pageFaultCounter;
}

int Memory::BufferManager::getPHC() const
{
    return pageHitCounter;
}

double Memory::BufferManager::getHitRate() const
{
    double totalAccesses = pageHitCounter + pageFaultCounter;
    if (totalAccesses == 0) {
        return 0;
    }
    return (pageHitCounter / totalAccesses) * 100;
}
