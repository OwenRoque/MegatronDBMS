#include "bplustreeindex.h"

template<typename KeyType, typename ValueType, typename KeyComparator>
Core::BPlusTreeIndex<KeyType, ValueType, KeyComparator>::BPlusTreeIndex(const QList<SystemCatalog::indexMeta> &metadata)
    : Index(metadata), comparator(metadata)
{
    // init the comprator & tree ...
}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTreeIndex<KeyType, ValueType, KeyComparator>::insertEntry(Record &record, record_id rid)
{

}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTreeIndex<KeyType, ValueType, KeyComparator>::deleteEntry(Record &record, record_id rid)
{

}

template<typename KeyType, typename ValueType, typename KeyComparator>
void Core::BPlusTreeIndex<KeyType, ValueType, KeyComparator>::scanKey(Record &record, QList<record_id> &result)
{

}

