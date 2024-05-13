#ifndef SURFACE_H
#define SURFACE_H

#include "track.h"

namespace Storage
{
    class Surface : public Utility
    {
    public:
        Surface() = default;
        ~Surface() = default;
        Surface(const QString&, int, int, int, int, int);
        Surface(const QString&, int, int);
        QSharedPointer<Track> getTrack(int);
        // utility interface
        Space getSpace() const override;

    private:
        QList<QSharedPointer<Track>> tracks;

    };
}
#endif // SURFACE_H
