#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <diskcontroller.h>
#include "megatron_types.h"

#include <QObject>
#include <QBitArray>
#include <QByteArray>

// StorageManager will be a singleton

namespace Core
{
    class StorageManager : public QObject
    {
    Q_OBJECT
    public:
        static StorageManager& getInstance(QSharedPointer<Storage::DiskController> control = nullptr)
        {
            static StorageManager singleton(control);
            return singleton;
        }
        // Global Information about number of inodes/data blocks
        struct SummaryInformationBlock
        {
            int numInodes;
            int numDataBlocks;

            friend QDataStream& operator<<(QDataStream& out, const SummaryInformationBlock& sib) {
                out << sib.numInodes << sib.numDataBlocks;
                return out;
            }

            friend QDataStream& operator>>(QDataStream& in, SummaryInformationBlock& sib) {
                in >> sib.numInodes >> sib.numDataBlocks;
                return in;
            }
        };
        // for single block/page
        void readBlock(int, QByteArray&);
        void writeBlock(int, const QByteArray&);
        // for complete file
        // fileId = inode = <cylinder, inode>
        void readFile(std::tuple<int, int>, QByteArray&);
        void writeFile(std::tuple<int, int>, const QByteArray&);
        // storage policy: Data Organization by Cilinders
        void allocateBlock(int&);
        void deallocateBlock(int);

        void allocateFile(QList<int>& blockAddresses, int);
        void deallocateFile(int);
        QList<QPair<int, int>> findFreeGroups(const QBitArray&);
        // persistence
        bool serialize(const QString&);
        bool deserialize(const QString&);


    private:
        // Heavily inspired by UFS filesystem
        struct SuperBlock
        {
            int logicalBlockSize;
            int sectorsPerCylinder;
            int numDataBlocks;

            friend QDataStream& operator<<(QDataStream& out, const SuperBlock& superBlock) {
                out << superBlock.logicalBlockSize << superBlock.sectorsPerCylinder
                    << superBlock.numDataBlocks;
                return out;
            }
            friend QDataStream& operator>>(QDataStream& in, SuperBlock& superBlock) {
                in >> superBlock.logicalBlockSize >> superBlock.sectorsPerCylinder
                    >> superBlock.numDataBlocks;
                return in;
            }
        };

        struct Inode
        {
            QString fileName;
            int fileSize;
            Types::FileOrganization organization;
            // disk-block addresses, according to the file organization
            QList<int> pointers;
            friend QDataStream& operator<<(QDataStream& out, const Inode& inode) {
                out << inode.fileSize << inode.organization
                    << inode.pointers;
                return out;
            }
            friend QDataStream& operator>>(QDataStream& in, Inode& inode) {
                in >> inode.fileSize >> inode.organization
                    >> inode.pointers;
                return in;
            }
        };

        struct CylinderGroup
        {
            SuperBlock superBlock;
            int cylinderId;
            float fragmentation;
            QList<Inode> inodes;    // QMap<QString, Inode>
            // Free/Allocated block map (per cylinder)
            QBitArray blockMap;

            friend QDataStream& operator<<(QDataStream& out, const CylinderGroup& cg) {
                out << cg.superBlock << cg.cylinderId << cg.fragmentation << cg.inodes << cg.blockMap;
                return out;
            }
            friend QDataStream& operator>>(QDataStream& in, CylinderGroup& cg) {
                in >> cg.superBlock >> cg.cylinderId >> cg.fragmentation  >> cg.inodes >> cg.blockMap;
                return in;
            }
        };

        StorageManager(QSharedPointer<Storage::DiskController> control = nullptr);
        QSharedPointer<Storage::DiskController> controller;
        QList<CylinderGroup> cylinderGroups;
        SummaryInformationBlock sib;
        int currCylinderPos;
        Q_DISABLE_COPY(StorageManager);

    };
}

#endif // STORAGEMANAGER_H
