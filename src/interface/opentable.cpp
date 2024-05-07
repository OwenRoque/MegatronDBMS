#include "opentable.h"
#include "ui_opentable.h"

OpenTable::OpenTable(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OpenTable)
{
    ui->setupUi(this);
    connect(ui->dataToolButton, &QToolButton::clicked, this, [=]() { open(true); ui->okButton->setEnabled(filesLoaded()); });
    connect(ui->schemaToolButton, &QToolButton::clicked, this, [=]() { open(false); ui->okButton->setEnabled(filesLoaded()); });
    connect(ui->okButton, &QPushButton::clicked, this, &OpenTable::accept);
    ui->okButton->setEnabled(false);
}

OpenTable::~OpenTable()
{
    delete ui;
}

QString OpenTable::getDataPath() const
{
    return dataPath;
}

QString OpenTable::getSchemaPath() const
{
    return schemaPath;
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

void OpenTable::open(bool isData)
{
    QFileDialog dialog(this, tr("Open File"));
    initFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().constFirst(), isData)) {}
}

void OpenTable::error(bool isData)
{
    // Reset to default
    if (isData) {
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
    }
    else {
        schemaPath.clear();
        ui->schemaLabel->clear();
        ui->schemaLabel->setText("Schema:");
        if (ui->horizontalLayout->count() >= 3) {
            QWidget *widgetToRemove = ui->horizontalLayout->itemAt(2)->widget();
            if (widgetToRemove) {
                ui->horizontalLayout->removeWidget(widgetToRemove);
                delete widgetToRemove;
            }
        }
    }
}

void OpenTable::success(bool isData, const QString &fileName)
{
    // qDebug() << ui->horizontalLayout2->count() << " " << ui->horizontalLayout->count();
    if (isData) {
        dataPath = fileName;
        ui->dataLabel->clear();
        QFileInfo info(fileName);
        ui->dataLabel->setText("Data: <b>" + info.fileName() + "</b>");
        if (ui->horizontalLayout2->count() < 3) {
            QLabel * label = new QLabel("Loaded Successfully", this);
            label->setStyleSheet("color: green; font-weight: bold;");
            ui->horizontalLayout2->addWidget(label);
        }
    }
    else {
        schemaPath = fileName;
        ui->schemaLabel->clear();
        QFileInfo info(fileName);
        ui->schemaLabel->setText("Schema: <b>" + info.fileName() + "</b>");
        if (ui->horizontalLayout->count() < 3) {
            QLabel * label = new QLabel("Loaded Successfully", this);
            label->setStyleSheet("color: green; font-weight: bold;");
            ui->horizontalLayout->addWidget(label);
        }
    }
}

bool OpenTable::filesLoaded() const
{
    return !dataPath.isEmpty() && !schemaPath.isEmpty();
}

bool OpenTable::loadFile(const QString &fileName, bool isData)
{
    QFile file(fileName);
    if (!fileName.endsWith(".txt", Qt::CaseInsensitive) && !fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Invalid file format. Please select a .txt or .csv file."));
        error(isData);
        return false;
    }
    else if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot open %1: %2")
                                     .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        error(isData);
        return false;
    }
    // Save in the corresponding variable (dataPath, schemaPath)
    success(isData, fileName);
    setWindowFilePath(fileName);
    return true;
}
