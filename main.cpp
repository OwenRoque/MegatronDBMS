#include "megatron.h"
#include "diskinit.h"
#include "disk.h"
#include "diskcontroller.h"

#include <QApplication>
#include <QInputDialog>
#include <QStringList>
#include <QSharedPointer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString disksFile("E:/UNSA/2024A/BASE DE DATOS II/megatron/disks/");
    QDir dir(disksFile);
    if (!dir.exists())
        dir.mkpath(disksFile);
    QFileInfoList listFiles = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    QSharedPointer<Disk> disk;
    // Find disk
    if (listFiles.isEmpty())
    {
        DiskInit dialog;
        if (dialog.exec() != QDialog::Accepted)
            return 0;
        disk = QSharedPointer<Disk>(new Disk(disksFile + dialog.name,dialog.nPlatters, dialog.nTracks,
                                             dialog.nSectors, dialog.sectorSize, dialog.blockSize));
        disk->init();
    }
    else
    {
        QStringList disks;
        for (const QFileInfo& info : listFiles) disks.append(info.fileName());
        bool ok;
        QString name = QInputDialog::getItem(nullptr, "Load Disk", "Disks available:", disks, 0, false, &ok);
        if (ok && !name.isEmpty()) {
            QFile configFile(disksFile + "/" + name + "/disk.config");
            configFile.open(QIODevice::ReadOnly | QIODevice::Text);
            int nPlatters, nTracks, nSectors, sSize, bSize;
            QTextStream in(&configFile);
            in >> nPlatters >> nTracks >> nSectors >> sSize >> bSize;
            disk = QSharedPointer<Disk>(new Disk(name, nPlatters, nTracks, nSectors, sSize, bSize));
        } else return 0;
    }
    QSharedPointer<DiskController> controller(new DiskController(disk));
    Megatron w(nullptr, controller);
    w.show();
    return a.exec();
}
