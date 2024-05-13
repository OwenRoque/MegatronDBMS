#include "platter.h"

Storage::Platter::Platter(const QString &name, int nTracks, int nSectors, int nCylinder, int nHead, int nSector)
{
    createDirectory(name);
    QString dirNameFront = name + "/" + QString::number(0);
    QString dirNameBack = name + "/" + QString::number(1);
    front = QSharedPointer<Surface>(new Surface(dirNameFront, nTracks, nSectors, nCylinder, nHead * 2, nSector));
    back = QSharedPointer<Surface>(new Surface(dirNameBack, nTracks, nSectors, nCylinder, nHead * 2 + 1, nSector));
}

Storage::Platter::Platter(const QString &name, int nTracks, int nSectors)
{
    createDirectory(name);
    QString dirNameFront = name + "/" + QString::number(0);
    QString dirNameBack = name + "/" + QString::number(1);
    front = QSharedPointer<Surface>(new Surface(dirNameFront, nTracks, nSectors));
    back = QSharedPointer<Surface>(new Surface(dirNameBack, nTracks, nSectors));
}

QSharedPointer<Storage::Surface> Storage::Platter::getSurface(int index)
{
    if (index == 0)
        return front;
    else
        return back;
}

Utility::Space Storage::Platter::getSpace() const
{
    Space p;

    Space ft = front->getSpace();
    p.usedSpaceSize += ft.usedSpaceSize;
    p.usedDiskSpace += ft.usedDiskSpace;
    p.freeSpaceSize += ft.freeSpaceSize;
    p.freeDiskSpace += ft.freeDiskSpace;
    Space bk = back->getSpace();
    p.usedSpaceSize += bk.usedSpaceSize;
    p.usedDiskSpace += bk.usedDiskSpace;
    p.freeSpaceSize += bk.freeSpaceSize;
    p.freeDiskSpace += bk.freeDiskSpace;

    return p;
}
