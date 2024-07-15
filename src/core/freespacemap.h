#ifndef FREESPACEMAP_H
#define FREESPACEMAP_H

#include <queue>
#include <vector>
#include <utility>
#include <QDebug>
#include <QByteArray>
#include <QIODevice>
#include <block.h>

namespace Core
{
    using block_id_t = Storage::block_id_t;

    class FreeSpaceMap
    {
    public:
        using QPair = std::pair<block_id_t, quint8>;
        struct Compare {
            bool operator()(const QPair &a, const QPair &b) {
                // max-heap based on free-space fraction
                return a.second < b.second;
            }
        };

        FreeSpaceMap() = default;

        void insert(block_id_t blockId, quint8 freeSpaceFraction);
        block_id_t getBlockWithMoreFreeSpace();
        QByteArray toBytes() const;
        void fromBytes(const QByteArray& fsm);
        void printHeap() const;
        size_t size() const;

    private:
        std::priority_queue<QPair, std::vector<QPair>, Compare> maxHeap;
    };
}

#endif // FREESPACEMAP_H
