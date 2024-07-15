#include "diskload.h"
#include "ui_diskload.h"

DiskLoad::DiskLoad(const QStringList& disks, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DiskLoad)
{
    ui->setupUi(this);
    ui->comboBox->addItems(disks);
    connect(ui->okButton, &QPushButton::clicked, this, &DiskLoad::onOkButtonClicked);
    connect(ui->newDiskButton, &QPushButton::clicked, this, &DiskLoad::onNewDiskButtonClicked);
}

DiskLoad::~DiskLoad()
{
    delete ui;
}

QString DiskLoad::getSelectedDisk() const
{
    return selectedDisk;
}

void DiskLoad::onOkButtonClicked()
{
    selectedDisk = ui->comboBox->currentText();
    accept();
}

void DiskLoad::onNewDiskButtonClicked()
{
    // send custom signal
    done(2);
}
