#include "block.h"

Storage::Block::Block(int blockAddress, const QByteArray &data)
    : blockId(blockAddress)
{
    QDataStream in(data);
    in >> header.type;
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

QSharedPointer<Storage::Sector> Storage::Block::getSector(int index)
{
    return sectors.at(index);
}

void Storage::Block::setId(int id)
{
    this->blockId = id;
}

void Storage::Block::setHeader(const Header::BlockType& type)
{
    this->header.type = type;
}

void Storage::Block::setData(const QByteArray &data)
{
    this->data = data;
}

void Storage::Block::setSectors(QList<QSharedPointer<Sector>> sec)
{
    this->sectors = sec;
}
