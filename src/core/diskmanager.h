#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <diskcontroller.h>
#include <block.h>
#include "megatron_types.h"

#include <QObject>
#include <QBitArray>
#include <QByteArray>
#include <QPointer>
#include <QVariant>
#include <QtMath>

// DiskManager will be a singleton

namespace Core
{
    // number of blocks to be added to a fileNode
    // every time it's full/near full
    extern int autoGrowthFactor;

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
        // number of fileNodes in this cylinder
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
                                           QString storageFile = QString())
        {
            static DiskManager singleton(control, storageFile);
            return singleton;
        }

        QSharedPointer<Storage::Block> readBlock(int, QByteArray&);
        void writeBlock(int, QSharedPointer<Storage::Block>);

        // fileId = node = <cylinder, inode>
        // void readFile(std::tuple<int, int>, QByteArray&);
        // void writeFile(std::tuple<int, int>, const QByteArray&);
        // storage policy: Data Organization by Cylinders
        int allocateBlock();
        void deallocateBlock(int);
        Core::FileNode allocateFileNode(int fileSize = Storage::blockSize);
        void deallocateFileNode(Core::FileNode&);
        float fragmentationLevel(const QBitArray&);

        int newFileGroup(Types::FileOrganization fo, int fileSize = Storage::blockSize);
        bool deleteFileGroup(int);
        QVariant locateFileGroup(int);
        QList<QPair<int, int>> findFreeGroups(const QBitArray&);
        // persistence
        bool saveToDisk();
        bool readFromDisk();

    private:
        DiskManager(QSharedPointer<Storage::DiskController> control = nullptr,
                       QString storageFile = QString());
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
        FileNode() = default;
        // allocate more constant space every time the FileNode is full/near full.
        // This technique reduces the frequency of allocations and can improve efficiency.
        bool autogrow()
        {
            QList<int> extraBlocks(Core::autoGrowthFactor, -1);
            Core::DiskManager *dm = &Core::DiskManager::getInstance();
            for (int i = 0; i < Core::autoGrowthFactor; ++i)
                extraBlocks.append(dm->allocateBlock());
            if (extraBlocks.first() == -1)
                return false;
            blocks.append(extraBlocks);
            return true;
        }

        // fileNode ID
        int id;
        // size in blocks
        int size;
        // disk-block addresses
        QList<int> blocks;
        // location
        int cylinderGroup;

        friend QDataStream& operator<<(QDataStream& out, const FileNode& node) {
            out << node.size << node.blocks;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, FileNode& node) {
            in >> node.size >> node.blocks;
            return in;
        }
    };

    struct CylinderGroup
    {
        SuperBlock superBlock;
        int cylinderId;
        float fragmentation;
        QHash<int, FileNode> fileNodes;    // QList<FileNode> / QMap<int, FileNode>
        // Free/Allocated block map (per cylinder)
        QBitArray blockMap;

        friend QDataStream& operator<<(QDataStream& out, const CylinderGroup& cg) {
            out << cg.superBlock << cg.cylinderId << cg.fragmentation << cg.fileNodes << cg.blockMap;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, CylinderGroup& cg) {
            in >> cg.superBlock >> cg.cylinderId >> cg.fragmentation >> cg.fileNodes >> cg.blockMap;
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
        // QList<FileNode> indexes;

        friend QDataStream& operator<<(QDataStream& out, const HeapGroup& group) {
            out << group.type << group.freeSpace << group.data;
            return out;
        }

        friend QDataStream& operator>>(QDataStream& in, HeapGroup& group) {
            in >> group.type >> group.freeSpace >> group.data;
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
