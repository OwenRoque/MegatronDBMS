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
        Platter(const QString&, int, int, quint16, quint16, quint16);
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
