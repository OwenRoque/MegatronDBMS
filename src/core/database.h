#ifndef DATABASE_H
#define DATABASE_H

#include <QQueue>
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
        Database(QSharedPointer<Storage::DiskController> dc, QString storageFile, bool firstInit);
        Types::Return createRelation(Core::RelationInput response);

    private:
        SystemCatalog* sc;
        DiskManager* dm;
        QList<QSharedPointer<File>> relations;

    };
}

#endif // DATABASE_H
