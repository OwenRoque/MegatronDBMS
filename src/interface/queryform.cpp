#include "queryform.h"
#include "ui_queryform.h"
#include <systemcatalog.h>
#include <megatron_types.h>

#include <QMessageBox>
#include <QMultiMap>
#include <QList>

QueryForm::QueryForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QueryForm)
{
    ui->setupUi(this);
    tableWidget = ui->tableWidget;
    attrInput = ui->attrLineEdit;
    tableInput = ui->tableLineEdit;
    whereClause = ui->whereCheckBox;
    columnInput = ui->fieldOnelineEdit;
    comparisonOperator = ui->operatorComboBox;
    firstCond = ui->fieldTwolineEdit;
    secondCond = ui->fieldThreelineEdit;
    selectIntoClause = ui->selectIntoCheckBox;
    newTableInput = ui->selectIntoLineEdit;
    createActions();
}

QueryForm::~QueryForm()
{
    delete ui;
}

void QueryForm::warning(const QString &message, QWidget *parent)
{
    QMessageBox msgBox(QMessageBox::Warning, "Warning", message, QMessageBox::Ok, parent);
    QString styleSheet = "QMessageBox QLabel { font-size: 11pt; }";
    msgBox.setStyleSheet(styleSheet);
    msgBox.exec();
}

bool QueryForm::validateForm()
{
    QString table = tableInput->text().trimmed();
    QString col = columnInput->text().trimmed();
    QString fcond = firstCond->text().trimmed();
    QString scond = secondCond->text().trimmed();
    QString name = newTableInput->text().trimmed();
    int op = comparisonOperator->currentIndex();
    // form validation
    if (attrInput->text().isEmpty()) { warning("Attributes field is empty.", this); return false; }
    else if (table.isEmpty()) { warning("Table field is empty.", this); return false; }
    auto isNumber = [](const QString& q) {
        bool ok;
        q.toDouble(&ok);
        return ok;
    };
    if (whereClause->isChecked()) {
        if (col.isEmpty()) { warning("Column field is empty.", this); return false; }
        switch (op) {
        case 0: case 1: case 4: case 5:
        {
            if (fcond.isEmpty()) { warning("Condition field is empty.", this); return false; }
            else if (!isNumber(fcond)) { warning("Condition field needs to be a digit.", this); return false; }
            break;
        }
        case 2: case 3: case 6: case 7: case 8: case 9: case 10: case 11:
        {
            if (fcond.isEmpty()) { warning("Condition field is empty.", this); return false; }
            break;
        }
        case 12: case 13: case 14: case 15:
            // already handled column field
            break;
        case 16: case 17:
        {
            if (fcond.isEmpty()) { warning("Lower limit field is empty.", this); return false; }
            else if (scond.isEmpty()) { warning("Upper limit field is empty.", this); return false; }
            else if (!isNumber(fcond) && !isNumber(scond)) { warning("Lower/Upper fields need to be a digit.", this); return false; }
            break;
        }
        }
    }
    else if (selectIntoClause->isChecked()) {
        if (name.isEmpty()) { warning("New Table Name field is empty.", this); return false; }
    }
    return true;
}

QString QueryForm::generateExecutionPlan()
{
    QString plan;
    // some syntactic/semantic validations included
    SystemCatalog *sysCat = &SystemCatalog::getInstance();

    QStringList attributes = attrInput->text().simplified().split(",");
    for (auto& i : attributes) i = i.trimmed(); // clean spaces

    // FROM: get table information
    QString tableName = tableInput->text().trimmed();
    auto table = sysCat->find(tableName);
    // If table not found in schema
    if (table == sysCat->end()) {
        warning(tr("Table: %1 not found in schema.").arg(tableName), this);
        return QString();
    }
    // SELECT - SelectAll: '*' case
    if (attributes.size() == 1) {
        if (attributes.contains("*"))
            plan.append((char)Types::SelectAll);
        else
            plan.append((char)Types::SelectCustom);
    }
    // SELECT - SelectCustom: custom attributes
    else {
        if (attributes.size() > 1) {
            if (attributes.contains("") || attributes.contains("*")) {
                warning("Attributes field: Bad syntax.", this);
                return QString();
            }
            else
                plan.append((char)Types::SelectCustom);
        }
        // Seek for specified attribute(s) 'position'
        // while (table != sysCat->end() && table.key() == tableName) {
        //     const SystemCatalog::attrMeta& attrInfo = table.value();
        //     for (qsizetype i = 0; i < attributes.size() || attrPositions.size() != attributes.size(); i++)
        //         if (attrInfo.attributeName == attributes[i])
        //             attrPositions.append(attrInfo.position);
        //     ++table;
        // }
    }
    // INTO:
    if (selectIntoClause->isChecked())
        plan.append((char)Types::SelectInto);
    // WHERE:
    if (whereClause->isChecked())
        plan.append((char)Types::Where);
    // End of executionPlan
    return plan;
}

bool QueryForm::executeExecutionPlan(const QString& plan)
{
    // SELECT * FROM ...
    if (plan == "A") {
        QString tableName = tableInput->text().trimmed();
        return exec(tableName);
    }
    // SELECT * INTO ... FROM ...
    else if (plan == "AI") {
        QString newTableName = newTableInput->text().trimmed();
        QString tableName = tableInput->text().trimmed();

        return exec(newTableName, tableName);
    }
    // SELECT * FROM ... WHERE ...
    else if (plan == "AW") {
        QString tableName = tableInput->text().trimmed();
        QString field = columnInput->text().trimmed();
        int optor = comparisonOperator->currentIndex();
        switch (optor) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9: case 10: case 11:
        {
            QString condition = firstCond->text().trimmed();
            return exec(tableName, field, optor, condition);
        }
        case 12: case 13: case 14: case 15:
        {
            QString condition1 = firstCond->text().trimmed();
            QString condition2 = secondCond->text().trimmed();
            return exec(tableName, field, optor, condition1, condition2);
        }
        case 16: case 17:
        {
            return exec(tableName, field, optor);
        }
        }
    }
    // SELECT * INTO ... FROM ... WHERE ...
    else if (plan == "AIW") {
        QString tableName = tableInput->text().trimmed();
        QString newTableName = newTableInput->text().trimmed();
        QString field = columnInput->text().trimmed();
        int optor = comparisonOperator->currentIndex();
        switch (optor) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9: case 10: case 11:
        {
            QString condition = firstCond->text().trimmed();
            return exec(newTableName, tableName, field, optor, condition);
        }
        case 12: case 13: case 14: case 15:
        {
            QString condition1 = firstCond->text().trimmed();
            QString condition2 = secondCond->text().trimmed();
            return exec(newTableName, tableName, field, optor, condition1, condition2);
        }
        case 16: case 17:
        {
            return exec(newTableName, tableName, field, optor);
        }
        }
    }

    // SELECT ... FROM ...
    else if (plan == "C") {
        QStringList attributes = attrInput->text().trimmed().split(",");
        for (auto& i : attributes) i = i.trimmed(); // clean spaces
        QString tableName = tableInput->text().trimmed();
        return exec(attributes, tableName);
    }
    // SELECT ... INTO ... FROM ...
    else if (plan == "CI") {
        QStringList attributes = attrInput->text().trimmed().split(",");
        for (auto& i : attributes) i = i.trimmed(); // clean spaces
        QString newTableName = newTableInput->text().trimmed();
        QString tableName = tableInput->text().trimmed();
        return exec(attributes, newTableName, tableName);
    }
    // SELECT ... FROM ... WHERE ...
    else if (plan == "CW") {
        QStringList attributes = attrInput->text().trimmed().split(",");
        for (auto& i : attributes) i = i.trimmed(); // clean spaces
        QString tableName = tableInput->text().trimmed();
        QString field = columnInput->text().trimmed();
        int optor = comparisonOperator->currentIndex();
        switch (optor) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9: case 10: case 11:
        {
            QString condition = firstCond->text().trimmed();
            return exec(attributes, tableName, field, optor, condition);
        }
        case 12: case 13: case 14: case 15:
        {
            QString condition1 = firstCond->text().trimmed();
            QString condition2 = secondCond->text().trimmed();
            return exec(attributes, tableName, field, optor, condition1, condition2);
        }
        case 16: case 17:
        {
            return exec(attributes, tableName, field, optor);
        }
        }
    }
    // SELECT ... INTO ... FROM ... WHERE ...
    else if (plan == "CIW") {
        QStringList attributes = attrInput->text().trimmed().split(",");
        for (auto& i : attributes) i = i.trimmed(); // clean spaces
        QString newTableName = newTableInput->text().trimmed();
        QString tableName = tableInput->text().trimmed();
        QString field = columnInput->text().trimmed();
        int optor = comparisonOperator->currentIndex();
        switch (optor) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9: case 10: case 11:
        {
            QString condition = firstCond->text().trimmed();
            return exec(attributes, newTableName, tableName, field, optor, condition);
        }
        case 12: case 13: case 14: case 15:
        {
            QString condition1 = firstCond->text().trimmed();
            QString condition2 = secondCond->text().trimmed();
            return exec(attributes, newTableName, tableName, field, optor, condition1, condition2);
        }
        case 16: case 17:
        {
            return exec(attributes, newTableName, tableName, field, optor);
        }
        }
    }
    else {
        warning(tr("Plan: %1 invalid. generateExecutionPlan() failed.").arg(plan));
        return false;
    }
    return false;
}

void QueryForm::insertRecord()
{
    // Exclude character '#'
}

void QueryForm::deleteRecord()
{

}

void QueryForm::runQuery()
{
    if (!validateForm()) return;
    // Clear tableWidget for future queries
    else if (tableWidget->rowCount() != 0 ||
             tableWidget->columnCount() != 0) {
        tableWidget->setRowCount(0);
        tableWidget->setColumnCount(0);
    }
    QString plan = generateExecutionPlan();
    if (plan.isEmpty()) return;
    // Define query templates, plan clauses' order MATTER
    if (executeExecutionPlan(plan)) {
        emit refreshUi();
    }
}

void QueryForm::clear()
{
    // Clear all
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(0);
    attrInput->clear();
    tableInput->clear();
    newTableInput->clear();
    columnInput->clear();
    firstCond->clear();
    secondCond->clear();
}

bool QueryForm::exec(const QString &tableName)
{
    SystemCatalog *sysCat = &SystemCatalog::getInstance();
    // sysCat->getDiskController()->readBlock();
    QString path(sysCat->getDbDirPath() + "/" + tableName + ".txt");
    QFile tableFile(path);
    tableFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<SystemCatalog::attrMeta> meta = sysCat->values(tableName);
    std::reverse(meta.begin(), meta.end());

    // Show
    QStringList headers;
    for (const auto& a : meta) headers.append(a.attributeName);
    tableWidget->setColumnCount(headers.size());
    tableWidget->setHorizontalHeaderLabels(headers);
    QTextStream in(&tableFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList dataList = line.split("#");
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);
        for (int i = 0; i < dataList.size(); i++) {
            QTableWidgetItem *item = new QTableWidgetItem(dataList[i]);
            tableWidget->setItem(row, i, item);
        }
    }
    // char c;
    // std::string line;
    // falta completar algoritmo
    // while (std::getline(tableFile, line)) {
    //     std::stringstream ss(line);
    //     // get position pointers
    //     int i = 0;
    //     int *posValues = new int[headers.size()];
    //     posValues[0] = 0; i++;
    //     while (ss.get(c)) {
    //         if (c == '#') {
    //             posValues[i] = ss.tellg();
    //             i++;
    //         }
    //     }
    //     ss.seekg(0, ss.beg);
    //     for (int j = 0; j < headers.size(); ++j) {
    //         cout <<
    //     }
    //     delete [] posValues;
    // }
    tableFile.close();
    return true;
}

bool QueryForm::exec(const QString &newTableName, const QString &tableName)
{
    if (tableName == newTableName) {
        warning(tr("Table: %1 already exists.").arg(tableName));
        return false;
    }

    SystemCatalog *sysCat = &SystemCatalog::getInstance();
    QString tablePath(sysCat->getDbDirPath() + "/" + tableName + ".txt");
    QFile tableFile(tablePath);
    tableFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<SystemCatalog::attrMeta> meta = sysCat->values(tableName);
    std::reverse(meta.begin(), meta.end());

    // Write schema
    for (const auto& m : meta) {
        sysCat->insertTableMetadata(newTableName, m);
    }
    sysCat->writeToSchema(newTableName);

    QString newTablePath(sysCat->getDbDirPath() + "/" + newTableName + ".txt");
    QFile newTableToCreate(newTablePath);
    newTableToCreate.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream in(&tableFile);
    QTextStream out(&newTableToCreate);

    QStringList headers;
    for (const auto& a : meta) headers.append(a.attributeName);
    tableWidget->setColumnCount(headers.size());
    tableWidget->setHorizontalHeaderLabels(headers);
    while (!in.atEnd()) {
        QString line = in.readLine();
        out << line << "\n";
        // Show
        QStringList dataList = line.split("#");
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);
        for (int i = 0; i < dataList.size(); i++) {
            QTableWidgetItem *item = new QTableWidgetItem(dataList[i]);
            tableWidget->setItem(row, i, item);
        }
    }

    tableFile.close();
    newTableToCreate.close();
    return true;
}

bool QueryForm::exec(const QString &tableName, const QString &field,
                     int optor, const QString &condition)
{
    SystemCatalog *sysCat = &SystemCatalog::getInstance();
    QString path(sysCat->getDbDirPath() + "/" + tableName + ".txt");
    QFile tableFile(path);
    tableFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QList<SystemCatalog::attrMeta> meta = sysCat->values(tableName);
    std::reverse(meta.begin(), meta.end());

    int fieldPosition = 0;
    char fieldType = ' ';
    for (const auto& f : meta) {
        if (f.attributeName == field) {
            fieldPosition = f.position;
            fieldType = f.type;
            break;
        }
    }

    // Handle data type mismatch
    if (optor == 0 || optor == 1 || optor == 4 || optor == 5) {
        if (fieldType != 'i' || fieldType != 'f' || fieldType != 'd') {
            warning("Incompatible data types, comparison is not possible.", this);
            return false;
        }
    }

    // Show
    QStringList headers;
    for (const auto& a : meta) headers.append(a.attributeName);
    tableWidget->setColumnCount(headers.size());
    tableWidget->setHorizontalHeaderLabels(headers);
    QTextStream in(&tableFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList dataList = line.split("#");
        // Manage operator type
        switch (optor) {
        // toDouble casting manages all numeric types...
        case 0: // <
        {
            if (dataList[fieldPosition].toDouble() < condition.toDouble())
                break;
            else continue;
        }
        case 1: // >
        {
            if (dataList[fieldPosition].toDouble() > condition.toDouble())
                break;
            else continue;
        }
        case 2: // isNotEqualTo
        {
            if (dataList[fieldPosition] != condition)
                break;
            else continue;
        }
        case 3: // isEqualTo
        {
            if (dataList[fieldPosition] == condition)
                break;
            else continue;
        }
        case 4: // <=
        {
            if (dataList[fieldPosition].toDouble() <= condition.toDouble())
                break;
            else continue;
        }
        case 5: // >=
        {
            if (dataList[fieldPosition].toDouble() >= condition.toDouble())
                break;
            else continue;
        }
        case 6: // Contains
        {
            if (dataList[fieldPosition].contains(condition))
                break;
            else continue;
        }
        case 7: // BeginsWith
        {
            if (dataList[fieldPosition].startsWith(condition))
                break;
            else continue;
        }
        case 8: // EndsWith
        {
            if (dataList[fieldPosition].endsWith(condition))
                break;
            else continue;
        }
        case 9: // DoesNotContain
        {
            if (!dataList[fieldPosition].contains(condition))
                break;
            else continue;
        }
        case 10: // DoesNotBeginWith
        {
            if (!dataList[fieldPosition].startsWith(condition))
                break;
            else continue;
        }
        case 11: // DoesNotEndWith
        {
            if (!dataList[fieldPosition].endsWith(condition))
                break;
            else continue;
        }
        }
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);
        for (int i = 0; i < dataList.size(); i++) {
            QTableWidgetItem *item = new QTableWidgetItem(dataList[i]);
            tableWidget->setItem(row, i, item);
        }
    }
    return true;
}

bool QueryForm::exec(const QString &tableName, const QString &field, int optor, const QString &condition1, const QString &condition2)
{
    return true;
}

bool QueryForm::exec(const QString &tableName, const QString &field, int optor)
{
    return true;
}

bool QueryForm::exec(const QString &newTableName, const QString &tableName, const QString &field, int optor, const QString &condition)
{
    return true;
}

bool QueryForm::exec(const QString &newTableName, const QString &tableName, const QString &field, int optor, const QString &condition1, const QString &condition2)
{
    return true;
}

bool QueryForm::exec(const QString &newTableName, const QString &tableName, const QString &field, int optor)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &tableName)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &newTableName, const QString &tableName)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &tableName, const QString &field, int optor, const QString &condition)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &tableName, const QString &field, int optor, const QString &condition1, const QString &condition2)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &tableName, const QString &field, int optor)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &newTableName, const QString &tableName, const QString &field, int optor, const QString &condition)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &newTableName, const QString &tableName, const QString &field, int optor, const QString &condition1, const QString &condition2)
{
    return true;
}

bool QueryForm::exec(const QStringList &attributes, const QString &newTableName, const QString &tableName, const QString &field, int optor)
{
    return true;
}

void QueryForm::createActions()
{
    columnInput->setEnabled(false);
    firstCond->setEnabled(false);
    secondCond->setEnabled(false);
    secondCond->setVisible(false);

    connect(whereClause, &QCheckBox::stateChanged, this, [this](int state) {
        bool ret = state == Qt::Checked ? true : false;
        columnInput->setEnabled(ret);
        comparisonOperator->setEnabled(ret);
        firstCond->setEnabled(ret);
        secondCond->setEnabled(ret);
        columnInput->clear();
        firstCond->clear();
        secondCond->clear();
    });

    connect(comparisonOperator, &QComboBox::currentIndexChanged, this, [this](int index) {
        switch (index) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9: case 10: case 11:
        {
            firstCond->clear();
            firstCond->setVisible(true);
            secondCond->clear();
            secondCond->setVisible(false);
            break;
        }
        case 12: case 13: case 14: case 15:
        {
            firstCond->clear();
            firstCond->setVisible(false);
            secondCond->clear();
            secondCond->setVisible(false);
            break;
        }
        case 16: case 17:
        {
            firstCond->clear();
            firstCond->setVisible(true);
            secondCond->clear();
            secondCond->setVisible(true);
            break;
        }
        }
    });

    newTableInput->setEnabled(false);
    connect(selectIntoClause, &QCheckBox::stateChanged, this, [this](int state) {
        newTableInput->setEnabled(state == Qt::Checked ? true : false);
    });

    connect(ui->runButton, &QPushButton::clicked, this, &QueryForm::runQuery);
    connect(ui->clearButton, &QPushButton::clicked, this, &QueryForm::clear);
    // tabWidget->centralwidget->Megatron
    connect(this, SIGNAL(refreshUi()), parent()->parent()->parent(), SLOT(loadTableTree()));
}
