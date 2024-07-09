#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <diskcontroller.h>
#include <block.h>
#include "megatron_types.h"

#include <QObject>
#include <QBitArray>
#include <QByteArray>
#include <QSharedPointer>
#include <QVariant>
#include <QtMath>

// DiskManager will be a singleton

namespace Core
{
    // number of blocks to be added to a fileNode
    // every time it's full/near full
    extern qsizetype AutoGrowthFactor;

    // Global Information about number of file-nodes/groups,
    // it also records changes that take place as the filesystem is used
    struct SummaryInformationBlock
    {
        // total n° of fileGroups in the disk
        int numFileGroups;
        // total n° of fileNodes in the disk
        int numFileNodes;
        int fileNodeIdCounter;
        int fileGroupIdCounter;

        friend QDataStream& operator<<(QDataStream& out, const SummaryInformationBlock& sib) {
            out << sib.numFileGroups << sib.numFileNodes
                << sib.fileNodeIdCounter << sib.fileGroupIdCounter;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, SummaryInformationBlock& sib) {
            in >> sib.numFileGroups >> sib.numFileNodes
                >> sib.fileNodeIdCounter >> sib.fileGroupIdCounter;
            return in;
        }
    };

    // Stores information about the filesystem, metadata about the cylinder where is included
    struct SuperBlock
    {
        // each cylinder has the same number of sectors
        int sectorsPerCylinder;
        // number of fileNodes in this cylinder, redundant value
        // (a fileNode's blocks can be scattered across multiple cylinders)
        int numFileNodes;

        friend QDataStream& operator<<(QDataStream& out, const SuperBlock& superBlock) {
            out << superBlock.sectorsPerCylinder << superBlock.numFileNodes;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, SuperBlock& superBlock) {
            in >> superBlock.sectorsPerCylinder >> superBlock.numFileNodes;
            return in;
        }
    };

    struct FileNode;
    struct CylinderGroup;

    class DiskManager : public QObject
    {
        Q_OBJECT
    public:
        static DiskManager& getInstance(QSharedPointer<Storage::DiskController> control = nullptr,
                                           QString storageFile = QString(), bool firstInit = false)
        {
            static DiskManager singleton(control, storageFile, firstInit);
            return singleton;
        }

        QSharedPointer<Storage::Block> readBlock(int, QByteArray&);
        void writeBlock(int, QSharedPointer<Storage::Block>);

        // storage policy: Data Organization by Cylinders
        int allocateBlock();
        void deallocateBlock(int);
        Core::FileNode allocateFileNode(int fileSize = Storage::blockSize);
        void deallocateFileNode(Core::FileNode&);
        bool autogrowFileNode(Core::FileNode&, int&);
        float fragmentationLevel(const QBitArray&);

        quint64 newFileGroup(Types::FileOrganization fo, quint64 fileSize = Storage::blockSize);
        bool deleteFileGroup(int);
        QVariant locateFileGroup(int);
        QList<QPair<int, int>> findFreeGroups(const QBitArray&);
        // persistence
        bool saveToDisk();
        bool readFromDisk();

    private:
        DiskManager(QSharedPointer<Storage::DiskController> control = nullptr,
                       QString storageFile = QString(), bool firstInit = false);
        QSharedPointer<Storage::DiskController> controller;

        SummaryInformationBlock sib;
        QList<CylinderGroup> cylinderGroups;
        int currCylinderPos;
        QHash<int, QVariant> fileGroups;
        QString storageFile;

        Q_DISABLE_COPY(DiskManager);

    };

    // Structs used in DiskManager

    struct FileNode
    {
        FileNode() { id = -1; size = 0; };

        // fileNode ID
        int id;
        // size in blocks
        int size;
        // disk-block addresses
        QList<int> blocks;
        // since a fileNode can be fragmented across multiple cylinders
        // not implemented at the moment, it can be calculated but it's costly
        // QList<int> cylinders;

        friend QDataStream& operator<<(QDataStream& out, const FileNode& node) {
            out << node.id << node.size << node.blocks;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, FileNode& node) {
            in >> node.id >> node.size >> node.blocks;
            return in;
        }
    };

    struct CylinderGroup
    {
        SuperBlock superBlock;
        int cylinderId;
        float fragmentation;
        // Free/Allocated block map (per cylinder)
        QBitArray blockMap;

        friend QDataStream& operator<<(QDataStream& out, const CylinderGroup& cg) {
            out << cg.superBlock << cg.cylinderId << cg.fragmentation << cg.blockMap;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, CylinderGroup& cg) {
            in >> cg.superBlock >> cg.cylinderId >> cg.fragmentation >> cg.blockMap;
            return in;
        }
    };

    // FileGroups

    class HeapGroup
    {
    public:
        HeapGroup() = default;
        ~HeapGroup() = default;

        Types::FileOrganization type = Types::FileOrganization::Heap;
        FileNode freeSpace;
        FileNode data;
        QList<FileNode> indexes;

        friend QDataStream& operator<<(QDataStream& out, const HeapGroup& group) {
            out << group.type << group.freeSpace << group.data << group.indexes;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, HeapGroup& group) {
            in >> group.type >> group.freeSpace >> group.data >> group.indexes;
            return in;
        }

    };

    class SequentialGroup
    {
    public:
        SequentialGroup() = default;
        ~SequentialGroup() = default;

        Types::FileOrganization type = Types::FileOrganization::Sequential;
        FileNode data;
        // QList<FileNode> indexes;

        friend QDataStream& operator<<(QDataStream& out, const SequentialGroup& group) {
            out << group.type << group.data;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, SequentialGroup& group) {
            in >> group.type >> group.data;
            return in;
        }

    };

    class HashGroup
    {
    public:
        HashGroup() = default;
        ~HashGroup() = default;

        Types::FileOrganization type = Types::FileOrganization::Hash;
        FileNode data;
        // QList<FileNode> indexes;

        friend QDataStream& operator<<(QDataStream& out, const HashGroup& group) {
            out << group.type << group.data;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, HashGroup& group) {
            in >> group.type >> group.data;
            return in;
        }

    };

    class BPlusGroup
    {
    public:
        BPlusGroup() = default;
        ~BPlusGroup() = default;

        Types::FileOrganization type = Types::FileOrganization::BPlusTree;
        FileNode data;
        // QList<FileNode> indexes;

        friend QDataStream& operator<<(QDataStream& out, const BPlusGroup& group) {
            out << group.type << group.data;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, BPlusGroup& group) {
            in >> group.type >> group.data;
            return in;
        }

    };

}

// Register metatypes for QVariant stream operators
Q_DECLARE_METATYPE(Core::HeapGroup)
Q_DECLARE_METATYPE(Core::SequentialGroup)
Q_DECLARE_METATYPE(Core::HashGroup)
Q_DECLARE_METATYPE(Core::BPlusGroup)

#endif // DISKMANAGER_H
