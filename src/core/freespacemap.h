#ifndef FREESPACEMAP_H
#define FREESPACEMAP_H

#include <queue>
#include <vector>
#include <utility>
#include <QDebug>
#include <QByteArray>
#include <QIODevice>

namespace Core
{
    class FreeSpaceMap
    {
    public:
        using QPair = std::pair<qint64, quint8>;
        struct Compare {
            bool operator()(const QPair &a, const QPair &b) {
                // max-heap based on free-space fraction
                return a.second < b.second;
            }
        };

        FreeSpaceMap() = default;

        void insert(quint64 blockId, quint8 freeSpaceFraction);
        quint64 getBlockWithMoreFreeSpace();
        QByteArray toBytes() const;
        void fromBytes(const QByteArray& fsm);
        void printHeap() const;
        size_t size() const;

    private:
        std::priority_queue<QPair, std::vector<QPair>, Compare> maxHeap;
    };
}

#endif // FREESPACEMAP_H
