#include "disk.h"

#include <QDir>

Disk::Disk(const QString &name, int platters, int tracks, int sectors, int sSize, int bSize) :
    name(name), nPlatters(platters), nTracks(tracks), nSectors(sectors), sectorSize(sSize), blockSize(bSize)
{
    blockFactor = blockSize / sectorSize;
}

void Disk::init()
{
    createDirectory(name);
    // Configuration file to:
    // - store init disk values
    // - manage free space in disk (QBitArray)
    // - store sys. cat. blocks to load to cache
    QFile configFile(name + "/disk.config");
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        out << nPlatters << " " << nTracks << " " << nSectors << " " << sectorSize << " " << blockSize << Qt::endl;
        for (int i = 0; i < fullDiskSize() / blockSize; i++)
            out << 0;
        // syscat blocks go here
        configFile.close();
    }
    for (int i = 0; i < nPlatters; i++) {
        createDirectory(name + "/" + QString::number(i));
        for (int j = 0; j < 2; j++) {
            createDirectory(name + "/" + QString::number(i) +
                            "/" + QString::number(j));
            for (int k = 0; k < nTracks; k++) {
                createDirectory(name + "/" + QString::number(i) +
                                "/" + QString::number(j) +
                                "/" + QString::number(k));
                for (int l = 0; l < nSectors; l++) {
                    QFile sector(name + "/" + QString::number(i) +
                                 "/" + QString::number(j) +
                                 "/" + QString::number(k) +
                                 "/" + QString::number(l) + ".txt");
                    if (sector.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&sector);
                        out << "# SECTOR HEADER" << Qt::endl;   // works as delimiter
                        out << 4 << Qt::endl;
                        if (l % blockFactor == 0) {
                            // Add block header after every blockFactor sectors
                            out << "# BLOCK HEADER" << Qt::endl;
                            // Undefined blockType, subtype atm
                            out << Qt::endl;
                        }
                        sector.close();
                    }
                }
            }
        }
    }
}

void Disk::createDirectory(const QString &name)
{
    QDir directory(name);
    if (!directory.exists())
        directory.mkpath(name);
}

int Disk::getNTracks() const
{
    return nTracks;
}

int Disk::getNSectors() const
{
    return nSectors;
}

int Disk::getSectorSize() const
{
    return sectorSize;
}

int Disk::getBlockSize() const
{
    return blockSize;
}

double Disk::fullDiskSize() const
{
    return nPlatters * 2 * nTracks * nSectors * sectorSize;
}

double Disk::fullPlaterSize() const
{
    return 2 * nTracks * nSectors * sectorSize;
}

double Disk::fullSurfaceSize() const
{
    return nTracks * nSectors * sectorSize;
}

double Disk::fullTrackSize() const
{
    return nSectors * sectorSize;
}

int Disk::getBlockFactor() const
{
    return blockFactor;
}
