#ifndef DISKCONTROLLER_H
#define DISKCONTROLLER_H

#include "disk.h"
#include "armassembly.h"
#include <QSharedPointer>

namespace Storage
{
    class DiskController
    {
    public:
        DiskController(QSharedPointer<Storage::Disk> d = nullptr);
        // getters
        QSharedPointer<Storage::Disk> getDisk();
        int getNCylinders() const;
        int getNHeads() const;
        int getNSectors() const;
        void moveArmTo(int, int);
        // Read/Write low-level methods
        void readBlock(int, QByteArray&);
        void readSecLog(int, char*);
        void readSector(int, int, int, char*);
        void writeBlock(int, const QByteArray&);
        void writeSecLog(int, const char*, int);
        void writeSector(int, int, int, const char*, int);
        // void displaySpace(QWidget*, const Space&) const;

    private:
        QSharedPointer<Storage::Disk> disk;
        int nCylinders;
        int nHeads;
        int nSectors;
        ArmAssembly arm;

    };
}

#endif // DISKCONTROLLER_H
