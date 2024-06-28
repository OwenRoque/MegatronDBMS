#include "sector.h"
#include "disk.h"

Storage::Sector::Sector(const QString &name, quint16 nCylinder, quint16 nHead, quint16 nSector)
{
    sectorPath = name + ".bin";
    chsAddress = std::make_tuple(nCylinder, nHead, nSector);
    QFile sector(sectorPath);
    if (sector.open(QIODevice::WriteOnly) && sector.size() == 0)
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
    if (sector.open(QIODevice::ReadOnly))
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

std::tuple<quint16, quint16, quint16> Storage::Sector::getChsAddress() const
{
    return chsAddress;
}

quint16 Storage::Sector::getCurrentSize() const
{
    return currentSize;
}

void Storage::Sector::setCurrentSize(quint16 newCurrentSize)
{
    currentSize = newCurrentSize;
}

qint64 Storage::Sector::size() const
{
    QFile sector(sectorPath);
    qint64 size = sector.size() - (4 * sizeof(quint16));
    qDebug() << size;
    return size;
}

void Storage::Sector::print() const
{
    qDebug() << "(" << std::get<0>(chsAddress) << ", " << std::get<1>(chsAddress) << ", " << std::get<2>(chsAddress) << ")"
             << " -> " << (std::get<0>(chsAddress) * 10 + std::get<1>(chsAddress)) * 8 + (std::get<2>(chsAddress) - 1);
}

