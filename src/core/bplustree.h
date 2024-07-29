#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "page.h"

namespace Core
{
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
    using

    public:
        BPlusTree();
    };
}

#endif // BPLUSTREE_H
