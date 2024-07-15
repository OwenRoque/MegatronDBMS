#ifndef DATABASE_H
#define DATABASE_H

#include "systemcatalog.h"
#include "diskmanager.h"
#include <buffermanager.h>
#include "file.h"
#include "megatron_structs.h"

namespace Core
{
    class Database
    {
    public:
        Database() = default;
        ~Database() = default;
        Database(QSharedPointer<Storage::DiskController> dc, const QString& storagePath,
                 const QString& catalogPath, const QString& replacerPolicy, int bufferSize, bool firstInit);
        Types::Return createRelation(Core::RelationInput response);

    private:
        SystemCatalog* sc;
        DiskManager* dm;
        Memory::BufferManager* bm;
        QList<QSharedPointer<File>> relations;

    };
}

#endif // DATABASE_H
