#include "newrelation.h"
#include "ui_newrelation.h"

NewRelation::NewRelation(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRelation)
{
    ui->setupUi(this);
    ui->OrgComboBox->addItem("Heap", Types::FileOrganization::Heap);
    ui->OrgComboBox->addItem("Sequential/Sorted", Types::FileOrganization::Sequential);
    ui->OrgComboBox->addItem("Hash", Types::FileOrganization::Hash);
    ui->OrgComboBox->addItem("B+", Types::FileOrganization::BPlusTree);

    // TODO: enable more FOs when they are implemented
    auto* model = qobject_cast<QStandardItemModel*>(ui->OrgComboBox->model());
    for (int i = 1; i < 4; i++) {
        auto* item = model->item(i);
        item->setEnabled(false);
    }

    connect(ui->dataToolButton, &QToolButton::clicked, this, &NewRelation::open);
    connect(ui->groupBox, &QGroupBox::toggled, this, [=](bool toggled) {
        if (!toggled)
        {
            dataPath.clear();
            ui->tableWidget->clearContents();
        }
    });
    connect(ui->addButton, &QPushButton::clicked, this, &NewRelation::addRow);
    connect(ui->removeButton, &QPushButton::clicked, this, &NewRelation::removeRow);
    connect(ui->upButton, &QPushButton::clicked, this, &NewRelation::moveRowUp);
    connect(ui->downButton, &QPushButton::clicked, this, &NewRelation::moveRowDown);

    connect(ui->okButton, &QPushButton::clicked, this, [=]() { if (validate()) accept(); });
    ui->okButton->setEnabled(false);
}

NewRelation::~NewRelation()
{
    delete ui;
}

Core::RelationInput NewRelation::getDataPackage() const
{
    Core::RelationInput package;
    package.relationName = this->relationName;
    package.dataPath = this->dataPath;
    package.attributes = this->attributes;
    package.indexes = this->indexes;
    package.fileOrg = this->fileOrg;
    package.recFormat = this->recFormat;
    return package;
}

void NewRelation::addRow()
{
    int newRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(newRow);

    QLineEdit *nameLineEdit = new QLineEdit(this);
    nameLineEdit->setMaxLength(50);
    ui->tableWidget->setCellWidget(newRow, 0, nameLineEdit);

    QComboBox *typeLineEdit = new QComboBox(this);
    typeLineEdit->addItem("TINYINT", Types::DataType::TinyInt);
    typeLineEdit->addItem("SMALLINT", Types::DataType::SmallInt);
    typeLineEdit->addItem("INT", Types::DataType::Int);
    typeLineEdit->addItem("BIGINT", Types::DataType::BigInt);
    typeLineEdit->addItem("FLOAT", Types::DataType::Float);
    typeLineEdit->addItem("DOUBLE", Types::DataType::Double);
    typeLineEdit->addItem("BOOL", Types::DataType::Bool);
    typeLineEdit->addItem("CHAR()", Types::DataType::Char);
    typeLineEdit->addItem("VARCHAR()", Types::DataType::Varchar);
    typeLineEdit->setEditable(true);
    typeLineEdit->setInsertPolicy(QComboBox::NoInsert);
    typeLineEdit->lineEdit()->setReadOnly(true);

    connect(typeLineEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        QVariant userData = typeLineEdit->itemData(index);
        Types::DataType type = static_cast<Types::DataType>(userData.toInt());

        if (type == Types::Char || type == Types::Varchar) {
            typeLineEdit->lineEdit()->setReadOnly(false);
        } else {
            typeLineEdit->lineEdit()->setReadOnly(true);
        }
        // qDebug() << "Selected Type:" << type;
    });

    ui->tableWidget->setCellWidget(newRow, 1, typeLineEdit);

    QComboBox *keyComboBox = new QComboBox(this);
    keyComboBox->addItem("NONE", Types::KeyType::None);
    keyComboBox->addItem("PRIMARY", Types::KeyType::Primary);
    keyComboBox->addItem("UNIQUE", Types::KeyType::Unique);
    ui->tableWidget->setCellWidget(newRow, 2, keyComboBox);

    QCheckBox *nullRadioButton = new QCheckBox(this);
    ui->tableWidget->setCellWidget(newRow, 3, nullRadioButton);

    QCheckBox *aiRadioButton = new QCheckBox(this);
    ui->tableWidget->setCellWidget(newRow, 4, aiRadioButton);
}

void NewRelation::removeRow()
{
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow >= 0) {
        ui->tableWidget->removeRow(currentRow);
    }
}

void NewRelation::moveRowUp()
{
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow > 0) {
        swapRows(currentRow, currentRow - 1);
        ui->tableWidget->selectRow(currentRow - 1);
    }
}

void NewRelation::moveRowDown()
{
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < ui->tableWidget->rowCount() - 1) {
        swapRows(currentRow, currentRow + 1);
        ui->tableWidget->selectRow(currentRow + 1);
    }
}

bool NewRelation::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!fileName.endsWith(".txt", Qt::CaseInsensitive) && !fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Invalid file format. Please select a .txt or .csv file."));
        error();
        return false;
    }
    else if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot open %1: %2")
                                     .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        error();
        return false;
    }
    QTextStream in(&file);
    QString header;
    in.readLineInto(&header);
    file.close();
    // Save in the corresponding variable (dataPath, schemaPath)
    success(fileName, header);
    setWindowFilePath(fileName);
    return true;
}

static void initFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;
    if (firstDialog) {
        firstDialog = false;
        const QStringList downloadsLocations = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
        dialog.setDirectory(downloadsLocations.isEmpty() ? QDir::currentPath() : downloadsLocations.last());
    }
    QStringList mimeTypeFilters;
    mimeTypeFilters << "CSV files (*.csv)" << "Text files (*.txt)";
    dialog.setNameFilters(mimeTypeFilters);
    dialog.selectNameFilter("CSV files (*.csv)");
    dialog.setAcceptMode(acceptMode);
    dialog.setDefaultSuffix("csv"); // Default to .csv extension
}

void NewRelation::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().constFirst())) {}
}

bool NewRelation::validate()
{
    if (ui->relNameLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Relation Name field is empty.");
        return false;
    }
    else if (ui->tableWidget->rowCount() == 0)
    {
        QMessageBox::warning(this, "Warning", "There must be at least one attribute row for the relation.");
        return false;
    }
    // validate while setting values
    relationName = ui->relNameLineEdit->text().simplified();
    QVariant value = ui->OrgComboBox->currentData();
    fileOrg = static_cast<Types::FileOrganization>(value.toInt());
    if (ui->fixedRadioButton->isChecked()) recFormat = Types::RecordFormat::Fixed;
    else recFormat = Types::RecordFormat::Variable;
    QString primaryField;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QString attribName = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(row, 0))->text().simplified();
        QVariant typeValue = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1))->currentData();
        QString text = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1))->currentText().simplified();
        QVariant keyValue = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2))->currentData();
        bool null = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 3))->isChecked();
        QCheckBox* ai = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 4));
        int autoIncrement = (ai->isChecked()) ? 1 : -1;
        // empty row
        if (attribName.isEmpty())
        {
            QMessageBox::warning(this, "Warning", "Row " + QString::number(row) + " has no attribute name.");
            attributes.clear();
            indexes.clear();
            return false;
        }
        // valid type
        Types::DataType type = static_cast<Types::DataType>(typeValue.toInt()); int length = -1;
        QRegularExpression charRegex("^CHAR\\((\\d+)\\)$");
        QRegularExpression varcharRegex("^VARCHAR\\((\\d+)\\)$");

        QRegularExpressionMatch match;
        if (type == Types::Char && (match = charRegex.match(text)).hasMatch()) {
            length = match.captured(1).toInt();
        } else if (type == Types::Varchar && (match = varcharRegex.match(text)).hasMatch()) {
            if (recFormat == Types::RecordFormat::Fixed)
            {
                QMessageBox::warning(this, "Warning",
                    tr("Fixed-Length Record Format does not support VARCHAR datatype. "
                        "Use a Variable-Length Record Format instead. (Declared in: %1)").arg(attribName));
                attributes.clear();
                indexes.clear();
                return false;
            }
            length = match.captured(1).toInt();
        }
        else {
            QMessageBox::warning(this, "Warning", tr("Incorrect syntax: %1 in row %2").arg(text).arg(row));
            attributes.clear();
            indexes.clear();
            return false;
        }
        // Create indexes according to defined keys
        Types::KeyType key = static_cast<Types::KeyType>(keyValue.toInt());
        switch (key) {
        case Types::KeyType::Primary:
        {
            if (primaryField.isEmpty())
            {
                primaryField = attribName;
                QString indexName = "PK_" + relationName + "_" + attribName;
                Types::IndexType idx;
                switch (fileOrg) {
                case Types::FileOrganization::Heap:
                    idx = Types::IndexType::NonClusterIndex;
                    break;
                case Types::FileOrganization::Sequential:
                    idx = Types::IndexType::ClusterIndex;
                    break;
                case Types::FileOrganization::Hash:
                    idx = Types::IndexType::HashIndex;
                    break;
                case Types::FileOrganization::BPlusTree:
                    idx = Types::IndexType::BPlusTreeIndex;
                    break;
                }
                Core::IndexProperties idxProp =
                    {
                        .columns = {attribName},
                        .keyType = key,
                        .columnOrders = {Core::IndexProperties::ASC}
                    };
                indexes.emplace_back(indexName, attribName, idx, idxProp);
            }
            else
            {
                QMessageBox::warning(this, "Warning", "Duplicated Primary Key");
                attributes.clear();
                indexes.clear();
                return false;
            }
            break;
        }
        case Types::KeyType::Unique:
        {
            QString indexName = "UK_" + relationName + "_" + attribName;
            Types::IndexType idx = Types::IndexType::NonClusterIndex;
            Core::IndexProperties idxProp =
            {
                .columns = {attribName},
                .keyType = key,
                .columnOrders = {Core::IndexProperties::ASC}
            };
            indexes.emplace_back(indexName, attribName, idx, idxProp);
            break;
        }
        case Types::KeyType::Foreign:
        case Types::KeyType::None:
            break;
        }
        // add null, auto-increment properties
        attributes.emplace_back(attribName, type, length, null, autoIncrement);
    }
    return true;
}

void NewRelation::swapRows(int row1, int row2)
{
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
        QWidget *widget1 = ui->tableWidget->cellWidget(row1, col);
        QWidget *widget2 = ui->tableWidget->cellWidget(row2, col);

        ui->tableWidget->removeCellWidget(row1, col);
        ui->tableWidget->removeCellWidget(row2, col);

        if (widget1) ui->tableWidget->setCellWidget(row2, col, widget1);
        if (widget2) ui->tableWidget->setCellWidget(row1, col, widget2);
    }
}

void NewRelation::error()
{
    // Reset to default
    dataPath.clear();
    ui->dataLabel->clear();
    ui->dataLabel->setText("Data:");
    // Remove success message
    if (ui->horizontalLayout2->count() >= 3) {
        QWidget *widgetToRemove = ui->horizontalLayout2->itemAt(2)->widget();
        if (widgetToRemove) {
            ui->horizontalLayout2->removeWidget(widgetToRemove);
            delete widgetToRemove;
        }
    }
    // update attributes table
    ui->tableWidget->clearContents();
}

void NewRelation::success(const QString &fileName, const QString& header)
{
    // qDebug() << ui->horizontalLayout2->count() << " " << ui->horizontalLayout->count();
    dataPath = fileName;
    ui->dataLabel->clear();
    QFileInfo info(fileName);
    ui->dataLabel->setText("Data: <b>" + info.fileName() + "</b>");
    if (ui->horizontalLayout2->count() < 3) {
        QLabel * label = new QLabel("Loaded Successfully", this);
        label->setStyleSheet("color: green; font-weight: bold;");
        ui->horizontalLayout2->addWidget(label);
    }
    // update attributes table
    ui->tableWidget->clearContents();
    QStringList attrNames = header.split(",");
    for (auto& i : attrNames)
    {
        i.replace('"', QString());
        this->addRow();
        QLineEdit* attribName = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->rowCount(), 0));
        attribName->setText(i);
    }
}
