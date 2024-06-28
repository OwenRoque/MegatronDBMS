#include "newrelation.h"
#include "ui_newrelation.h"


NewRelation::NewRelation(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRelation)
{
    ui->setupUi(this);
    tableWidget = new AttributeTableWidget(this);
    ui->verticalLayout_2->addWidget(tableWidget);

    ui->orgComboBox->addItem("", QVariant());
    ui->orgComboBox->addItem("Heap", QVariant::fromValue(Types::FileOrganization::Heap));
    ui->orgComboBox->addItem("Sequential/Sorted", QVariant::fromValue(Types::FileOrganization::Sequential));
    ui->orgComboBox->addItem("Hash", QVariant::fromValue(Types::FileOrganization::Hash));
    ui->orgComboBox->addItem("B+", QVariant::fromValue(Types::FileOrganization::BPlusTree));

    // TODO: enable more FOs when they are implemented
    auto* model = qobject_cast<QStandardItemModel*>(ui->orgComboBox->model());
    for (int i = 2; i < 5; i++) {
        auto* item = model->item(i);
        item->setEnabled(false);
    }

    ui->charsetComboBox->addItem("UTF-8" , QVariant::fromValue(Types::Charset::Utf8));
    ui->charsetComboBox->addItem("UTF-16 (Default)", QVariant::fromValue(Types::Charset::Utf16));
    ui->charsetComboBox->addItem("UTF-32" , QVariant::fromValue(Types::Charset::Utf32));
    ui->charsetComboBox->addItem("Latin1" , QVariant::fromValue(Types::Charset::Latin1));
    ui->charsetComboBox->setCurrentIndex(1); // default

    connect(ui->dataToolButton, &QToolButton::clicked, this, &NewRelation::open);
    connect(ui->groupBox, &QGroupBox::toggled, this, [=, this](bool toggled) {
        if (toggled == false)
            this->error();
    });

    connect(ui->addButton, &QPushButton::clicked, tableWidget, [=, this]() { tableWidget->addRow(); });
    connect(ui->removeButton, &QPushButton::clicked, tableWidget, &AttributeTableWidget::removeRow);

    connect(ui->okButton, &QPushButton::clicked, this, [=, this]() {
        if (this->validate()) accept();
    });
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

bool NewRelation::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!fileName.endsWith(".txt", Qt::CaseInsensitive) && !fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Invalid file format. \nPlease select a .txt or .csv file."));
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
    QStringList attributeList = header.split(",");
    for (auto& a: attributeList) a.replace('"', QString());
    // Save in the corresponding variable (dataPath, schemaPath)
    success(fileName, attributeList);
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
    else if (tableWidget->rowCount() == 0)
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

    /// Attribute Validation
    auto attributeValues = tableWidget->tableAttributes();
    bool aiHasBeenSet = false;
    QSet<QString> filterDuplicateAttributes;
    for (const auto& i : attributeValues)
    {
        QString attrNameValue = std::get<0>(i).simplified();
        if (filterDuplicateAttributes.find(attrNameValue) == filterDuplicateAttributes.end())
            filterDuplicateAttributes.insert(attrNameValue);
        else {
            QMessageBox::warning(this, "Warning",  tr("Duplicate attribute found: \n"
                                                     "\t\"%1\"").arg(attrNameValue));
            attributes.clear();
            indexes.clear();
            return false;
        }

        Types::DataType typeValue = std::get<1>(i);
        QString columnValue = std::get<2>(i).simplified();
        Types::KeyConstraintType keyValue = std::get<3>(i);
        QString defaultValue = std::get<4>(i).simplified();
        bool unsignedValue = std::get<5>(i);
        bool nullableValue = std::get<6>(i);
        bool aiValue = std::get<7>(i);
        QString commentValue = std::get<8>(i).simplified();

        if (attrNameValue.isEmpty()){
            QMessageBox::warning(this, "Warning",  "Empty attribute name field found.");
            attributes.clear();
            indexes.clear();
            return false;
        }

        if (typeValue == Types::Varchar && recFormat == Types::RecordFormat::Fixed) {
            QMessageBox::warning(this, "Warning",
                tr("Fixed-Length Record Format does not support VARCHAR datatypes. \n"
                "Use a Variable-Length Record Format or change your VARCHAR field to CHAR (Declared in: \"%1\")")
                .arg(attrNameValue));
            attributes.clear();
            indexes.clear();
            return false;
        }

        quint16 maxCharLength = 0;
        if (typeValue == Types::Char || typeValue == Types::Varchar)
            maxCharLength = columnValue.toUShort();

        // handle columnType storage format
        switch (typeValue) {
        case Types::TinyInt:
        case Types::UTinyInt:
            columnValue = "tinyint";
            break;
        case Types::SmallInt:
        case Types::USmallInt:
            columnValue = "smallint";
            break;
        case Types::Int:
        case Types::UInt:
            columnValue = "int";
            break;
        case Types::BigInt:
        case Types::UBigInt:
            columnValue = "bigint";
            break;
        case Types::Float:
            columnValue = "float";
            break;
        case Types::Double:
            columnValue = "double";
            break;
        case Types::Bool:
            columnValue = "bool";
            break;
        case Types::Enum:
            columnValue = "enum(" + columnValue + ")";
            break;
        case Types::Char:
            columnValue = "char(" + columnValue + ")";
            break;
        case Types::Varchar:
            columnValue = "varchar(" + columnValue + ")";
            break;
        }

        // determine whether unsigned, nullable or ai states are valid according to dataType, keyConstraint
        if (typeValue == Types::DataType::Bool ||
            typeValue == Types::DataType::Enum ||
            typeValue == Types::DataType::Char ||
            typeValue == Types::DataType::Varchar) {
            unsignedValue = false;
            aiValue = false;

        }
        if (keyValue == Types::KeyConstraintType::Primary)
            nullableValue = false;
        if (keyValue != Types::KeyConstraintType::Primary &&
            keyValue != Types::KeyConstraintType::Unique)
            aiValue = false;

        if (aiValue) {
            if (!aiHasBeenSet)
                aiHasBeenSet = true;
            else {
                QMessageBox::warning(this, "Warning",  "Duplicated auto-increment.");
                attributes.clear();
                indexes.clear();
                return false;
            }
        }

        attributes.emplaceBack(attrNameValue, typeValue, columnValue, maxCharLength, defaultValue,
                               nullableValue, unsignedValue, aiValue, keyValue, commentValue);
    }

    /// Index Validation
    auto indexValues = tableWidget->tableIndexes();
    for (const auto& i : indexValues)
    {
        QString attrNameValue;
        QString indexNameValue;
        Types::KeyConstraintType keyTypeValue;
        Types::IndexType indexTypeValue;
        QString commentValue;
        std::tie(attrNameValue, indexNameValue, keyTypeValue, indexTypeValue, commentValue) = i;

        // default configuration properties (PRIMARY)
        Core::IndexProperties idxProp =
        {
            .isNonUnique = false,
            .isNullable = false,
            .isClustered = true,
            .order = Types::ASC,
            .ordinalPosition = 1,
            .comment = commentValue
        };

        if (fileOrg == Types::FileOrganization::Heap &&
            keyTypeValue == Types::KeyConstraintType::Primary)
        {
            int ret = QMessageBox::information(this, "Information", tr("Heap File Organization do not implement any kind of data sorting "
                "physically on disk via indexes. The Primary Key defined on: \n"
                    "\t\"%1\" \n"
                    "will become a non-clustered index. \n"
                    "Do you want to proceed?").arg(attrNameValue), QMessageBox::Ok | QMessageBox::Cancel);
            switch (ret) {
            case QMessageBox::Ok:
            {
                idxProp.isClustered = false;
                break;
            }
            case QMessageBox::Cancel:
            {
                attributes.clear();
                indexes.clear();
                return false;
            }
            }
        }

        // validate that file organization and index types match
        if (keyTypeValue == Types::KeyConstraintType::Primary) {
            switch (indexTypeValue) {
            case Types::SequentialIndex:
            {
                if (fileOrg != Types::FileOrganization::Sequential)
                {
                    QMessageBox::warning(this, "Warning",  "Sequential File Organization needs to match with Index Type.");
                    attributes.clear();
                    indexes.clear();
                    return false;
                }
            }
            case Types::HashIndex:
            {
                if (fileOrg != Types::FileOrganization::Hash)
                {
                    QMessageBox::warning(this, "Warning",  "Hash File Organization needs to match with Index Type.");
                    attributes.clear();
                    indexes.clear();
                    return false;
                }
            }
            case Types::BPlusTreeIndex:
            {
                if (fileOrg != Types::FileOrganization::BPlusTree)
                {
                    QMessageBox::warning(this, "Warning",  "B+Tree File Organization needs to match with Index Type.");
                    attributes.clear();
                    indexes.clear();
                    return false;
                }
            }
            }

            // preparing index properties according to keyConstraint
            switch(keyTypeValue) {
            case Types::Primary:
            {
                // nothing to adjust, default template is Primary
                break;
            }
            case Types::Unique:
            {
                const auto attribute = std::find_if(attributes.cbegin(), attributes.cend(), [&attrNameValue](const auto& tuple) {
                    QString attrName = std::get<0>(tuple);
                    return attrName == attrNameValue;
                });
                idxProp.isClustered = false;
                idxProp.isNonUnique = false;
                idxProp.isNullable = std::get<5>(*attribute);       // null attribute state
                break;
            }
            case Types::Index:
            {
                const auto attribute = std::find_if(attributes.cbegin(), attributes.cend(), [&attrNameValue](const auto& tuple) {
                    QString attrName = std::get<0>(tuple);
                    return attrName == attrNameValue;
                });
                idxProp.isClustered = false;
                idxProp.isNonUnique = true;
                idxProp.isNullable = std::get<5>(*attribute);       // null attribute state
                break;
            }
            case Types::Foreign:
            case Types::None:
                break;
            }
        }

        indexes.emplaceBack(indexNameValue, attrNameValue, indexTypeValue, idxProp);
    }
    return true;
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
    tableWidget->clearContents();
}

void NewRelation::success(const QString &fileName, const QStringList& attributeList)
{
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
    tableWidget->clearContents();
    for (qsizetype i = 0; i < attributeList.size(); ++i)
    {
        tableWidget->addRow(attributeList.at(i));
    }
}
