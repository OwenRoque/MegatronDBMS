#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <QDir>

class Utility
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
    void createDirectory(const QString &name)
    {
        QDir directory(name);
        if (!directory.exists())
            directory.mkpath(name);
    }
};


#endif // UTILITY_H
