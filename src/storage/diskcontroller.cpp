#include "diskcontroller.h"

#include <QTextStream>
#include <QMessageBox>

Storage::DiskController::DiskController(QSharedPointer<Disk> d) : disk(d)
{
    QList<QSharedPointer<Surface>> allSurfaces;
    for (int i = 0; i < disk->getNPlatters(); i++)
    {
        allSurfaces.push_back(disk->getPlatter(i)->getSurface(0));
        allSurfaces.push_back(disk->getPlatter(i)->getSurface(1));
    }
    arm = Storage::ArmAssembly(allSurfaces);
}

QSharedPointer<Storage::Disk> Storage::DiskController::getDisk()
{
    return disk;
}

int Storage::DiskController::getNCylinders() const
{
    return nCylinders;
}

int Storage::DiskController::getNHeads() const
{
    return nHeads;
}

int Storage::DiskController::getNSectors() const
{
    return nSectors;
}

void Storage::DiskController::moveArmTo(int c, int s)
{
    arm.moveTo(c, s);
}

// void Storage::DiskController::displaySpace(QWidget *parent, const Space& s) const
// {
//     QString msg = "Used Size: " + QString::number(s.usedSpaceSize) + "b / " + QString::number(s.usedSpaceSize / 1e+6) + "MB\n" +
//                   "Used Disk Size: " + QString::number(s.usedDiskSpace) + "b / " + QString::number(s.usedDiskSpace / 1e+6) + "MB\n" +
//                   "Free Size: " + QString::number(s.freeSpaceSize) + "b / " + QString::number(s.freeSpaceSize / 1e+6) + "MB\n" +
//                   "Free Disk Size: " + QString::number(s.freeDiskSpace) + "b / " + QString::number(s.freeDiskSpace / 1e+6) + "MB";
//     QMessageBox msgBox(parent);
//     msgBox.setText(msg);
//     msgBox.exec();
// }

void Storage::DiskController::readBlock(int block, QByteArray& buffer)
{
    buffer.resize(blockSize);
    // Calculate logic sectors of block (call readSecLog)
    for (int i = 0; i < blockFactor; i++, block++)
    {
        readSecLog(block, buffer.data() + (i * sectorSize));
    }
}

void Storage::DiskController::readSecLog(int seclog, char* buffer)
{
    // calculate surface/head, cylinder/track, sector (LBA->CHS)
    int c = seclog / (nHeads * nSectors);
    int h = (seclog / nSectors) % nHeads;
    int s = (seclog % nSectors) + 1;
    readSector(c, h, s, buffer);
}

void Storage::DiskController::readSector(int c, int h, int s, char* buffer)
{
    // read operation
    moveArmTo(c, s);
    QString sectorPath = arm.getSector(h)->getSectorPath();
    QFile sector(sectorPath);
    if (sector.open(QIODevice::ReadOnly))
    {
        QDataStream stream(&sector);
        // Ignore header (16 bytes)
        stream.skipRawData(16);
        QByteArray data = sector.read(sectorSize);
        std::memcpy(buffer, data.constData(), sectorSize);
        sector.close();
    }
}

void Storage::DiskController::writeBlock(int block, const QByteArray& data)
{
    // Calculate logic sectors of block (call writeSecLog)
    for (int i = 0; i < blockFactor; i++, block++)
    {
        QByteArray chunk = data.mid(i * sectorSize, sectorSize);
        writeSecLog(block, chunk.constData(), chunk.size());
    }
}

void Storage::DiskController::writeSecLog(int seclog, const char* data, int dataSize)
{
    // calculate surface/head, cylinder/track, sector (LBA->CHS)
    int c = seclog / (nHeads * nSectors);
    int h = (seclog / nSectors) % nHeads;
    int s = (seclog % nSectors) + 1;
    writeSector(c, h, s, data, dataSize);
}

void Storage::DiskController::writeSector(int c, int h, int s, const char* data, int dataSize)
{
    // write operation
    moveArmTo(c, s);
    QSharedPointer<Sector> sect = arm.getSector(h);
    QString sectorPath = sect->getSectorPath();
    QFile sector(sectorPath);
    if (sector.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QDataStream stream(&sector);
        std::tuple<int, int, int> chs = sect->getChsAddress();

        stream << std::get<0>(chs) << std::get<1>(chs) << std::get<2>(chs) << dataSize;
        stream.writeRawData(data, dataSize);

        int remainingSize = sectorSize - dataSize;
        if (remainingSize > 0) {
            QByteArray padding(remainingSize, '\0');
            stream.writeRawData(padding.constData(), remainingSize);
        }
        sector.close();
    }
}

