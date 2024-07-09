#ifndef DISKLOAD_H
#define DISKLOAD_H

#include <QDialog>

namespace Ui {
class DiskLoad;
}

class DiskLoad : public QDialog
{
    Q_OBJECT

public:
    explicit DiskLoad(const QStringList& disks = QStringList(), QWidget *parent = nullptr);
    ~DiskLoad();
    QString getSelectedDisk() const;

// signals:
//     void newDiskRequested();

private slots:
    void onOkButtonClicked();
    void onNewDiskButtonClicked();

private:
    Ui::DiskLoad *ui;
    QString selectedDisk;
};

#endif // DISKLOAD_H
