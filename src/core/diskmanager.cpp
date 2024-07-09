#include "diskmanager.h"
#include <cmath>

qsizetype Core::AutoGrowthFactor = 2;

Core::DiskManager::DiskManager(QSharedPointer<Storage::DiskController> control, QString storageFile, bool firstInit)
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
            // 1 = free, 0 = allocated
            .blockMap = QBitArray(blocksPerCylinder, 1)
        };
        cylinderGroups.append(cg);
    }
    sib = SummaryInformationBlock
    {
        .numFileGroups = 0,
        .numFileNodes = 0,
        .fileNodeIdCounter = 1,
        .fileGroupIdCounter = 1
    };

    // Store default values at startup on disk (.bin file)
    if (firstInit) {
        QFile file(storageFile);
        if (file.open(QIODevice::WriteOnly))
        {
            QDataStream out(&file);
            out << sib;
            out << cylinderGroups;
            out << currCylinderPos;
            out << fileGroups;
            file.close();
        }
    }
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
        CylinderGroup& current = cylinderGroups[currCylinderPos];
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
    qDebug() << "The disk is full, no space for more block allocations.";
    // return invalid block address
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

Core::FileNode Core::DiskManager::allocateFileNode(int fileSize)
{
    Core::FileNode node;
    // autogrow when allocating for the first time
    int nDataBlocks = qCeil(fileSize / (float)Storage::blockSize) + AutoGrowthFactor;
    node.size = nDataBlocks * Storage::blockSize;
    QList<int> blockAddresses;
    blockAddresses.reserve(nDataBlocks);

    int blocksNeeded = nDataBlocks;
    int startCylinderPos = currCylinderPos;
    QList<int> cylindersUsed;

    while (blocksNeeded > 0)
    {
        CylinderGroup& current = cylinderGroups[currCylinderPos];
        if (current.fragmentation <= 0.95f)
        // Allocating blocks as long as the current cylinder has space left (only if fragmentation level isn't too high)
        // if they're not enough, the rest will be allocated in next cylinder
        {
            // <offset, length>
            QList<QPair<int, int>> groups = findFreeGroups(current.blockMap);
            for (const auto& group : groups)
            {
                int blocksToAllocate = std::min(blocksNeeded, group.second);

                // add to cylindersUsed if there's space to alloc.
                if (blocksToAllocate != 0) {
                    cylindersUsed.append(currCylinderPos);
                    // (some or all) data of this FileNode will be stored in this cylinder
                    current.superBlock.numFileNodes++;
                }

                for (int i = 0; i < blocksToAllocate; ++i)
                {
                    // convert relative block number to absolute (LBA number)
                    int blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + (group.first + i);
                    // add to blockArray
                    blockAddresses.append(blockAddress);
                    // update blockmap
                    current.blockMap.setBit(group.first + i, 0);
                }

                blocksNeeded -= blocksToAllocate;

                if (blocksNeeded == 0)
                    break;
            }
        }
        // move to next cylinder when:
        // - current cylinder has a high fragmentation level
        // - current cylinder has no more free space, but there are still more blocks left to allocate
        if (blocksNeeded != 0) {
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            // if no cylinder had fragmentation level lower than .95
            // went through all the cylinders of the disk
            if (currCylinderPos == startCylinderPos)
            {
                qDebug() << "FileNode not allocated. There's no space left on disk.";
                // return invalid FileNode
                return Core::FileNode();
            }
        }
    }
    // success case
    // Update fragmentation levels, only on cylinders where blocks were allocated
    for (const int i : cylindersUsed)
    {
        CylinderGroup& current = cylinderGroups[i];
        current.fragmentation = fragmentationLevel(current.blockMap);
    }
    // update global counters
    node.blocks = blockAddresses;
    node.id = sib.fileNodeIdCounter;
    sib.fileNodeIdCounter++;
    sib.numFileNodes++;

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
        int cylinderGroup = qFloor(b / (float)spc);
        cylinders.insert(cylinderGroup);
        int relativeBlockNumber = b - (cylinderGroup * spc);
        // mark is as free in the corresponding blockMap
        CylinderGroup& current = cylinderGroups[cylinderGroup];
        current.blockMap.setBit(relativeBlockNumber, 1);
    }
    // recalculate cylinder fragmentation
    for (const int c : cylinders)
    {
        CylinderGroup& current = cylinderGroups[c];
        current.fragmentation = fragmentationLevel(current.blockMap);
        // Reduce redundant entry/fileNode
        current.superBlock.numFileNodes--;
    }
    // update global counters
    sib.numFileNodes--;
    // not decrementing fileNodeIdCounter
    // reset fileNode
    node.id = -1;
    node.blocks.clear();
    node.size = 0;
}

bool Core::DiskManager::autogrowFileNode(Core::FileNode &node, int& startIndex)
{
    // allocate more constant space every time the FileNode is full/near full.
    // This technique reduces the frequency of allocations and can improve efficiency.
    QList<int> extraBlocks;
    extraBlocks.reserve(Core::AutoGrowthFactor);
    for (int i = 0; i < Core::AutoGrowthFactor; ++i)
        extraBlocks.append(this->allocateBlock());
    // in case allocation of extra space fails
    if (extraBlocks.indexOf(-1) != -1)
        return false;
    // set startIndex value
    startIndex = node.blocks.size();
    // update node properties
    node.blocks.append(extraBlocks);
    node.size += Storage::blockSize * Core::AutoGrowthFactor;
    return true;
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
    float fragment = (free - freeMax)/(float)free;
    // round to 3 decimals
    return std::round(fragment * 1000.0) / 1000.0;
}

quint64 Core::DiskManager::newFileGroup(Types::FileOrganization fo, quint64 fileSize)
{
    quint64 FileGroupLocation = 0;
    switch (fo)
    {
    case Types::FileOrganization::Heap:
    {
        // pointers or shared-pointers are not necessary, since they will be inserted in a hash data-structure,
        // accessed by iterators/const-iterators, which hold the reference to them
        HeapGroup heap;
        // allocating data blocks first
        heap.data = allocateFileNode(fileSize);
        // 9 bytes is the size of a FreeSpaceMap entry
        heap.freeSpace = allocateFileNode(9 * heap.data.blocks.size());

        // fileGroupId from Information Block
        FileGroupLocation = sib.fileGroupIdCounter;
        // update global counters
        sib.fileGroupIdCounter++;
        sib.numFileGroups++;
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
        // update global counters
        sib.fileGroupIdCounter++;
        sib.numFileGroups++;
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
        // update global counters
        sib.numFileGroups--;
    }
    else if (filegroup.value().canConvert<Core::SequentialGroup>())
    {
        SequentialGroup sequential = filegroup.value().value<Core::SequentialGroup>();
        deallocateFileNode(sequential.data);
        // update global counters
        sib.numFileGroups--;
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
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QDataStream out(&file);
        out << sib;
        out << cylinderGroups;
        out << currCylinderPos;
        out << fileGroups;
        file.close();
        return true;
    }
    return false;
}

bool Core::DiskManager::readFromDisk()
{
    QFile file(storageFile);
    if (file.open(QIODevice::ReadOnly))
    {
        // Clear current default values
        cylinderGroups.clear();
        fileGroups.clear();
        QDataStream in(&file);
        in >> sib;
        in >> cylinderGroups;
        in >> currCylinderPos;
        in >> fileGroups;
        file.close();
        return true;
    }
    return false;
}
