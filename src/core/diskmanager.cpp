#include "diskmanager.h"
#include <cmath>

int Core::autoGrowthFactor = 2;

Core::DiskManager::DiskManager(QSharedPointer<Storage::DiskController> control, QString storageFile)
    : controller(control), storageFile(storageFile)
{
    // Optional: register metatypes for signal/slot funcionality
    qRegisterMetaType<HeapGroup>();
    qRegisterMetaType<SequentialGroup>();
    qRegisterMetaType<HashGroup>();
    qRegisterMetaType<BPlusGroup>();

    QSharedPointer<Storage::Disk> d = controller->getDisk();
    int nCylinders = controller->getNCylinders();
    cylinderGroups.reserve(nCylinders);
    int cylSize = d->getSectorSize() * d->getNSectors() * controller->getNHeads();
    int blocksPerCylinder = cylSize / d->getBlockSize();
    int sectorsPerCyl = blocksPerCylinder * d->getBlockFactor();
    for (int i = 0; i < nCylinders; i++)
    {
        SuperBlock sb
        {
            .sectorsPerCylinder = sectorsPerCyl,
            .numFileNodes = 0
        };
        CylinderGroup cg
        {
            .superBlock = sb,
            .cylinderId = i,
            .fragmentation = 0.0f,
            .fileNodes = QHash<int, FileNode>(),
            // 1 = free, 0 = allocated
            .blockMap = QBitArray(blocksPerCylinder, 1)
        };
        cylinderGroups.append(cg);
    }
    sib = SummaryInformationBlock
    {
        .numFileGroups = 0,
        .numFileNodes = 0,
        .fileNodeIdCounter = 0,
        .fileGroupIdCounter = 0
    };
}

QSharedPointer<Storage::Block> Core::DiskManager::readBlock(int blockAddress, QByteArray &buffer)
{
    controller->readBlock(blockAddress, buffer);
    return QSharedPointer<Storage::Block>(new Storage::Block(blockAddress, buffer));
}

void Core::DiskManager::writeBlock(int blockAddress, QSharedPointer<Storage::Block> block)
{
    QByteArray data;
    Storage::Block::Header h = block->getHeader();
    data.append(h.type);
    data.append(block->getData());
    controller->writeBlock(blockAddress, data);
}

// For overflow pages, new File datapages
int Core::DiskManager::allocateBlock()
{
    int blockAddress = -1;
    QPair<int, int> smallestLeftmostGroup;
    for (qsizetype i = 0; i < cylinderGroups.size(); ++i)
    {
        CylinderGroup current = cylinderGroups.at(currCylinderPos);
        int nFreeBlocks = current.blockMap.count(true);
        if (nFreeBlocks == 0)
        {
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            continue;
        }
        QList<QPair<int, int>> groups = findFreeGroups(current.blockMap);
        // choose left-most smallest group where block can fit in!!!
        QPair<int, int> smallestLeftmostGroup = groups.first();
        for (const auto& group : groups)
        {
            if (group.first < smallestLeftmostGroup.first)
                smallestLeftmostGroup = group;
            else if (group.first == smallestLeftmostGroup.first && group.second < smallestLeftmostGroup.second)
                smallestLeftmostGroup = group;
        }
        // get relative block number from cylinder
        int relativeBlockAddress = smallestLeftmostGroup.first;
        // update blockmap
        current.blockMap.setBit(relativeBlockAddress, 0);
        // convert relative block number to absolute (LBA number)
        blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + relativeBlockAddress;
        return blockAddress;
    }
    return blockAddress;
}

void Core::DiskManager::deallocateBlock(int blockAddress)
{
    // Clear block's data, set it as free
    QByteArray data;
    QSharedPointer<Storage::Block> target = this->readBlock(blockAddress, data);
    target->setHeader(Storage::Block::Header::BlockType::Free);
    target->setData(QByteArray());
    this->writeBlock(blockAddress, target);
}

// Optimized for new Relation from File (bulk data insert to disk)
Core::FileNode Core::DiskManager::allocateFileNode(int fileSize)
{
    Core::FileNode node;
    // autogrow when allocating for the first time
    int nDataBlocks = qCeil(fileSize / Storage::blockSize) + autoGrowthFactor;
    node.size = nDataBlocks * Storage::blockSize;
    QList<int> blockAddresses(nDataBlocks, -1);
    for (qsizetype i = 0; i < cylinderGroups.size(); ++i)
    {
        CylinderGroup current = cylinderGroups.at(currCylinderPos);
        int nFreeBlocks = current.blockMap.count(true);
        if (nDataBlocks > nFreeBlocks)
        {
            qDebug() << "Not possible to allocate FileNode in this cylinder";
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            continue;
        }
        if (current.fragmentation <= 0.95f)
        {
            QList<QPair<int, int>> groups = findFreeGroups(current.blockMap);
            // modify condition: choose left-most BIGGEST group where block can fit in!!!
            // assuming that after creation of Relation X user will proceed to fill
            // so when that X increases in size, blocks allocated will be contiguous
            // <offset, length>
            for (const auto& group : groups)
            {
                if (group.second >= nDataBlocks)
                {
                    for (int i = group.first; i < group.first + group.second; ++i)
                    {
                        // convert relative block number to absolute (LBA number)
                        int blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + i;
                        blockAddresses.append(blockAddress);
                        // update blockMap
                        current.blockMap.setBit(i, 0);
                    }
                    // recalculate cylinder fragmentation
                    current.fragmentation = fragmentationLevel(current.blockMap);
                    node.blocks = blockAddresses;
                    return node;
                }
            }
            // if it doesn't fit in any group, merge smaller groups into one of the desired size
            // from left to right
            int currentBlocks = 0;
            for (const auto& group : groups)
            {
                if (currentBlocks + group.second <= nDataBlocks)
                {
                    for (int i = 0; i < group.second; ++i)
                    {
                        // convert relative block number to absolute (LBA number)
                        int blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + (group.first + i);
                        blockAddresses.append(blockAddress);
                        // update blockMap
                        current.blockMap.setBit(group.first + i, 0);
                    }
                    currentBlocks += group.second;
                }
                else
                {
                    for (int i = 0; i < nDataBlocks - currentBlocks; ++i)
                    {
                        int blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + (group.first + i);
                        blockAddresses.append(blockAddress);
                        // update blockMap
                        current.blockMap.setBit(group.first + i, 0);
                    }
                    // recalculate cylinder fragmentation
                    current.fragmentation = fragmentationLevel(current.blockMap);
                    node.blocks = blockAddresses;
                    return node;
                }
            }
        }
        else
        {
            qDebug() << "Fragmentation level in cylinder " + QString::number(currCylinderPos) + " is too high.";
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            continue;
        }
    }
    qDebug() << "FileNode not allocated. There's no space left on disk.";
    node.blocks = blockAddresses;
    return node;
}

// Sets blocks of the FileNode as free in the blockMap
void Core::DiskManager::deallocateFileNode(Core::FileNode& node)
{
    // Make sure to evict this filenode's pages from buffer before calling
    // this method (done in database::dropRelation)
    QSet<int> cylinders;
    for (const int b : node.blocks)
    {
        deallocateBlock(b);
        // convert blockAddress to relative, calculate the cylinder where the block resides
        // some blocks might not be located in the cylinderGroup defined in the FileNode:
        // - After a node growth, block allocation
        int spc = cylinderGroups.at(currCylinderPos).superBlock.sectorsPerCylinder;
        int cylinderGroup = qFloor(b/spc);
        cylinders.insert(cylinderGroup);
        int relativeBlockNumber = b - (cylinderGroup * spc);
        // mark is as free in the corresponding blockMap
        auto current = cylinderGroups.at(cylinderGroup);
        current.blockMap.setBit(relativeBlockNumber, 1);
    }
    // recalculate cylinder fragmentation
    for (const int c : cylinders)
    {
        auto current = cylinderGroups.at(c);
        current.fragmentation = fragmentationLevel(current.blockMap);
    }
    // reset Filenode (fileGroup will delete it anyway)
    node.id = -1;
    node.cylinderGroup = -1;
    node.blocks.clear();
    node.size = 0;
}

float Core::DiskManager::fragmentationLevel(const QBitArray& bm)
{
    int free = bm.count(true);
    QList<QPair<int, int>> groups = findFreeGroups(bm);
    // calculate size of largest free block
    int freeMax = 0;
    for (const auto& g : groups)
        if (g.second > freeMax)
            freeMax = g.second;
    // fragmentation formula:
    // free - freemax / free
    // where: free     = total number of free blocks
    //        freemax  = size of largest free block
    float fragment = (free - freeMax)/free;
    // round to 3 decimals
    return std::round(fragment * 1000.0) / 1000.0;
}

int Core::DiskManager::newFileGroup(Types::FileOrganization fo, int fileSize)
{
    int FileGroupLocation = -1;
    switch (fo)
    {
    case Types::FileOrganization::Heap:
    {
        HeapGroup heap;
        // allocating data blocks first
        heap.data = allocateFileNode(fileSize);
        // 5 bytes is the size of a FreeSpaceMap entry
        heap.freeSpace = allocateFileNode(5 * heap.data.blocks.size());

        // fileGroupId from Information Block
        FileGroupLocation = sib.fileGroupIdCounter;
        // update global counter
        sib.fileGroupIdCounter++;
        // insert FileGroup
        fileGroups.insert(FileGroupLocation, QVariant::fromValue(heap));
        break;
    }
    case Types::FileOrganization::Sequential:
    {
        SequentialGroup sequential;
        // allocating data blocks first
        sequential.data = allocateFileNode(fileSize);

        // fileGroupId
        FileGroupLocation = sib.fileGroupIdCounter;
        // update global counter
        sib.fileGroupIdCounter++;
        // insert FileGroup
        fileGroups.insert(FileGroupLocation, QVariant::fromValue(sequential));
        // 5 bytes is the size of a RID
        // store in indexes
        // node.header = allocateFile(5 * datablocks.size(), false);
        break;
    }
    case Types::FileOrganization::Hash:
        break;
    case Types::FileOrganization::BPlusTree:
        break;
    }
    return FileGroupLocation;
}

bool Core::DiskManager::deleteFileGroup(int fileGroupId)
{
    auto filegroup = fileGroups.find(fileGroupId);
    if (filegroup.value().canConvert<Core::HeapGroup>())
    {
        HeapGroup heap = filegroup.value().value<Core::HeapGroup>();
        deallocateFileNode(heap.freeSpace);
        deallocateFileNode(heap.data);
    }
    else if (filegroup.value().canConvert<Core::SequentialGroup>())
    {
        SequentialGroup sequential = filegroup.value().value<Core::SequentialGroup>();
        deallocateFileNode(sequential.data);
    }
    else if (filegroup.value().canConvert<Core::HashGroup>())
    {
        // TODO
        ;
    }
    else if (filegroup.value().canConvert<Core::BPlusGroup>())
    {
        // TODO
        ;
    }
    else
    {
        qDebug() << "Unknown FileGroup";
        return false;
    }
    return fileGroups.remove(fileGroupId);
}

QVariant Core::DiskManager::locateFileGroup(int fileGroupId)
{
    auto fileGroup = fileGroups.find(fileGroupId);
    if (fileGroup != fileGroups.end()) {
        return QVariant::fromValue(fileGroup.value());
    }
    return QVariant();
}

QList<QPair<int, int>> Core::DiskManager::findFreeGroups(const QBitArray &bm)
{
    QList<QPair<int, int>> groups;
    int length = bm.size();
    int start = -1;
    for (qsizetype i = 0; i < length; ++i)
    {
        if (bm.testBit(i))
        {
            if (start == -1)
                start = i;
        }
        else
        {
            if (start != -1)
            {
                groups.append({start, i - start});
                start = -1;
            }
        }
    }
    if (start != -1) {
        groups.append({start, length - start});
    }
    return groups;
}

bool Core::DiskManager::saveToDisk()
{
    QFile file(storageFile);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    out << sib;
    out << cylinderGroups;
    out << currCylinderPos;
    out << fileGroups;
    file.close();
    return true;
}

bool Core::DiskManager::readFromDisk()
{
    QFile file(storageFile);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    // Clear current default values
    cylinderGroups.clear();
    QDataStream in(&file);
    in >> sib;
    in >> cylinderGroups;
    in >> currCylinderPos;
    in >> fileGroups;
    file.close();
    return true;
}
