#include "megatron.h"
#include <diskinit.h>
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
        DiskInit dialog;
        if (dialog.exec() != QDialog::Accepted)
            return 0;
        diskPath = disksPath + "/" + dialog.name;
        disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, dialog.nPlatters, dialog.nTracks,
                                             dialog.nSectors, dialog.sectorSize, dialog.blockSize, firstInit));
    }
    // or choose a disk (multiple disks can be created, each one with different data)
    else
    {
        firstInit = false;
        QStringList disks;
        for (const QFileInfo& info : listFiles) disks.append(info.fileName());
        bool ok;
        QString name = QInputDialog::getItem(nullptr, "Load Disk", "Disks available:", disks, 0, false, &ok);
        if (ok && !name.isEmpty())
        {
            QFile configFile(disksPath + "/" + name + "/" + "disk.config");
            configFile.open(QIODevice::ReadOnly | QIODevice::Text);
            int nPlatters, nTracks, nSectors, sSize, bSize;
            QTextStream in(&configFile);
            in >> nPlatters >> nTracks >> nSectors >> sSize >> bSize;
            diskPath = disksPath + "/" + name;
            disk = QSharedPointer<Storage::Disk>(new Storage::Disk(diskPath, nPlatters, nTracks, nSectors, sSize, bSize, firstInit));
        }
        else
            return 0;
    }
    QSharedPointer<Storage::DiskController> controller(new Storage::DiskController(disk));

    // init buffer pool here

    Megatron w(nullptr, diskPath, controller, firstInit);
    w.show();
    return a.exec();
}
