#ifndef DISKINIT_H
#define DISKINIT_H

#include <QDialog>

namespace Ui {
class DiskInit;
}

class DiskInit : public QDialog
{
    Q_OBJECT

public:
    explicit DiskInit(QWidget *parent = nullptr);
    ~DiskInit();
    QString name;
    int nPlatters;
    int nTracks;
    int nSectors;
    int sectorSize;
    int blockSize;

private slots:
    void setBlockMultiplier(int);

private:
    Ui::DiskInit *ui;

};

#endif // DISKINIT_H
