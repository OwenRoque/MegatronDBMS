#include "freespacemap.h"

void Core::FreeSpaceMap::insert(quint64 blockId, quint8 freeSpaceFraction)
{
    if (freeSpaceFraction > 255) {
        throw std::invalid_argument("Free space fraction must be a value between 0 and 255.");
    }
    maxHeap.push(QPair(blockId, freeSpaceFraction));
}

quint64 Core::FreeSpaceMap::getBlockWithMoreFreeSpace()
{
    if (maxHeap.empty()) {
        throw std::runtime_error("FreeSpaceMap: maxHeap is empty.");
    }
    QPair topPair = maxHeap.top();
    maxHeap.pop();
    return topPair.first;
}

QByteArray Core::FreeSpaceMap::toBytes() const
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);

    auto tempHeap = maxHeap;
    while (!tempHeap.empty()) {
        QPair pair = tempHeap.top();
        tempHeap.pop();
        stream << pair.first << pair.second;
    }

    return byteArray;
}

void Core::FreeSpaceMap::fromBytes(const QByteArray &fsm)
{
    QDataStream stream(fsm);

    qint64 blockId;
    quint8 freeSpaceFraction;
    while (!stream.atEnd()) {
        stream >> blockId >> freeSpaceFraction;
        maxHeap.push(QPair(blockId, freeSpaceFraction));
    }
}

void Core::FreeSpaceMap::printHeap() const
{
    auto tempHeap = maxHeap;
    while (!tempHeap.empty()) {
        QPair pair = tempHeap.top();
        tempHeap.pop();
        qDebug() << "Block ID: " << pair.first << ", Free Space Fraction: " << (int)pair.second;
    }
}

size_t Core::FreeSpaceMap::size() const
{
    return maxHeap.size();
}
