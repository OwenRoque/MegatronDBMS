#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <megatron_types.h>
#include "replacer.h"

namespace Memory
{

class BufferManager : public QObject
{
    Q_OBJECT
public:
    static BufferManager& getInstance(size_t size = 0, const Types::ReplacementPolicy& policy = Types::ReplacementPolicy::Default)
    {
        static BufferManager singleton(size, policy);
        return singleton;
    }

    /**
     * @brief returns the size (number of frames) of the buffer pool
     */
    auto getPoolSize() -> size_t;

    /**
     * @brief returns all pages in the buffer pool
     */
    auto getFrames() -> QList<QSharedPointer<Memory::Frame>>;

    /**
     * @brief Retrieves the frame_id that can hold a page, either from the free list or a victim frame chosen by the replacer
     * (when the free list is empty)
     * @return INVALID_PAGE_ID if every page in the buffer pool is pinned, otherwise a valid frame_id
     */
    auto getVictimFrameId() -> frame_id_t;

    /**
     * @brief Create a new page in the buffer pool. Set page_id to the new page's id, or nullptr if all frames
     * are currently in use and not evictable (in another word, pinned).
     * The creation of a new page is performed when there is actually an empty block on disk, ensuring that the page can be saved to disk later.
     * @param[out] page_id id of created page (it will be updated if a new page gets successfully created, otherwise an invalid id is set)
     * @return nullptr if no new pages could be created, otherwise pointer to new page
     */
    auto newPage(page_id_t* page_id) -> QSharedPointer<Memory::Frame>;

    /**
     * @brief Fetch the requested page from the buffer pool. Return nullptr if page_id needs to be fetched from the disk
     * but all frames are currently in use and not evictable (in another word, pinned).
     * @param page_id id of page to be fetched
     * @return nullptr if page_id cannot be fetched, otherwise pointer to the requested page
     */
    auto fetchPage(page_id_t page_id) -> QSharedPointer<Memory::Frame>;

    /**
     * @brief Unpin the target page from the buffer pool. If page_id is not in the buffer pool or
     * its pin count is already 0, return false.
     * @param page_id id of page to be unpinned
     * @param is_dirty true if the page should be marked as dirty, false otherwise.
     * This parameter keeps track of whether a page was modified while it was pinned.
     * @return false if the page is not in the page table or its pin count is <= 0 before this call, true otherwise
     */
    auto unpinPage(page_id_t page_id, bool is_dirty) -> bool;

    /**
     * @brief Flush the target page to disk, regardless of its dirty bit status
     * @param page_id id of page to be flushed, cannot be INVALID_PAGE_ID
     * @return false if the page could not be found in the page table, true otherwise
     */
    auto flushPage(page_id_t page_id) -> bool;

    /**
     * @brief Delete a page from the buffer pool. If page_id is not in the buffer pool, do nothing and return true. If the
     * page is pinned and cannot be deleted, return false immediately.
     * @param page_id id of page to be deleted
     * @return false if the page exists but could not be deleted, true if the page didn't exist or deletion succeeded
     */
    auto deletePage(page_id_t page_id) -> bool;

    /**
     * @brief Flush all the pages in the buffer pool to disk.
     */
    void flushAllPages();

    // performance methods
    /**
     * @brief Page Fault Counter getter, number of times a page was not found in memory (lookup on disk)
     */
    int getPFC() const;

    /**
     * @brief Page Hit Counter getter, number of times a page was found in memory (no lookup)
     */
    int getPHC() const;

    /**
     * @brief calculates the hit rate, the higher the better
     * @return hit rate percentage
     */
    double getHitRate() const;


private:
    // constructor (private since it's a singleton)
    BufferManager(size_t pool_size, const Types::ReplacementPolicy& policy);
    // size of the buffer: number of pages it can hold
    size_t bufferSize;
    // chosen replacement policy
    Types::ReplacementPolicy policy;
    // buffer pool: array of frames, each frame holds a page
    QList<QSharedPointer<Memory::Frame>> frames;
    // <page_id, frame_id> Page Table for keeping track of buffer pool pages
    QHash<page_id_t, frame_id_t> pageTable;
    // replacer to find unpinned pages for replacement
    QSharedPointer<Memory::Replacer> replacer;
    // list of free frames that don't have any pages on them
    QList<frame_id_t> freeList;
    // number of times a page is not found in the page table
    int pageFaultCounter;
    // number of times a page is found in the page table
    int pageHitCounter;

    // helper function to convert block to derived page (data/index)
    QSharedPointer<Core::Page> pageConverter(QSharedPointer<Storage::Block> block);

    Q_DISABLE_COPY(BufferManager);
};

} // namespace Memory

#endif // BUFFERMANAGER_H
