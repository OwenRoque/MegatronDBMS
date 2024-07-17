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
        FreeSpaceMap(const QList<int>& init_list);

        void insert(block_id_t blockId, quint8 freeSpaceFraction);
        block_id_t getBlockWithMoreFreeSpace();
        QByteArray toBytes() const;
        void fromBytes(const QByteArray& fsm);
        void printHeap() const;
        size_t size() const;

        /**
         * Serialization of Free Space Map File
         * This file shouldn't be saved as an object in manager.bin
         * Instead, it should have a corresponding derived Page, so that it can be uploaded to
         * memory in blocks of data. We're simplifying its storage in this implementation
         * "(...) The array is stored in a file, whose blocks are fetched into memory, as required (Ch. 13, pg. 596)"
         */

        friend QDataStream& operator<<(QDataStream& out, const Core::FreeSpaceMap& fsm)
        {
            QByteArray byteArray = fsm.toBytes();
            out << byteArray;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, Core::FreeSpaceMap& fsm) {
            QByteArray byteArray;
            in >> byteArray;
            fsm.fromBytes(byteArray);
            return in;
        }

    private:
        std::priority_queue<QPair, std::vector<QPair>, Compare> maxHeap;

    };

}

#endif // FREESPACEMAP_H
