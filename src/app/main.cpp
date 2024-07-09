#include "megatron.h"
#include <diskinit.h>
#include <diskload.h>
#include <diskcontroller.h>

#include <QApplication>
#include <QInputDialog>
#include <QStringList>
#include <QSharedPointer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // const path to disks' directory
    const QString disksPath(QCoreApplication::applicationDirPath() + "/disks");
    QDir dir(disksPath);

    if (!dir.exists())
        dir.mkpath(disksPath);

    QFileInfoList listFiles = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    QString diskPath;
    QSharedPointer<Storage::Disk> disk;
    // find disk
    // If there's no disk previously created, create a new one
    bool firstInit = true;
    if (listFiles.isEmpty())
    {
        DiskInit initDialog;
        if (initDialog.exec() != QDialog::Accepted)
            return 0;
        diskPath = disksPath + "/" + initDialog.name;
        disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, initDialog.nPlatters, initDialog.nTracks,
                                             initDialog.nSectors, initDialog.sectorSize, initDialog.blockSize, firstInit));
    }
    // or choose a disk (multiple disks can be created, each one with different data)
    else
    {
        firstInit = false;
        QStringList disks;
        for (const QFileInfo& info : listFiles) disks.append(info.fileName());
        DiskLoad loadDialog(disks);
        // QObject::connect(&loadDialog, &DiskLoad::newDiskRequested, &loadDialog, [&]() {
        //     // exit code for this case is rejected
        //     firstInit = true;
        //     DiskInit initDialog;
        //     if (initDialog.exec() == QDialog::Accepted) {
        //         diskPath = disksPath + "/" + initDialog.name;
        //         disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, initDialog.nPlatters, initDialog.nTracks, initDialog.nSectors,
        //                                                                initDialog.sectorSize, initDialog.blockSize, firstInit));
        //     }
        // });
        // if (/*ok && */!name.isEmpty())
        auto ret = loadDialog.exec();
        if (ret == QDialog::Accepted)
        {
            QString name = loadDialog.getSelectedDisk();
            QFile configFile(disksPath + "/" + name + "/" + "disk.config");
            configFile.open(QIODevice::ReadOnly | QIODevice::Text);
            int nPlatters, nTracks, nSectors, sSize, bSize;
            QTextStream in(&configFile);
            in >> nPlatters >> nTracks >> nSectors >> sSize >> bSize;
            diskPath = disksPath + "/" + name;
            disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, nPlatters, nTracks, nSectors, sSize, bSize, firstInit));
        }
        else if (ret == QDialog::Rejected)
            return 0;
        else if (ret == 2)
        {
            firstInit = true;
            DiskInit initDialog;
            if (initDialog.exec() != QDialog::Accepted) {
                return 0;
            }
            // Asignar parámetros del nuevo disco
            diskPath = disksPath + "/" + initDialog.name;
            disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, initDialog.nPlatters, initDialog.nTracks, initDialog.nSectors,
                                                                   initDialog.sectorSize, initDialog.blockSize, firstInit));
        }
    }
    QSharedPointer<Storage::DiskController> controller(new Storage::DiskController(disk));

    // init buffer pool here

    Megatron w(nullptr, diskPath, controller, firstInit);
    w.show();
    return a.exec();
}
