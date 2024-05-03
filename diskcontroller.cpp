#include "diskcontroller.h"

#include <QTextStream>
#include <QMessageBox>

DiskController::DiskController(QSharedPointer<Disk> d) : disk(d) {
    // qDebug() << disk->fullDiskSize();
}

void DiskController::displaySpace(QWidget *parent, Space& s) const
{
    QString msg = "Used Size: " + QString::number(s.usedSpaceSize) + "b / " + QString::number(s.usedSpaceSize / 1e+6) + "MB\n" +
                  "Used Disk Size: " + QString::number(s.usedDiskSpace) + "b / " + QString::number(s.usedDiskSpace / 1e+6) + "MB\n" +
                  "Free Size: " + QString::number(s.freeSpaceSize) + "b / " + QString::number(s.freeSpaceSize / 1e+6) + "MB\n" +
                  "Free Disk Size: " + QString::number(s.freeDiskSpace) + "b / " + QString::number(s.freeDiskSpace / 1e+6) + "MB";
    QMessageBox msgBox(parent);
    msgBox.setText(msg);
    msgBox.exec();
}

DiskController::Space DiskController::getSpaceOf()
{
    Space s;
    return s;
}

DiskController::Space DiskController::getSpaceOf(int nPlatter, int nSurface, int nTrack, int nSector)
{
    Space s;
    return s;
}

DiskController::Space DiskController::getSpaceOf(char object, int n)
{
    Space s;
    return s;
}


DiskController::Space DiskController::getSpaceOf(int nPlatter, int nSurface)
{
    Space s;
    return s;
}

void DiskController::readBlock()
{
    qDebug() << "test read block";
    // Calculate logic sectors of block (call readSecLog)
}

void DiskController::readSecLog()
{
    // calculate surface/head, cylinder/track, sector (LBA->CHS)
}

void DiskController::readSector()
{
    // read operation
}

void DiskController::writeBlock()
{
    // Calculate logic sectors of block (call writeSecLog)
}

void DiskController::writeSecLog()
{
    // calculate surface/head, cylinder/track, sector (LBA->CHS)
}

void DiskController::writeSector()
{
    // write operation
}

