#include "sector.h"
#include "disk.h"

Storage::Sector::Sector(const QString &name, int nCylinder, int nHead, int nSector)
{
    sectorPath = name + ".bin";
    chsAddress = std::make_tuple(nCylinder, nHead, nSector);
    QFile sector(sectorPath);
    if (sector.open(QIODevice::WriteOnly | QIODevice::Text) && sector.size() == 0)
    {
        currentSize = 0;
        QDataStream out(&sector);
        // write sector header
        // it does not occupy bytes from the data-bytes defined on disk
        out << nCylinder << nHead << nSector << currentSize;
        sector.close();
    }
}

Storage::Sector::Sector(const QString &name)
{
    sectorPath = name + ".bin";
    QFile sector(sectorPath);
    int nCylinder, nHead, nSector;
    if (sector.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDataStream in(&sector);
        // QString delim;
        in >> nCylinder >> nHead >> nSector >> currentSize;
        chsAddress = std::make_tuple(nCylinder, nHead, nSector);
        sector.close();
    }
}

Utility::Space Storage::Sector::getSpace() const
{
    Space s;
    if (currentSize == 0)
    {
        s.freeSpaceSize = s.freeDiskSpace = sectorSize;
        s.usedSpaceSize = s.usedDiskSpace = 0;
    }
    else
    {
        s.usedSpaceSize = currentSize;
        s.usedDiskSpace = sectorSize;
        s.freeSpaceSize = sectorSize - currentSize;
        s.freeDiskSpace = 0;
    }
    return s;
}

QString Storage::Sector::getSectorPath() const
{
    return sectorPath;
}

std::tuple<int, int, int> Storage::Sector::getChsAddress() const
{
    return chsAddress;
}

int Storage::Sector::getCurrentSize() const
{
    return currentSize;
}

void Storage::Sector::setCurrentSize(int newCurrentSize)
{
    currentSize = newCurrentSize;
}

void Storage::Sector::print() const
{
    qDebug() << "(" << std::get<0>(chsAddress) << ", " << std::get<1>(chsAddress) << ", " << std::get<2>(chsAddress) << ")"
             << " -> " << (std::get<0>(chsAddress) * 10 + std::get<1>(chsAddress)) * 8 + (std::get<2>(chsAddress) - 1);
}

