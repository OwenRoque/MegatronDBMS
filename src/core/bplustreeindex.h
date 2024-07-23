#ifndef BPLUSTREEINDEX_H
#define BPLUSTREEINDEX_H

#include "index.h"
#include "bplustree.h"
#include "generic_comparator.h"
#include "generic_key.h"

namespace Core
{
    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPlusTreeIndex : public Index
    {
    public:
        /**
         * @brief creates a new BPlusTreeIndex
         * @param metadata metadata of this index, retrieved from system catalog
         */
        BPlusTreeIndex(const QList<Core::SystemCatalog::indexMeta>& metadata);
        /**
         * @brief creates an index entry with the given record (key)
         * @param record record object to create its linked index entry
         * @param rid rowId/recordId, location of the record in data file (value)
         */
        void insertEntry(Core::Record& record, record_id rid) override;
        /**
         * @brief deletes an index entry with the given record (key)
         * @param record record object to locate & delete its linked index entry
         * @param rid rowId/recordId, location of the record in data file (value)
         */
        void deleteEntry(Core::Record& record, record_id rid) override;
        /**
         * @brief searches index entries which match the predicate
         * @param record record object to create its linked index entry
         * @param result list of record ids
         */
        void scanKey(Core::Record& record, QList<record_id>& result) override;

    protected:
        KeyComparator comparator;
        BPlusTree<KeyType, ValueType, KeyComparator> tree;

    };
}

#endif // BPLUSTREEINDEX_H
