#ifndef PAGE_H
#define PAGE_H

#include <QSharedPointer>
#include <QByteArray>
#include <QBitArray>
#include <QList>
#include <block.h>
#include "disk.h"
#include "record.h"
#include "megatron_types.h"

namespace Core
{
    using page_id_t = qint32;
    using slot_id_t = qint16;

    static constexpr qint32 INVALID_PAGE_ID = -1;

    class Page
    {
    public:
        Page(QSharedPointer<Storage::Block>);
        Page(page_id_t);
        virtual ~Page() = 0;
        page_id_t getId() const;
        virtual QSharedPointer<Storage::Block> toBlock() = 0;

    protected:
        page_id_t id;
        Types::PageType pageType;

    };

    struct rowId
    {
        page_id_t p_id;
        slot_id_t s_id;
    };

    struct slotEntry
    {
        qint16 offset;
        qint16 length;
        bool operator==(const slotEntry &other) const {
            return offset == other.offset && length == other.length;
        }

        bool operator!=(const slotEntry &other) const {
            return !(*this == other);
        }
    };


    class DataPage : public Page
    {
    public:
        DataPage(QSharedPointer<Storage::Block>);
        DataPage(page_id_t);
        virtual bool addRecord(const Core::Record&) = 0;
        virtual bool deleteRecord(const slot_id_t&) = 0;
        virtual QByteArray findRecord(const slot_id_t&) = 0;
        virtual quint8 getFreeSpace() const = 0;

    };

    class UnpackedDataPage : public DataPage
    {
    public:
        UnpackedDataPage(QSharedPointer<Storage::Block>);
        UnpackedDataPage(page_id_t, int);
        bool addRecord(const Core::Record&) override;
        bool deleteRecord(const slot_id_t&) override;
        QByteArray findRecord(const slot_id_t&) override;
        quint8 getFreeSpace() const override;
        QSharedPointer<Storage::Block> toBlock() override;

    private:
        quint16 numberOfSlots;
        quint16 recordSize;
        QBitArray bitmap;
        QByteArray data;

    };

    class SlottedPage : public DataPage
    {
    public:
        SlottedPage(QSharedPointer<Storage::Block>);
        SlottedPage(page_id_t);
        bool addRecord(const Core::Record&) override;
        bool deleteRecord(const slot_id_t&) override;
        QByteArray findRecord(const slot_id_t&) override;
        quint8 getFreeSpace() const override;
        QSharedPointer<Storage::Block> toBlock() override;

    private:
        quint16 numberOfSlots;
        QPair<quint16, quint16> freeSpacePointer;
        QList<slotEntry> slotArray;
        QByteArray data;

    };

    class FreePage : public Page
    {
    public:
        FreePage(page_id_t id) : Page(id) {}
        QSharedPointer<Storage::Block> toBlock() override {
            // Since this is a free page, it simply returns a null pointer or an empty implementation,
            // although it is expected this page type will never be written to disk
            return QSharedPointer<Storage::Block>(nullptr);
        }

    };

    // helper method to calculate index page size
    static int indexPageSize(int headerSize) { return Storage::blockSize - headerSize; }

    #define MappingType QPair<KeyType, ValueType>
    class BPTIndexPage : public virtual Page
    {
    /**
     * It actually serves as a header part for each B+ tree page and
     * contains information shared by both leaf page and internal page.
     */
    public:
        /**
         * @brief constructs a new BPTIndexPage, sets its page_id based on the block id
         * @param block from disk
         */
        BPTIndexPage(QSharedPointer<Storage::Block>);

        /**
         * @brief constructs a new BPTIndexPage, sets its page_id based on the block id
         * @param page_id value, when a page is created in memory (the page_id is still being retrieved from disk anyway)
         */
        BPTIndexPage(page_id_t);

        /**
         * @brief helper method to get & set page type
         */
        bool isLeafPage() const;
        bool isRootPage() const;
        void setIndexPageType(Types::PageType);

        /**
         * @brief helper methods to get & set page size
         * size/degree/fanout = number of key/value pairs stored in the page
         */
        int getSize() const;
        void setSize(int);
        void increaseSize(int);

        /**
         * @brief Helper methods to get/set max size (capacity) of the page
         * maxSize/order = maximum number of key/value pairs per node
         */
        int getMaxSize() const;
        void setMaxSize(int);

        /** helper method to get min page size
         * generally, minPageSize == maxPageSize / 2
         * minPageSize = the min. amount of keys in a node
         * it varies whether it's root, internal or leaf
         */
        int getMinSize() const;

        /**
         * @brief Helper methods to get/set parent page id
         */
        page_id_t getParentPageId() const;
        void setParentPageId(page_id_t);

    private:
        // number of key/value pairs (index entries) stored in the page
        int size;
        // maximum number of keys per node
        int maxSize;
        // id of parent page (not root page)
        page_id_t parentPageId;
        // HEADER SIZE: PageType + size + maxSize + parentPageId = 16 bytes in total

    };

    // #define INTERNAL_PAGE_HEADER_SIZE 16
    // #define INTERNAL_PAGE_SIZE ((PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(MappingType)))
    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPTInternalIndexPage : public BPTIndexPage
    {
    /**
     * stores 'n' indexed keys and 'n+1' child pointers (page_id) within internal page.
     * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
     *      K(i) <= K < K(i+1).
     * NOTE: since the number of keys does not equal to number of child pointers,
     * the first key always remains invalid. That is to say, any search/lookup
     * should ignore the first key.
     * (1st entry just holds a pointer, with no value)
     * keys are stored in increasing order.
     */
    public:
        /**
         * @brief Init method after creating a new internal page
         * Including set page type, set current size, set parent id and set max page size
         */
        void init(page_id_t parentId = INVALID_PAGE_ID, int max_size = Storage::blockSize);

        /** @brief
         * Helper methods to get/set the key associated with input 'index'
         * (a.k.a array offset)
         */
        KeyType keyAt(int index) const;
        void setKeyAt(int index, const KeyType &key);

        /** @brief
         * Helper method to find and return array index(or offset),
         * so that its value equals to input "value"
         */
        int valueIndex(const ValueType &value) const;

        /** @brief
         * Helper method to get the value associated with input 'index'
         * (a.k.a array offset)
         */
        ValueType valueAt(int index) const;

        /**
         * @brief LOOKUP
         * Find and return the child pointer(page_id) which points to the child page that contains input "key"
         * Start the search from the second key(the first key should always be invalid)
         * This method performs a binary serach
         * @param key lookup target
         * @param comparator to actually determine lesser/greater than values
         * @return child pointer (page_id) which points to the child page
         */
        ValueType lookup(const KeyType &key, const KeyComparator &comparator) const;

        /**
         * @brief INSERTION
         * Populate new root page with old_value + new_key & new_value
         * When the insertion cause overflow from leaf page all the way upto the root
         * page, we should create a new root page and populate its elements.
         * NOTE: This method is only called within 'insertIntoParent()' (bplustree.cpp)
         */
        void populateNewRoot(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);

        /**
         * @brief insertNodeAfter
         * Insert new_key & new_value pair right after the pair with its value == old_value
         * @return new size after insertion
         */
        int insertNodeAfter(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);

        /**
         * @brief REMOVE
         * Remove the key & value pair in internal page according to input index
         * (a.k.a array offset)
         * NOTE: store key & value pair continuously after deletion
         * @param index index target
         */
        void remove(int index);

        /**
         * @brief removeAndReturnOnlyChild
         * Remove the only key & value pair in internal page and return the value
         * NOTE: this method is only being called within adjustRoot() (bplustree.cpp)
         */
        ValueType removeAndReturnOnlyChild();

        /// Split and Merge utility methods
        /**
         * @brief MERGE
         * Remove all of key & value pairs from this page to "recipient" page.
         * The middle_key is the separation key we should get from the parent.
         * We need to make sure the middle key is added to the recipient to maintain the invariant.
         * We also need to use the BufferPoolManager to persist changes to the parent page id for those
         * pages that are moved to the recipient
         */
        void moveAllTo(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key);

        /**
         * @brief SPLIT
         * Remove half of key & value pairs from this page to "recipient" page
         */
        void moveHalfTo(QSharedPointer<BPTInternalIndexPage> recipient);

        /**
         * @brief REDISTRIBUTE
         * Remove the first key & value pair from this page to tail of "recipient" page.
         *
         * The middle_key is the separation key we should get from the parent.
         * We need to make sure the middle key is added to the recipient to maintain the invariant.
         * We also need to use BufferPoolManager to persist changes to the parent page id for those
         * pages that are moved to the recipient
         */
        void moveFirstToEndOf(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key);

        /** @brief
         * Remove the last key & value pair from this page to head of "recipient" page.
         *
         * We need to handle the original dummy key properly, e.g. updating recipient’s array to position the middle_key at the
         * right place.
         * we also need to use BufferPoolManager to persist changes to the parent page id for those pages that are
         * moved to the recipient
         */
        void moveLastToFrontOf(QSharedPointer<BPTInternalIndexPage> recipient, const KeyType &middle_key);

    private:
        QList<MappingType> array;
        // HEADER SIZE: BPTIndexPage size = 16 bytes in total

        /**
         * @brief Copy entries here, starting from {items} and copy {size} entries.
         * Since it is an internal page, for all entries (pages) moved, their parents page now changes to me.
         * So I need to 'adopt' them by changing their parent page id, which needs to be persisted with BufferPoolManager
         */
        void copyNFrom(const QList<MappingType> &items, int size);

        /**
         * @brief Append an entry at the end.
         * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
         * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManager
         */
        void copyLastFrom(const MappingType &pair);

        /**
         * @brief Append an entry at the beginning.
         * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
         * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
         */
        void copyFirstFrom(const MappingType &pair);

    };

    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPTLeafIndexPage : public BPTIndexPage
    {
    public:
        /**
         * @brief Init method after creating a new leaf page
         * Including set page type, set current size, set parent id and set max page size
         */
        void init(page_id_t parentId = INVALID_PAGE_ID, int max_size = Storage::blockSize);

        /**
         * @brief
         * Helper methods to set/get next page id
         */
        page_id_t getNextPageId() const;
        void setNextPageId(page_id_t next_page_id);

        /**
         * @brief
         * Helper method to find and return the key associated with input "index"
         * (a.k.a array offset)
         */
        KeyType keyAt(int index) const;

        /**
         * @brief
         * Helper methods to find the first index i so that array[i].first >= key
         * NOTE: This method is only used when generating index iterator
         * it performs a binary search
         */
        int keyIndex(const KeyType &key, const KeyComparator &comparator) const;

        /**
         * @brief
         * Helper method to find and return the key & value pair associated with input
         * "index" (a.k.a array offset)
         */
        const MappingType &getItem(int index);

        /**
         * @brief INSERTION
         * Insert key & value pair into leaf page ordered by key
         * @return page size after insertion
         */
        int insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);

        /**
         * @brief LOOKUP
         * For the given key, check to see whether it exists in the leaf page. If it
         * does, then store its corresponding value in input "value" and return true.
         * If the key does not exist, then return false
         */
        bool lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const;

        /**
         * @brief DELETION
         * First look through leaf page to see whether delete key exist or not. If
         * exist, perform deletion, otherwise return immediately.
         * NOTE: store key & value pair continuously after deletion
         * @return page size after deletion
         */
        int removeAndDeleteRecord(const KeyType &key, const KeyComparator &comparator);

        /**
         * @brief SPLIT
         * Remove half of key & value pairs from this page to "recipient" page
         */
        void moveHalfTo(QSharedPointer<BPTLeafIndexPage> recipient);

        /**
         * @brief MERGE
         * Remove all of key & value pairs from this page to "recipient" page.
         * Don't forget to update the next_page id in the sibling page
         */
        void moveAllTo(QSharedPointer<BPTLeafIndexPage> recipient);

        /**
         * @brief REDISTRIBUTE
         * Remove the first key & value pair from this page to "recipient" page.
         */
        void moveFirstToEndOf(QSharedPointer<BPTLeafIndexPage> recipient);

        /**
         * @brief
         * Remove the last key & value pair from this page to "recipient" page.
         */
        void moveLastToFrontOf(QSharedPointer<BPTLeafIndexPage> recipient);

    private:
        QList<MappingType> array;
        page_id_t nextPageId;
        // HEADER SIZE: 16 bytes from BPTIndexPage + nextPageId(4 bytes) = 20 bytes in total

        /**
         * @brief
         * Copy {size} number of elements from {items} into me.
         */
        void copyNFrom(const QList<MappingType>& items, int size);

        /** @brief
         * Copy the item into the end of my item list. (Append item to my array)
         */
        void copyLastFrom(const MappingType &item);

        /**
         * @brief
         * Insert item at the front of my items. Move items accordingly.
         */
        void copyFirstFrom(const MappingType &item);
    };

}

#endif // PAGE_H
