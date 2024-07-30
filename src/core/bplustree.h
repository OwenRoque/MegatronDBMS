#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "page.h"

namespace Core
{
    // define B+ tree operation type enum
    enum class OperationType { READ = 0, INSERT, REMOVE };

    /**
     * Main class providing the API for the B+ Tree.
     *
     * Implementation of simple b+ tree data structure where internal pages direct
     * the search and leaf pages contain actual data.
     * (1) Only unique keys are supported (PRIMARY, UNIQUE constraints only)
     * (2) Supports insert & remove operations
     * (3) The structure should shrink and grow dynamically
     * (4) TODO: Implement index iterator for range scan
     */
    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPlusTree
    {
        using InternalPage = Core::BPTInternalIndexPage<KeyType, ValueType, KeyComparator>;
        using LeafPage = Core::BPTLeafIndexPage<KeyType, ValueType, KeyComparator>;

    public:
        explicit BPlusTree(const QString& name, const KeyComparator &comparator, int leaf_max_size, int internal_max_size);

        /**
         * @return true if this B+ tree has no keys and values.
         */
        bool isEmpty() const;

        /**
         * @brief INSERTION
         * Insert a key-value pair into this B+ tree.
         * if current tree is empty, start new tree, update root page id and insert
         * entry, otherwise insert into leaf page.
         * @return since we only support unique key, if user try to insert duplicate
         * keys return false, otherwise return true.
         */
        bool insert(const KeyType &key, const ValueType &value);

        /**
         * @brief Remove a key and its value from this B+ tree.
         * Delete key & value pair associated with input key
         * If current tree is empty, return immdiately.
         * If not, User needs to first find the right leaf page as deletion target, then
         * delete entry from leaf page. Remember to deal with redistribute or merge if
         * necessary.
         */
        void remove(const KeyType &key);

        /**
         * @brief return the value associated with a given key
         * save the result in the container
         */
        bool getValue(const KeyType &key, QList<ValueType>* result);

    private:
        QString indexName;
        page_id_t rootPageId;
        KeyComparator comparator;
        int leafMaxSize;
        int internalMaxSize;

        /**
         * @brief Insert constant key & value pair into an empty tree
         * User needs to first ask for new page from buffer pool manager
         * (NOTICE: throw an "out of memory" exception if returned value is nullptr),
         * then update b+ tree's root page id and insert entry directly into leaf page.
         */
        void startNewTree(const KeyType &key, const ValueType &value);

        /**
         * @brief Insert constant key & value pair into leaf page
         * User needs to first find the right leaf page as insertion target, then look
         * through leaf page to see whether insert key exist or not. If exist, return
         * immdiately, otherwise insert entry. Remember to deal with split if necessary.
         * @return since we only support unique key, if user try to insert duplicate
         * keys return false, otherwise return true.
         */
        bool insertIntoLeaf(const KeyType &key, const ValueType &value);

        /**
         * @brief Insert key & value pair into internal page after split
         * @param old_node input page from split() method
         * @param key
         * @param new_node returned page from split() method
         * User needs to first find the parent page of old_node, parent node must be
         * adjusted to take info of new_node into account. Remember to deal with split
         * recursively if necessary.
         */
        void insertIntoParent(QSharedPointer<Core::BPTIndexPage> old_node, const KeyType &key, QSharedPointer<Core::BPTIndexPage> new_node);

        /**
         * @brief SPLIT
         * Split input page and return newly created page.
         * Using template N to represent either internal page or leaf page.
         * User needs to first ask for new page from buffer pool manager
         * (NOTICE: throw an "out of memory" exception if returned value is nullptr),
         * then move half of key & value pairs from input page to newly created page
         */
        template <typename N>
        QSharedPointer<N> split(QSharedPointer<N> node);

        /**
         * @brief coalesceOrRedistribute
         * User needs to first find the sibling of input page. If sibling's size + input
         * page's size > page's max size, then redistribute. Otherwise, merge.
         * Using template N to represent either internal page or leaf page.
         * @return: true means target leaf page should be deleted, false means no
         * deletion happens
         */
        template <typename N>
        bool coalesceOrRedistribute(QSharedPointer<N> node);

        /**
         * @brief coalesce
         * Move all the key & value pairs from one page to its sibling page, and notify
         * buffer pool manager to delete this page. Parent page must be adjusted to
         * take info of deletion into account. Remember to deal with coalesce or
         * redistribute recursively if necessary.
         * Using template N to represent either internal page or leaf page.
         * @param   neighbor_node sibling page of input "node"
         * @param   node input from method coalesceOrRedistribute()
         * @param   parent parent page of input "node"
         * @return  true means parent node should be deleted, false means no deletion happened
         */
        template <typename N>
        bool coalesce(QSharedPointer<N> neighbor_node, QSharedPointer<N> node, QSharedPointer<InternalPage> parent, int index);

        /**
         * @brief redistribute
         * Redistribute key & value pairs from one page to its sibling page. If index ==
         * 0, move sibling page's first key & value pair into end of input "node",
         * otherwise move sibling page's last key & value pair into head of input
         * "node".
         * Using template N to represent either internal page or leaf page.
         * @param   neighbor_node      sibling page of input "node"
         * @param   node               input from method coalesceOrRedistribute()
         */
        template <typename N>
        void redistribute(QSharedPointer<N> neighbor_node, QSharedPointer<N> node, int index);

        /**
         * @brief Update root page if necessary
         * NOTE: size of root page can be less than min size and this method is only
         * called within coalesceOrRedistribute() method
         * case 1: when you delete the last element in root page, but root page still
         * has one last child
         * case 2: when you delete the last element in whole b+ tree
         * @return true means root page should be deleted, false means no deletion happened
         */
        bool adjustRoot(QSharedPointer<BPTIndexPage> old_root_node);

        /**
         * @brief findLeafPage
         * Find leaf page containing particular key, if leftMost flag == true, find
         * the left most leaf page
         */
        QSharedPointer<Page> findLeafPage(const KeyType &key, bool leftMost = false, OperationType operation = OperationType::READ);

        /**
         * @brief updateRootPageId
         * @param insert_record
         */
        void updateRootPageId(int insert_record = 0);

        QSharedPointer<BPTIndexPage> toTreePage(QSharedPointer<Page> page) {
            return qSharedPointerDynamicCast<BPTIndexPage>(page);
        }
        QSharedPointer<InternalPage> toInternalPage(QSharedPointer<Page> page) {
            return qSharedPointerDynamicCast<InternalPage>(page);
        }
        QSharedPointer<LeafPage> toLeafPage(QSharedPointer<Page> page) {
            return qSharedPointerDynamicCast<LeafPage>(page);
        }

        // QSharedPointer<InternalPage> toInternalPage(QSharedPointer<BPTIndexPage> page) {
        //     return qSharedPointerCast<InternalPage>(page);
        // }

        QSharedPointer<Page> newPage(page_id_t *page_id);

        // template <typename N>
        // QSharedPointer<N> createNewPage(page_id_t* new_page_id);

        bool isPageSafe(QSharedPointer<BPTIndexPage> page, OperationType operation);
    };
}

#endif // BPLUSTREE_H
