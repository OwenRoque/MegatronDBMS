#include "storagemanager.h"

Core::StorageManager::StorageManager(QSharedPointer<Storage::DiskController> control)
    : controller(control)
{
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
            .logicalBlockSize = controller->getDisk()->getBlockSize(),
            .sectorsPerCylinder = sectorsPerCyl,
            .numDataBlocks = 0
        };
        CylinderGroup cg
        {
            .superBlock = sb,
            .cylinderId = i,
            .inodes = QList<Inode>(),
            // 1 = free, 0 = allocated
            .blockMap = QBitArray(blocksPerCylinder, 1)
        };
        cylinderGroups.append(cg);
    }
    sib = SummaryInformationBlock { .numInodes = 0, .numDataBlocks = 0 };
}

void Core::StorageManager::readBlock(int blockAddress, QByteArray &buffer)
{
    controller->readBlock(blockAddress, buffer);
}

void Core::StorageManager::writeBlock(int blockAddress, const QByteArray &data)
{
    controller->writeBlock(blockAddress, data);
}

// Use this method for pre-existent Files
void Core::StorageManager::readFile(std::tuple<int, int> index, QByteArray &data)
{
    QList<int> filePointers = cylinderGroups.at(std::get<0>(index)).inodes.at(std::get<1>(index)).pointers;
    // Disk sheduler can be implementated here to optimize disk I/O requests
    for (int p : filePointers)
    {
        controller->readBlock(p, data);
    }
}

// Use this method for pre-existent Files
void Core::StorageManager::writeFile(std::tuple<int, int> index, const QByteArray &data)
{
    QList<int> filePointers = cylinderGroups.at(std::get<0>(index)).inodes.at(std::get<1>(index)).pointers;
    // Disk sheduler can be implementated here to optimize disk I/O requests
    for (int p : filePointers)
    {
        controller->writeBlock(p, data);
    }
}

// For overflow pages, new File datapages
void Core::StorageManager::allocateBlock(int& blockAddress)
{
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
        // convert First position from Pair to blockId (LBA number)
        int relativeBlockAddress = smallestLeftmostGroup.first;
        blockAddress = currCylinderPos * (current.superBlock.sectorsPerCylinder) + relativeBlockAddress;
        break;
    }
}

void Core::StorageManager::deallocateBlock(int blockAddress)
{

}

// define method to create inode for NEW file and allocate data blocks (pointers)



// Optimized for new Relation from File (bulk data insert to disk)
void Core::StorageManager::allocateFile(QList<int>& blockAddresses, int fileSize)
{
    int nFileBlocks = qCeil(fileSize / Storage::blockSize);
    for (qsizetype i = 0; i < cylinderGroups.size(); ++i)
    {
        CylinderGroup current = cylinderGroups.at(currCylinderPos);
        int nFreeBlocks = current.blockMap.count(true);
        if (nFileBlocks > nFreeBlocks)
        {
            qDebug() << "Not possible to allocate File in this cylinder";
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            continue;
        }
        if (current.fragmentation <= 0.95f)
        {
            QList<QPair<int, int>> groups = findFreeGroups(current.blockMap);
            // modify condition: choose BIGGEST group where block can fit in!!!
            // assuming that after creation of Relation X user will proceed to fill
            // so when that X increases in size, blocks allocated will be contiguous
            for (const auto& group : groups)
            {
                if (group.second >= nFileBlocks)
                {
                    for (int i = group.first; i < group.first + group.second; ++i)
                        blockAddresses.append(i);
                    return;
                }
            }
            // if it doesn't fit, merge smaller groups into one of the desired size
            int currentBlocks = 0;
            for (const auto& group : groups)
            {
                if (currentBlocks + group.second <= nFileBlocks)
                {
                    for (int i = 0; i < group.second; ++i)
                        blockAddresses.append(group.first + i);
                    currentBlocks += group.second;
                }
                else
                {
                    for (int i = 0; i < nFileBlocks - currentBlocks; ++i)
                        blockAddresses.append(group.first + i);
                    return;
                }

            }
        }
        else
        {
            qDebug() << "Fragmentation level in this cylinder is too high.";
            currCylinderPos = (currCylinderPos + 1) % cylinderGroups.size();
            continue;
        }
    }
    qDebug() << "File not inserted :(";
}

void Core::StorageManager::deallocateFile(int blockAddress)
{

}

QList<QPair<int, int> > Core::StorageManager::findFreeGroups(const QBitArray &bm)
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

bool Core::StorageManager::serialize(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    out << sib;
    out << cylinderGroups;
    file.close();
    return true;
}

bool Core::StorageManager::deserialize(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    // Clear current default values
    cylinderGroups.clear();
    QDataStream in(&file);
    in >> sib;
    in >> cylinderGroups;
    file.close();
    return true;
}


