#include "surface.h"

Storage::Surface::Surface(const QString &name, int nTracks, int nSectors, quint16 nCylinder, quint16 nHead, quint16 nSector)
{
    createDirectory(name);
    for (int i = 0, nCylinderIndex = nCylinder; i < nTracks; i++, nCylinderIndex++)
    {
        QString dirName = name + "/" + QString::number(i);
        this->tracks.emplace_back(new Track(dirName, nSectors, nCylinderIndex, nHead, nSector));
    }
}

Storage::Surface::Surface(const QString &name, int nTracks, int nSectors)
{
    createDirectory(name);
    for (int i = 0; i < nTracks; i++)
    {
        QString dirName = name + "/" + QString::number(i);
        this->tracks.emplace_back(new Track(dirName, nSectors));
    }
}

QSharedPointer<Storage::Track> Storage::Surface::getTrack(int index)
{
    return tracks.at(index);
}

Utility::Space Storage::Surface::getSpace() const
{
    Space s;
    for (const auto& track : tracks)
    {
        Space t = track->getSpace();
        s.usedSpaceSize += t.usedSpaceSize;
        s.usedDiskSpace += t.usedDiskSpace;
        s.freeSpaceSize += t.freeSpaceSize;
        s.freeDiskSpace += t.freeDiskSpace;
    }
    return s;
}
