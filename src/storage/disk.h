#ifndef DISK_H
#define DISK_H

#include <QObject>
#include <QString>
#include <QList>

// class Platter;
// class Surface;
// class Track;
// class Block;
// class Sector;

class Capacity
{
public:
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
    virtual Space getSpace() const = 0;
};

class Disk : public QObject
{
    Q_OBJECT
public:
    Disk() = default;
    Disk(const QString&, int, int, int, int, int);
    void init();
    void createDirectory(const QString&);
    // getters
    int getNPlatters() const;
    int getNTracks() const;
    int getNSectors() const;
    int getSectorSize() const;
    int getBlockSize() const;
    int getBlockFactor() const;
    // meta-size info
    double fullDiskSize() const;
    double fullPlaterSize() const;
    double fullSurfaceSize() const;
    double fullTrackSize() const;

private:
    QString name;
    int nPlatters;
    int nTracks;
    int nSectors;
    int sectorSize;
    int blockSize;
    int blockFactor;
    // QList<Platter> platters;
};


// class Platter : public Capacity
// {

// };

// class Surface : public Capacity
// {

// };

// class Track : public Capacity
// {

// };

// class Block : public Capacity
// {

// };

// class Sector : public Capacity
// {

// };

#endif // DISK_H
