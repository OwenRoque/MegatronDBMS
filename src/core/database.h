#ifndef DATABASE_H
#define DATABASE_H

#include "systemcatalog.h"
#include "diskmanager.h"
#include "file.h"
#include "megatron_structs.h"

namespace Core
{
    class Database
    {
    public:
        Database() = default;
        ~Database() = default;
        Database(QSharedPointer<Storage::DiskController> dc, QString storageFile);
        Types::Return createRelation(Core::RelationInput response);

    private:
        SystemCatalog* sc;
        DiskManager* dm;
        QList<QSharedPointer<File>> relations;

    };
}

#endif // DATABASE_H
