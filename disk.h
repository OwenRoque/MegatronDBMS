#ifndef DISK_H
#define DISK_H

#include <QObject>
#include <QString>

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

};

#endif // DISK_H
