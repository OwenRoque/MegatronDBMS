#include "newrelation.h"
#include "ui_newrelation.h"

NewRelation::NewRelation(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRelation)
{
    ui->setupUi(this);
    ui->orgComboBox->addItem("Heap", QVariant::fromValue(Types::FileOrganization::Heap));
    ui->orgComboBox->addItem("Sequential/Sorted"/*, QVariant::fromValue(Types::FileOrganization::Sequential)*/);
    ui->orgComboBox->addItem("Hash"/*, QVariant::fromValue(Types::FileOrganization::Hash)*/);
    ui->orgComboBox->addItem("B+"/*, QVariant::fromValue(Types::FileOrganization::BPlusTree)*/);

    // TODO: enable more FOs when they are implemented
    auto* model = qobject_cast<QStandardItemModel*>(ui->orgComboBox->model());
    for (int i = 1; i < 4; i++) {
        auto* item = model->item(i);
        item->setEnabled(false);
    }

    ui->charsetComboBox->addItem("UTF-8" , QVariant::fromValue(Types::Charset::Utf8));
    ui->charsetComboBox->addItem("UTF-16 (Default)", QVariant::fromValue(Types::Charset::Utf16));
    ui->charsetComboBox->addItem("UTF-32" , QVariant::fromValue(Types::Charset::Utf32));
    ui->charsetComboBox->addItem("Latin1" , QVariant::fromValue(Types::Charset::Latin1));

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
    package.charset = this->charset;
    return package;
}

void NewRelation::addRow()
{
    int newRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(newRow);

    QLineEdit *nameLineEdit = new QLineEdit(this);
    nameLineEdit->setMaxLength(50);
    ui->tableWidget->setCellWidget(newRow, 0, nameLineEdit);
    ui->tableWidget->setColumnWidth(0, 200);

    QComboBox *typeLineEdit = new QComboBox(this);
    typeLineEdit->addItem("");
    typeLineEdit->addItem("TINYINT", QVariant::fromValue(Types::DataType::TinyInt));
    typeLineEdit->addItem("SMALLINT", QVariant::fromValue(Types::DataType::SmallInt));
    typeLineEdit->addItem("INT", QVariant::fromValue(Types::DataType::Int));
    typeLineEdit->addItem("BIGINT", QVariant::fromValue(Types::DataType::BigInt));
    typeLineEdit->addItem("FLOAT", QVariant::fromValue(Types::DataType::Float));
    typeLineEdit->addItem("DOUBLE", QVariant::fromValue(Types::DataType::Double));
    typeLineEdit->addItem("BOOL", QVariant::fromValue(Types::DataType::Bool));
    typeLineEdit->addItem("CHAR()", QVariant::fromValue(Types::DataType::Char));
    typeLineEdit->addItem("VARCHAR()", QVariant::fromValue(Types::DataType::Varchar));
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
    ui->tableWidget->setColumnWidth(1, 150);

    QComboBox *indexComboBox = new QComboBox(this);
    indexComboBox->addItem("", Types::KeyConstraintType::None);
    indexComboBox->addItem("PRIMARY", Types::KeyConstraintType::Primary);
    indexComboBox->addItem("UNIQUE", Types::KeyConstraintType::Unique);
    indexComboBox->addItem("INDEX", Types::KeyConstraintType::Index);
    ui->tableWidget->setCellWidget(newRow, 2, indexComboBox);
    ui->tableWidget->setColumnWidth(2, 125);

    QLineEdit *defaultLineEdit = new QLineEdit(this);
    ui->tableWidget->setCellWidget(newRow, 3, defaultLineEdit);
    ui->tableWidget->setColumnWidth(3, 125);

    QCheckBox *unsignedCheckBox = new QCheckBox(this);
    ui->tableWidget->setCellWidget(newRow, 4, unsignedCheckBox);
    ui->tableWidget->setColumnWidth(4, 80);

    QCheckBox *nullCheckBox = new QCheckBox(this);
    ui->tableWidget->setCellWidget(newRow, 5, nullCheckBox);
    ui->tableWidget->setColumnWidth(5, 80);

    QCheckBox *aiCheckBox = new QCheckBox(this);
    ui->tableWidget->setCellWidget(newRow, 6, aiCheckBox);
    ui->tableWidget->setColumnWidth(6, 80);

    QLineEdit *commentLineEdit = new QLineEdit(this);
    ui->tableWidget->setCellWidget(newRow, 7, commentLineEdit);
    nameLineEdit->setMaxLength(255);
    ui->tableWidget->setColumnWidth(7, 100);

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
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
    fileOrg = ui->orgComboBox->currentData().value<Types::FileOrganization>();
    if (ui->fixedRadioButton->isChecked()) recFormat = Types::RecordFormat::Fixed;
    else recFormat = Types::RecordFormat::Variable;
    charset = ui->charsetComboBox->currentData().value<Types::Charset>();

    // Create GUI for index creation (PRIMARY, UNIQUE, INDEX)
    quint8 nPrimaryKeys = 0;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QString attributeName = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(row, 0))->text().simplified();
        Types::DataType typeValue = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1))->currentData().value<Types::DataType>();
        QString typeCharLength = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 1))->currentText().simplified();
        Types::KeyConstraintType keyValue = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2))->currentData().value<Types::KeyConstraintType>();
        QString defaultValue = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(row, 3))->text().simplified();
        bool isUnsigned = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 4))->isChecked();
        bool isNullable = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 5))->isChecked();
        bool ai = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(row, 6))->isChecked();
        QString commentValue = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(row, 7))->text().simplified();

        // empty row
        if (attributeName.isEmpty())
        {
            QMessageBox::warning(this, "Warning", "Row " + QString::number(row) + " has no attribute name.");
            attributes.clear();
            indexes.clear();
            return false;
        }

        // valid type
        quint16 length = 0;
        static QRegularExpression charRegex("^CHAR\\((\\d+)\\)$");
        static QRegularExpression varcharRegex("^VARCHAR\\((\\d+)\\)$");
        QRegularExpressionMatch match;

        if (typeValue == Types::Char && (match = charRegex.match(typeCharLength)).hasMatch())
        {
            length = match.captured(1).toUShort();
        }
        else if (typeValue == Types::Varchar && (match = varcharRegex.match(typeCharLength)).hasMatch())
        {
            if (recFormat == Types::RecordFormat::Fixed)
            {
                QMessageBox::warning(this, "Warning",
                    tr("Fixed-Length Record Format does not support VARCHAR datatypes. "
                        "Use a Variable-Length Record Format or change your VARCHAR field to CHAR (Declared in: %1)").arg(attributeName));
                attributes.clear();
                indexes.clear();
                return false;
            }
            length = match.captured(1).toUShort();
        }
        else
        {
            QMessageBox::warning(this, "Warning", tr("Incorrect syntax: %1 in row %2").arg(typeCharLength).arg(row));
            attributes.clear();
            indexes.clear();
            return false;
        }

        // Create indexes according to defined keys
        switch (keyValue) {
        case Types::KeyConstraintType::Primary:
        {
            QString indexName = "PK_" + relationName + "_" + attributeName;
            Types::IndexType idx;
            switch (fileOrg) {
            case Types::FileOrganization::Sequential:
                idx = Types::IndexType::SequentialIndex;
                break;
            case Types::FileOrganization::Hash:
                idx = Types::IndexType::HashIndex;
                break;
            case Types::FileOrganization::BPlusTree:
                idx = Types::IndexType::BPlusTreeIndex;
                break;
                // This case won't ever happen
            case Types::FileOrganization::Heap:
                break;
            }
            Core::IndexProperties idxProp =
            {
                .isNonUnique = false,
                .isNullable = false,
                .isClustered = true,
                .order = Types::ASC,
                .ordinalPosition = nPrimaryKeys,
                .comment = ""
            };
            indexes.emplace_back(indexName, attributeName, idx, idxProp);
            break;
        }
        case Types::KeyConstraintType::Unique:
        {
            QString indexName = "UK_" + relationName + "_" + attributeName;
            Types::IndexType idx = Types::IndexType::BPlusTreeIndex; // temporal
            Core::IndexProperties idxProp =
            {
                .isNonUnique = false,
                .isNullable = isNullable,
                .isClustered = false,
                .order = Types::ASC,
                .ordinalPosition = 0,
                .comment = ""
            };
            indexes.emplace_back(indexName, attributeName, idx, idxProp);
            break;
        }
        case Types::KeyConstraintType::Index:
        {
            QString indexName = "IX_" + relationName + "_" + attributeName;
            Types::IndexType idx = Types::IndexType::BPlusTreeIndex; // temporal
            Core::IndexProperties idxProp =
            {
                .isNonUnique = true,
                .isNullable = isNullable,
                .isClustered = false,
                .order = Types::ASC,
                .ordinalPosition = 0,
                .comment = ""
            };
            indexes.emplace_back(indexName, attributeName, idx, idxProp);
            break;
        }
        case Types::KeyConstraintType::Foreign:
        case Types::KeyConstraintType::None:
            break;
        }

        // add null, auto-increment properties
        attributes.emplace_back(attributeName, typeValue, typeCharLength.toLower(), length,
                                defaultValue, isNullable, isUnsigned, ai, keyValue, commentValue);
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
    if (ui->groupBox->layout()->count() >= 4) {
        QWidget *widgetToRemove = ui->groupBox->layout()->itemAt(3)->widget();
        if (widgetToRemove) {
            ui->groupBox->layout()->removeWidget(widgetToRemove);
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
    if (ui->groupBox->layout()->count() < 4) {
        QLabel * label = new QLabel("Loaded Successfully", this);
        label->setStyleSheet("color: green; font-weight: bold;");
        ui->groupBox->layout()->addWidget(label);
    }
    // update attributes table
    ui->tableWidget->clearContents();
    QStringList attrNames = header.split(",");
    for (auto& i : attrNames)
    {
        i.replace('"', QString());
        this->addRow();
        QLineEdit* attribName = qobject_cast<QLineEdit*>(ui->tableWidget->cellWidget(ui->tableWidget->rowCount() - 1, 0));
        attribName->setText(i);
    }
}
