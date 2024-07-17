#ifndef HEAPFILE_H
#define HEAPFILE_H

#include <QFile>
#include <QQueue>
#include "diskmanager.h"
#include "systemcatalog.h"
#include "file.h"

namespace Core
{
    class HeapFile : public File
    {
    public:
        HeapFile(const QString& relationName, bool firstInit = false);
        Types::Return insertRecord() override;
        Types::Return bulkInsertRecords(const QString&) override    ;
        Types::Return deleteRecord() override;
        bool autogrow() override;

    private:
        // our free space map file with be accessed through it's respective HeapGroup
        QSharedPointer<FreeSpaceMap> getFreeSpaceMap() {
            Core::DiskManager* dm = &Core::DiskManager::getInstance();
            Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
            // get relation metadata
            auto relation = sc->constFindRelation(this->relationName);
            // access to the fileGroup through its location
            QVariant fileGroupVariant = dm->locateFileGroup(relation->location);
            if (!fileGroupVariant.isValid() || !fileGroupVariant.canConvert<HeapGroup>()) {
                return nullptr;
            }
            auto fileGroup = fileGroupVariant.value<Core::HeapGroup>();
            return fileGroup.freeSpace;
        }
    };

}

#endif // HEAPFILE_H
