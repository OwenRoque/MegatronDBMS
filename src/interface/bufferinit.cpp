#include "bufferinit.h"
#include "ui_bufferinit.h"

BufferInit::BufferInit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BufferInit)
{
    ui->setupUi(this);
    connect(ui->okButton, &QPushButton::clicked, this, &BufferInit::onOkButtonClicked);
}

BufferInit::~BufferInit()
{
    delete ui;
}

int BufferInit::getBufferSize() const
{
    return bufferSize;
}

QString BufferInit::getPolicy() const
{
    return policy;
}

void BufferInit::onOkButtonClicked()
{
    bufferSize = ui->spinBox->value();
    policy = ui->comboBox->currentText();
    accept();
}
