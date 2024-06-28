#include "indexform.h"
#include "ui_indexform.h"

IndexForm::IndexForm(QWidget *parent, const Types::KeyConstraintType& opt)
    : QDialog(parent), ui(new Ui::IndexForm)
{
    ui->setupUi(this);

    const QList<QPair<QString, QVariant>> indexOptionComboBoxItems = {
        {"", QVariant()},
        {"PRIMARY", QVariant::fromValue(Types::KeyConstraintType::Primary)},
        {"UNIQUE", QVariant::fromValue(Types::KeyConstraintType::Unique)},
        {"INDEX", QVariant::fromValue(Types::KeyConstraintType::Index)}
    };
    const QList<QPair<QString, QVariant>> indexTypeComboBoxItems = {
        {"", QVariant()},
        {"BTREE", QVariant::fromValue(Types::IndexType::BPlusTreeIndex)},
        {"HASH", QVariant::fromValue(Types::IndexType::HashIndex)},
        {"SEQUENTIAL", QVariant::fromValue(Types::IndexType::SequentialIndex)}
    };
    for (const auto &item : indexOptionComboBoxItems) {
        ui->indexOptionComboBox->addItem(item.first, item.second);
    }
    for (const auto &item : indexTypeComboBoxItems) {
        ui->indexTypeComboBox->addItem(item.first, item.second);
    }

    if (opt != Types::KeyConstraintType::None) {
        int index = ui->indexOptionComboBox->findData(opt);
        ui->indexOptionComboBox->setCurrentIndex(index);
        ui->indexOptionComboBox->setEnabled(false);
    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &IndexForm::accept);
}

IndexForm::~IndexForm()
{
    delete ui;
}

QString IndexForm::getIndexName() const
{
    return ui->indexNameLineEdit->text().simplified();
}

Types::KeyConstraintType IndexForm::getIndexOption() const
{
    return ui->indexOptionComboBox->currentData().value<Types::KeyConstraintType>();
}

Types::IndexType IndexForm::getIndexType() const
{
    return ui->indexTypeComboBox->currentData().value<Types::IndexType>();
}

QString IndexForm::getComment() const
{
    return ui->commentTextEdit->toPlainText();
}

void IndexForm::setIndexName(const QString &indexName)
{
    ui->indexNameLineEdit->setText(indexName);
}

void IndexForm::setIndexOption(const Types::KeyConstraintType &indexOption)
{
    int index = ui->indexOptionComboBox->findData(indexOption);
    ui->indexOptionComboBox->setCurrentIndex(index);
}

void IndexForm::setIndexType(const Types::IndexType &indexType)
{
    int index = ui->indexTypeComboBox->findData(indexType);
    ui->indexTypeComboBox->setCurrentIndex(index);
}

void IndexForm::setComment(const QString &comment)
{
    ui->commentTextEdit->setText(comment);
}



