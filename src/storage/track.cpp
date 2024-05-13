#include "track.h"

Storage::Track::Track(const QString &name, int nSectors, int nCylinder, int nHead, int nSector)
{
    createDirectory(name);
    for (int i = 0, nSectorIndex = nSector; i < nSectors; i++, nSectorIndex++)
    {
        QString dirName = name + "/" + QString::number(i);
        this->sectors.emplace_back(new Sector(dirName, nCylinder, nHead, nSectorIndex));
    }
}

Storage::Track::Track(const QString &name, int nSectors)
{
    createDirectory(name);
    for (int i = 0; i < nSectors; i++)
    {
        QString dirName = name + "/" + QString::number(i);
        this->sectors.emplace_back(new Sector(dirName));
    }
}

Utility::Space Storage::Track::getSpace() const
{
    Space t;
    for (const auto& sector : sectors)
    {
        Space s = sector->getSpace();
        t.usedSpaceSize += s.usedSpaceSize;
        t.usedDiskSpace += s.usedDiskSpace;
        t.freeSpaceSize += s.freeSpaceSize;
        t.freeDiskSpace += s.freeDiskSpace;
    }
    return t;
}

QSharedPointer<Storage::Sector> Storage::Track::getSector(int index)
{
    return sectors.at(index);
}
