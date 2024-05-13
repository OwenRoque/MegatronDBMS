#ifndef PLATTER_H
#define PLATTER_H

#include "surface.h"

namespace Storage
{
    class Platter : public Utility
    {
    public:
        Platter() = default;
        ~Platter() = default;
        Platter(const QString&, int, int, int, int, int);
        Platter(const QString&, int, int);
        QSharedPointer<Surface> getSurface(int);
        // utility interface
        Space getSpace() const override;

    private:
        QSharedPointer<Surface> front;
        QSharedPointer<Surface> back;

    };
}
#endif // PLATTER_H
