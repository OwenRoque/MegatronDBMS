#include "rwhead.h"

Storage::RWHead::RWHead(QSharedPointer<Surface> s)
{
    surface = s;
}

void Storage::RWHead::setCylinder(int c)
{
    cylinder = surface->getTrack(c);
}

void Storage::RWHead::setSector(int s)
{
    // Since S starts at 1, we must decrease by 1 the search index
    sector = cylinder->getSector(s - 1);
}

QSharedPointer<Storage::Sector> Storage::RWHead::getSector()
{
    return sector;
}

