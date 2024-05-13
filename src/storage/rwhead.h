#ifndef RWHEAD_H
#define RWHEAD_H

#include "surface.h"
#include <QSharedPointer>

namespace Storage
{
    class RWHead
    {
    public:
        RWHead() = default;
        ~RWHead() = default;
        RWHead(QSharedPointer<Surface>);
        void setCylinder(int);
        void setSector(int);
        QSharedPointer<Sector> getSector();

    private:
        QSharedPointer<Surface> surface;
        QSharedPointer<Track> cylinder;
        QSharedPointer<Sector> sector;
    };
}

#endif // RWHEAD_H
