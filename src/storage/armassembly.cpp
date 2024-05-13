#include "armassembly.h"

Storage::ArmAssembly::ArmAssembly(QList<QSharedPointer<Surface>> allSurfaces)
{
    heads.reserve(allSurfaces.size());
    for (size_t i = 0; i < allSurfaces.size(); i++)
        heads.emplace_back(new RWHead(allSurfaces.at(i)));
}

void Storage::ArmAssembly::moveTo(int cylinder, int sector)
{
    for (auto rwh : heads)
    {
        rwh->setCylinder(cylinder);
        rwh->setSector(sector);
    }
}

QSharedPointer<Storage::Sector> Storage::ArmAssembly::getSector(int head)
{
    return heads.at(head)->getSector();
}
