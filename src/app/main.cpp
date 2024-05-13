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
    QSharedPointer<Storage::Disk> disk;
    // find disk
    if (listFiles.isEmpty())
    {
        DiskInit dialog;
        if (dialog.exec() != QDialog::Accepted)
            return 0;
        disk = QSharedPointer<Storage::Disk>(new Storage::Disk(disksPath + "/" + dialog.name, dialog.nPlatters, dialog.nTracks,
                                             dialog.nSectors, dialog.sectorSize, dialog.blockSize, true));
    }
    else
    {
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
            disk = QSharedPointer<Storage::Disk>(new Storage::Disk(disksPath + "/" + name, nPlatters, nTracks, nSectors, sSize, bSize, false));
        }
        else
            return 0;
    }
    QSharedPointer<Storage::DiskController> controller(new Storage::DiskController(disk));
    Megatron w(nullptr, controller);
    w.show();
    return a.exec();
}
