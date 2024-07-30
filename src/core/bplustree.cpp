#include "bplustree.h"
#include <buffermanager.h>

template<typename KeyType, typename ValueType, typename KeyComparator>
Core::BPlusTree<KeyType, ValueType, KeyComparator>::BPlusTree(const QString &name, const KeyComparator &comparator, int leaf_max_size, int internal_max_size)
{
    indexName = name;
    rootPageId = INVALID_PAGE_ID;
    this->comparator = comparator;
    leafMaxSize = leaf_max_size;
    internalMaxSize = internal_max_size;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::isEmpty() const
{
    return rootPageId == INVALID_PAGE_ID;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::insert(const KeyType &key, const ValueType &value)
{
    if (!isEmpty()) {
        return insertIntoLeaf(key, value);
    } else {
        startNewTree(key, value);
        return true;
    }
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTree<KeyType, ValueType, KeyComparator>::remove(const KeyType &key)
{
    if (isEmpty()) {
        return;
    }
    // Locate the leaf node and delete the key-value pair
    auto leaf_page = findLeafPage(key, false, OperationType::REMOVE);
    QSharedPointer<LeafPage> leaf = toLeafPage(leaf_page);
    int size = leaf->removeAndDeleteRecord(key, comparator);
    // Leaf nodes not in half-full state after deletion need to be
    // - merged with neighboring nodes or
    // - redistributed
    if (size < leaf->getMinSize()) {
        coalesceOrRedistribute(leaf);
    }

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    bm->unpinPage(leaf->getId(), true);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::getValue(const KeyType &key, QList<ValueType> *result)
{
    if (isEmpty()) {
        return false;
    }

    // Find key in leaf node
    auto leaf_page = findLeafPage(key);
    QSharedPointer<LeafPage> leaf = toLeafPage(leaf_page);

    ValueType value;
    auto success = leaf->lookup(key, &value, comparator);
    if (success) {
        result->push_back(value);
    }

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    bm->unpinPage(leaf->GetPageId(), false);
    return success;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTree<KeyType, ValueType, KeyComparator>::startNewTree(const KeyType &key, const ValueType &value)
{
    // Create a leaf node as the root node and insert new data
    QSharedPointer<LeafPage> root = toLeafPage(newPage(&rootPageId));
    root->init(rootPageId, INVALID_PAGE_ID, leafMaxSize);
    root->insert(key, value, comparator);

    updateRootPageId(1);

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    bm->unpinPage(rootPageId, true);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::insertIntoLeaf(const KeyType &key, const ValueType &value)
{
    // Locate the leaf node containing the key
    auto leaf_page = findLeafPage(key, false, OperationType::INSERT);
    QSharedPointer<LeafPage> leaf = toLeafPage(leaf_page);

    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    ValueType exist_value;
    // Cannot insert the same key
    if (leaf->lookup(key, &exist_value, comparator)) {
        bm->unpinPage(leaf->getId(), false);
        return false;
    }

    // If the leaf node is not full, insert it directly (leaving a space at the end of the array),
    // otherwise split the leaf node and update the parent.
    auto size = leaf->insert(key, value, comparator);

    if (size == leafMaxSize) {
        QSharedPointer<LeafPage> new_leaf = split(leaf);
        insertIntoParent(leaf, new_leaf->keyAt(0), new_leaf);
        bm->unpinPage(new_leaf->getId(), true);
    }

    bm->unpinPage(leaf->getId(), true);
    return true;
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTree<KeyType, ValueType, KeyComparator>::insertIntoParent(QSharedPointer<BPTIndexPage> old_node, const KeyType &key, QSharedPointer<BPTIndexPage> new_node)
{
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    // Split of root node requires a new root node, height of B+tree +1
    if (old_node->isRootPage()) {
        auto root_page = newPage(&rootPageId);
        // Create a new node and update the child node pointers
        QSharedPointer<InternalPage> root = toInternalPage(root_page);
        root->init(rootPageId, INVALID_PAGE_ID, internalMaxSize);
        root->populateNewRoot(old_node->getId(), key, new_node->getId());
        // Update the parent node pointer
        old_node->setParentPageId(rootPageId);
        new_node->setParentPageId(rootPageId);

        updateRootPageId(0);

        bm->unpinPage(rootPageId, true);
        return;
    }
    // Find the parent node and insert the leftmost key of the new node into it.
    auto parent_id = old_node->getParentPageId();
    QSharedPointer<InternalPage> parent = toInternalPage(bm->fetchPage(parent_id));
    auto size = parent->InsertNodeAfter(old_node->getId(), key, new_node->getId());
    // Need to split again if parent overflows (recursively)
    if (size == internalMaxSize) {
        QSharedPointer<InternalPage> new_page = split(parent);
        insertIntoParent(parent, new_page->keyAt(0), new_page);
        bm->unpinPage(new_page->getId(), true);
        bm->unpinPage(parent_id, true);
    } else {
        bm->unpinPage(parent_id, true);
    }
}

template<typename KeyType, typename ValueType, typename KeyComparator>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::adjustRoot(QSharedPointer<BPTIndexPage> old_root_node)
{
    bool is_deleted = false;
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    // Case 1: Internal node with only one child, promote the child as new root
    if (!old_root_node->isLeafPage() && old_root_node->getSize() == 1) {
        auto old_root = qSharedPointerDynamicCast<InternalPage>(old_root_node);
        rootPageId = old_root->removeAndReturnOnlyChild();

        // Fetch and update child node
        auto child = qSharedPointerDynamicCast<InternalPage>(bm->fetchPage(rootPageId));
        child->setParentPageId(INVALID_PAGE_ID);
        bm->unpinPage(rootPageId, true);

        updateRootPageId();
        is_deleted = true;

        // Case 2: Leaf node with no keys, delete the tree
    } else if (old_root_node->isLeafPage() && old_root_node->getSize() == 0) {
        rootPageId = INVALID_PAGE_ID;
        updateRootPageId();
        is_deleted = true;
    }
}

template<typename KeyType, typename ValueType, typename KeyComparator>
template<typename N>
QSharedPointer<N> Core::BPlusTree<KeyType, ValueType, KeyComparator>::split(QSharedPointer<N> node)
{
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    // create new page
    page_id_t new_page_id;
    auto new_frame = bm->newPage(&new_page_id);

    // Convert the frame to the appropriate page type
    auto new_page = new_frame->getPage();
    if (!new_page) {
        throw std::runtime_error("Out of memory");
    }
    QSharedPointer<N> new_node = qSharedPointerDynamicCast<N>(new_page);

    // Initialize the new node and move half of the items to it
    new_node->init(new_page_id, node->getParentPageId(), node->getMaxSize());
    node->moveHalfTo(new_node);

    return new_node;
}



// template<typename KeyType, typename ValueType, typename KeyComparator>
// template<typename N>
// QSharedPointer<N> Core::BPlusTree<KeyType, ValueType, KeyComparator>::createNewPage(page_id_t *new_page_id)
// {
//     Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
//     QSharedPointer<Memory::Frame> frame = bm->newPage(new_page_id);
//     if (frame == nullptr) {
//         throw std::runtime_error("Out of memory");
//     }

//     QSharedPointer<N> new_page = QSharedPointer<N>::create(*new_page_id);
//     frame->setPage(new_page);
//     return new_page;
// }

template<typename KeyType, typename ValueType, typename KeyComparator>
template<typename N>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::coalesceOrRedistribute(QSharedPointer<N> node)
{
    if (node->isRootPage()) {
        return adjustRoot(node);
    }

    // Find the sibling of the input page
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    auto parent_frame = bm->fetchPage(node->getParentPageId());
    QSharedPointer<InternalPage> parent = qSharedPointerDynamicCast<InternalPage>(parent_frame->getPage());

    int index = parent->valueIndex(node->getId());
    // index 0 must have a right sibling.
    int sibling_index = index > 0 ? index - 1 : 1;
    auto sibling_frame = bm->fetchPage(parent->valueAt(sibling_index));
    QSharedPointer<N> sibling = qSharedPointerDynamicCast<N>(sibling_frame->getPage());

    // If the combined size of the nodes is greater than max_size-1, redistribute, otherwise merge
    bool is_merge = sibling->getSize() + node->getSize() <= node->getMaxSize() - 1;
    if (is_merge) {
        coalesce(sibling, node, parent, index);
    } else {
        redistribute(sibling, node, index);
    }

    bm->unpinPage(parent->GetPageId(), true);
    bm->unpinPage(sibling->GetPageId(), true);

    return is_merge;
}


template<typename KeyType, typename ValueType, typename KeyComparator>
template<typename N>
bool Core::BPlusTree<KeyType, ValueType, KeyComparator>::coalesce(QSharedPointer<N> neighbor_node, QSharedPointer<N> node, QSharedPointer<InternalPage> parent, int index)
{
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();

    // If sibling node is on the right, swap pointers so data moves from right to left
    if (index == 0) {
        std::swap(node, neighbor_node);
    }

    QSharedPointer<N> child = node, neighbor_child = neighbor_node;
    QSharedPointer<InternalPage> parent_node = qSharedPointerDynamicCast<InternalPage>(parent);

    // For internal nodes, get the middle key from the parent
    KeyType middle_key;
    auto middle_index = index == 0 ? 1 : index;
    if (!child->IsLeafPage()) {
        middle_key = parent_node->KeyAt(middle_index);
    }

    // Move all key-value pairs to the sibling node, then delete the node
    child->moveAllTo(neighbor_child, middle_key);
    bm->unpinPage(child->getId(), true);
    bm->deletePage(child->getId());

    // Remove the key-value pair from the parent node and recursively adjust the parent
    parent_node->remove(middle_index);
    return coalesceOrRedistribute(parent_node);
}

template<typename KeyType, typename ValueType, typename KeyComparator>
template<typename N>
void Core::BPlusTree<KeyType, ValueType, KeyComparator>::redistribute(QSharedPointer<N> neighbor_node, QSharedPointer<N> node, int index)
{
    Memory::BufferManager* bm = &Memory::BufferManager::getInstance();
    // Fetch the parent page
    QSharedPointer<InternalPage> parent = qSharedPointerDynamicCast<InternalPage>(
        bm->fetchPage(node->getParentPageId()));

    // Get the middle key from the parent if the node is not a leaf
    KeyType middle_key;
    auto middle_index = index == 0 ? 1 : index;
    if (!node->isLeafPage()) {
        middle_key = parent->keyAt(middle_index);
    }

    // Move the key-value pairs between nodes based on the sibling's position
    if (index == 0) {
        neighbor_node->moveFirstToEndOf(node, middle_key);
        parent->setKeyAt(middle_index, neighbor_node->keyAt(0));
    } else {
        neighbor_node->moveLastToFrontOf(node, middle_key);
        parent->setKeyAt(middle_index, node->keyAt(0));
    }

    bm->unpinPage(parent->getId(), true);
}
