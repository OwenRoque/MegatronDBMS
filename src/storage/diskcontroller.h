#ifndef DISKCONTROLLER_H
#define DISKCONTROLLER_H

#include "disk.h"
#include <QObject>
#include <QSharedPointer>

class DiskController : public QObject
{
    Q_OBJECT
public:
    DiskController(QSharedPointer<Disk> d = nullptr);

    struct Space
    {
        Space() {
            usedSpaceSize = 0;
            usedDiskSpace = 0;
            freeSpaceSize = 0;
            freeDiskSpace = 0;
        }
        unsigned long int usedSpaceSize; // Actual space used
        unsigned long int usedDiskSpace; // Total size used on disk
        unsigned long int freeSpaceSize; // Actual free space (non-used)
        unsigned long int freeDiskSpace; // Total free space on disk
    };

    void displaySpace(QWidget*, const Space&) const;
    // space available methods
    // disk
    Space getSpaceOf();
    // block
    Space getSpaceOf(int nBlock);
    // sectors
    Space getSpaceOf(int nPlatter, int nSurface, int nTrack, int nSector);
    // Read/Write
    void readBlock();
    void readSecLog();
    void readSector();
    void writeBlock();
    void writeSecLog();
    void writeSector();

private:
    QSharedPointer<Disk> disk;

};

#endif // DISKCONTROLLER_H
