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
        Sector(const QString&, quint16, quint16, quint16);
        Sector(const QString&);
        // utility interface
        Space getSpace() const override;
        // getters
        QString getSectorPath() const;
        std::tuple<quint16, quint16, quint16> getChsAddress() const;
        quint16 getCurrentSize() const;
        // size setter
        void setCurrentSize(quint16);
        // system size
        qint64 size() const;
        void print() const;

    private:
        QString sectorPath;
        std::tuple<quint16, quint16, quint16> chsAddress;
        quint16 currentSize;

    };
}

#endif // SECTOR_H
