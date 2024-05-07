#include "diskinit.h"
#include "ui_diskinit.h"

#include <QPushButton>

DiskInit::DiskInit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DiskInit)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [this]() {
        name = ui->nameLineEdit->text().simplified();
        nPlatters = ui->platterSpinBox->value();
        nTracks = ui->trackSpinBox->value();
        nSectors = ui->sectorSpinBox->value();
        sectorSize = ui->sectorSizeSpinBox->value();
        blockSize = ui->blockSizeComboBox->currentText().toInt();
        this->accept();
    });
    connect(ui->sectorSpinBox, &QSpinBox::valueChanged, this, &DiskInit::setBlockMultiplier);
}

DiskInit::~DiskInit()
{
    delete ui;
}

void DiskInit::setBlockMultiplier(int sectorValue)
{
    int sSize = ui->sectorSizeSpinBox->value();
    ui->blockSizeComboBox->clear();
    for (int i = 1; i <= sectorValue; ++i)
        if (sectorValue % i == 0) {
            int bSize = sSize * i;
            ui->blockSizeComboBox->addItem(QString::number(bSize));
        }
}
