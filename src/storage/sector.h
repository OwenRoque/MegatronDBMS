#ifndef SECTOR_H
#define SECTOR_H

#include "utility.h"
#include <QFile>
#include <QDataStream>
#include <tuple>

namespace Storage
{
    class Sector : public Utility
    {
    public:
        Sector() = default;
        ~Sector() = default;
        Sector(const QString&, int, int, int);
        Sector(const QString&);
        // utility interface
        Space getSpace() const override;
        // getters
        QString getSectorPath() const;
        std::tuple<int, int, int> getChsAddress() const;
        int getCurrentSize() const;
        // size setter
        void setCurrentSize(int);
        void print() const;

    private:
        QString sectorPath;
        std::tuple<int, int, int> chsAddress;
        int currentSize;

    };
}

#endif // SECTOR_H
