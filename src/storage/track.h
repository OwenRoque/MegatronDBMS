#ifndef TRACK_H
#define TRACK_H

#include "sector.h"
#include <QList>
#include <QSharedPointer>

namespace Storage
{
    class Track : public Utility
    {
    public:
        Track() = default;
        ~Track() = default;
        Track(const QString&, int, quint16, quint16, quint16);
        Track(const QString&, int);
        // utility interface
        Space getSpace() const override;
        // getters
        QSharedPointer<Sector> getSector(int index);

    private:
        QList<QSharedPointer<Sector>> sectors;

    };
}

#endif // TRACK_H
