#ifndef ARMASSEMBLY_H
#define ARMASSEMBLY_H

#include "rwhead.h"

namespace Storage
{
    class ArmAssembly
    {
    public:
        ArmAssembly() = default;
        ~ArmAssembly() = default;
        ArmAssembly(QList<QSharedPointer<Surface>> allSurfaces);
        void moveTo(int cylinder, int sector);
        QSharedPointer<Sector> getSector(int head);

    private:
        QList<QSharedPointer<RWHead>> heads;
    };
}

#endif // ARMASSEMBLY_H
