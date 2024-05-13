#ifndef DISK_H
#define DISK_H

#include <QList>
#include <QSharedPointer>
#include "platter.h"

namespace Storage {

    extern int sectorSize;

    extern int blockSize;

    extern int blockFactor;

    class Disk : public Utility
    {
    public:
        Disk() = default;
        ~Disk() = default;
        Disk(const QString&, int, int, int, int, int, bool);
        // void init();
        // getters
        int getNPlatters() const;
        int getNTracks() const;
        int getNSectors() const;
        int getSectorSize() const;
        int getBlockSize() const;
        int getBlockFactor() const;
        QSharedPointer<Platter> getPlatter(int);
        // theoretical-size info
        double fullDiskSize() const;
        double fullPlaterSize() const;
        double fullSurfaceSize() const;
        double fullTrackSize() const;
        // utility interface
        Space getSpace() const override;

    private:
        QString name;
        int nPlatters;
        int nTracks;
        int nSectors;
        QList<QSharedPointer<Platter>> platters;
    };

}

#endif // DISK_H
