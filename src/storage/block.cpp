#include "block.h"
#include "disk.h"

Storage::Block::Block(Storage::block_id_t blockId, const QByteArray &data)
    : id(blockId)
{
    // block address (LBA value will change in intervals, depending on blockFactor)
    this->address = blockId * Storage::blockFactor;
    QDataStream in(data);
    in >> blockType;
    in >> this->data;
}

Utility::Space Storage::Block::getSpace() const
{
    Space b;
    for (const auto& sector : sectors)
    {
        Space s = sector->getSpace();
        b.usedSpaceSize += s.usedSpaceSize;
        b.usedDiskSpace += s.usedDiskSpace;
        b.freeSpaceSize += s.freeSpaceSize;
        b.freeDiskSpace += s.freeDiskSpace;
    }
    return b;
}

Storage::block_id_t Storage::Block::getId() const
{
    return id;
}

Storage::BlockType Storage::Block::getHeader() const
{
    return blockType;
}

QByteArray Storage::Block::getData() const
{
    return data;
}

QSharedPointer<Storage::Sector> Storage::Block::getSector(int index)
{
    return sectors.at(index);
}

void Storage::Block::setId(Storage::block_id_t id)
{
    this->id = id;
    this->address = id * Storage::blockFactor;
}

void Storage::Block::setHeader(const Storage::BlockType& type)
{
    this->blockType = type;
}

void Storage::Block::setData(const QByteArray &data)
{
    this->data = data;
}

void Storage::Block::setSectors(QList<QSharedPointer<Sector>> sec)
{
    this->sectors = sec;
}
