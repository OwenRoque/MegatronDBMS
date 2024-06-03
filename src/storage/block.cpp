#include "block.h"

Storage::Block::Block(int blockAddress, QByteArray &data) : blockId(blockAddress)
{
    QDataStream in(data);
    in >> header.type;
    in >> this->data;
}

void Storage::Block::setSectors(QList<QSharedPointer<Sector>> sec)
{
    sectors = sec;
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

int Storage::Block::getBlockId() const
{
    return blockId;
}

Storage::Block::Header Storage::Block::getHeader() const
{
    return header;
}

QByteArray Storage::Block::getData() const
{
    return data;
}

void Storage::Block::setHeader(Header::BlockType type)
{
    header.type = type;
}

void Storage::Block::setData(const QByteArray &data)
{
    this->data = data;
}

QSharedPointer<Storage::Sector> Storage::Block::getSector(int index)
{
    return sectors.at(index);
}
