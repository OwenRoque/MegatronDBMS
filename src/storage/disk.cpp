#include "disk.h"
#include <QDir>

int Storage::sectorSize = 0;
int Storage::blockSize = 0;
int Storage::blockFactor = 0;

Storage::Disk::Disk(const QString &name, int platters, int tracks,
                    int sectors, int sSize, int bSize, bool firstInit) :
    name(name), nPlatters(platters), nTracks(tracks), nSectors(sectors)
{
    sectorSize = sSize;
    blockSize = bSize;
    blockFactor = blockSize / sectorSize;
    createDirectory(name);
    // Configuration file to:
    // - store init disk values
    QFile configFile(name + "/" + "disk.config");
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&configFile);
        out << nPlatters << " " << nTracks << " " << nSectors << " " << sectorSize << " " << blockSize << Qt::endl;
        configFile.close();
    }
    // create storage.bin
    // - store storage manager data (cylinder groups metadata, inodes ...)
    // - it will store catalog tables' location on disk
    QFile manager(name + "/" + "storage.bin");
    if (manager.open(QIODevice::WriteOnly | QIODevice::Text))
        manager.close();

    for (int i = 0, nHead = 0; i < nPlatters; i++, nHead++)
    {
        QString dirName = name + "/" + QString::number(i);
        firstInit ?
            this->platters.emplace_back(new Platter(dirName, tracks, sectors, 0, nHead, 1)) :
            this->platters.emplace_back(new Platter(dirName, tracks, sectors));
    }
}

int Storage::Disk::getNPlatters() const
{
    return nPlatters;
}

int Storage::Disk::getNTracks() const
{
    return nTracks;
}

int Storage::Disk::getNSectors() const
{
    return nSectors;
}

int Storage::Disk::getSectorSize() const
{
    return sectorSize;
}

int Storage::Disk::getBlockSize() const
{
    return blockSize;
}

int Storage::Disk::getBlockFactor() const
{
    return blockFactor;
}

QSharedPointer<Storage::Platter> Storage::Disk::getPlatter(int index)
{
    return platters.at(index);
}

double Storage::Disk::fullDiskSize() const
{
    return nPlatters * 2 * nTracks * nSectors * sectorSize;
}

double Storage::Disk::fullPlaterSize() const
{
    return 2 * nTracks * nSectors * sectorSize;
}

double Storage::Disk::fullSurfaceSize() const
{
    return nTracks * nSectors * sectorSize;
}

double Storage::Disk::fullTrackSize() const
{
    return nSectors * sectorSize;
}

Utility::Space Storage::Disk::getSpace() const
{
    Space d;
    for (const auto& platter : platters)
    {
        Space p = platter->getSpace();
        d.usedSpaceSize += p.usedSpaceSize;
        d.usedDiskSpace += p.usedDiskSpace;
        d.freeSpaceSize += p.freeSpaceSize;
        d.freeDiskSpace += p.freeDiskSpace;
    }
    return d;
}
