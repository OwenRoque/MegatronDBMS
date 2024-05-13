#include "megatron.h"
#include "./ui_megatron.h"
#include "opentable.h"
#include "queryform.h"

#include <QDebug>
#include <QScrollArea>
#include <QTabWidget>
#include <QInputDialog>
#include <QTimer>

#include <sstream>

// bool is_empty(std::ifstream& pFile);

Megatron::Megatron(QWidget *parent, QSharedPointer<Storage::DiskController> control)
    : QMainWindow(parent), controller(control)
    , ui(new Ui::Megatron)
{
    ui->setupUi(this);

    QString dbPath(QCoreApplication::applicationDirPath() + "/db");
    dbDir = QDir(dbPath);
    tabWidget = ui->tabWidget;
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);
    tableTreeWidget = ui->treeWidget;
    sysCat = &SystemCatalog::getInstance(dbDir.absolutePath(), controller);

    // controller->readBlock();
    // controller->moveArmTo(0, 2);

    tabWidget->setVisible(false);
    tableTreeWidget->setVisible(false);
    // Load relations from schema in TreeWidget
    if (sysCat->initSchema()) {
        tableTreeWidget->setVisible(true);
        loadTableTree();
    }
    else {
        ui->actionNewQuery->setEnabled(false);
    }
    setCentralWidget(ui->centralwidget);
    // show OpenMessage default
    QWidget *openMessage = createOpenMessage(centralWidget());
    centralWidget()->layout()->addWidget(openMessage);
    createActions();
}

Megatron::~Megatron()
{
    delete ui;
}

QWidget * Megatron::createOpenMessage(QWidget *parent)
{
    // GridLayout
    QWidget *gridContainer = new QWidget(parent);
    gridContainer->setObjectName("OpenMessage");
    QGridLayout *gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

    QLabel *titleLabel = new QLabel("Create a Table/Query", gridContainer);
    titleLabel->setFont(QFont("Arial", 24));
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *menuLabel = new QLabel(gridContainer);
    QFont font = menuLabel->font();
    font.setPointSize(15);
    menuLabel->setFont(font);
    menuLabel->setText(
        "<html>"
        "<body>"
            "<ul style="
            "\"margin-top: 0px; "
            "margin-bottom: 0px; "
            "margin-left: 0px; "
            "margin-right: 0px; "
            "-qt-list-indent: 1;\">"
                "<li style="
                "\" margin-top:12px; "
                "margin-bottom:0px; "
                "margin-left:0px; "
                "margin-right:0px; "
                "-qt-block-indent:0; text-indent:0px;\">"
                "Table &gt; New Table (Ctrl + N)"
                "</li>"
                "<li style="
                "\" margin-top:0px; "
                "margin-bottom:12px; "
                "margin-left:0px; "
                "margin-right:0px; "
                "-qt-block-indent:0; "
                "text-indent:0px;\">"
                "Table &gt; Open Table (Ctrl + O)"
                "</li>"
                "<li style="
                "\" margin-top:12px; "
                "margin-bottom:0px; "
                "margin-left:0px; "
                "margin-right:0px; "
                "-qt-block-indent:0; "
                "text-indent:0px;\">"
                "Create a new query (Ctrl + Q)"
                "</li>"
                "<li style="
                "\" margin-top:0px; "
                "margin-bottom:12px; "
                "margin-left:0px; "
                "margin-right:0px; "
                "-qt-block-indent:0; "
                "text-indent:0px;\">"
                "Select a table from the left Tree"
                "</li>"
            "</ul>"
        "</body>"
        "</html>");
    menuLabel->setTextFormat(Qt::RichText);
    menuLabel->setAlignment(Qt::AlignLeft);

    QFrame *line = new QFrame(gridContainer);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);

    gridLayout->addWidget(titleLabel, 1, 1, 1, 1);
    gridLayout->addWidget(line, 2, 1, 1, 1);
    gridLayout->addWidget(menuLabel, 3, 1, 1, 1);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    gridLayout->addItem(verticalSpacer, 0, 1, 1, 1);

    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer, 1, 0, 1, 1);

    QSpacerItem *horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer_2, 1, 2, 1, 1);

    QSpacerItem *verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);
    gridLayout->addItem(verticalSpacer_2, 4, 1, 1, 1);

    gridContainer->setLayout(gridLayout);
    // QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    // if (mainLayout)
    //     mainLayout->addWidget(gridContainer);
    return gridContainer;
}

void Megatron::handleOpenMessage(bool ret)
{
    // It's always 3rd item since treeWidget & tabWidget are at 1st and 2nd pos. respectively
    QWidget *widget = ui->centralwidget->layout()->itemAt(2)->widget();
    if (widget)
        if (widget->objectName() == "OpenMessage" && widget->isVisible() == !ret)
            widget->setVisible(ret);
}

void Megatron::loadTableTree()
{
    tableTreeWidget->clear();
    QSet<QString> tableNames = sysCat->getTableNames();
    for (auto i = tableNames.cbegin(), end = tableNames.cend(); i != end; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(tableTreeWidget);
        QFont font;
        font.setPointSize(11);
        item->setFont(0, font);
        item->setText(0, *i);
    }
}

// bool is_empty(std::ifstream& pFile)
// {
//     return pFile.peek() == std::ifstream::traits_type::eof();
// }

void Megatron::createRelation(const QString &dt, const QString &sch)
{
    // Validate if relation already exists
    // Check if it exists in treeWdgt
    QFileInfo dataInfo(dt);
    QString relName = dataInfo.baseName();
    auto duplicateTable = [](QTreeWidget* tw, const QString& v) {
        for (int i = 0; i < tw->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = tw->topLevelItem(i);
            if (item) {
                QString value = item->text(0);
                if (value == v)
                    return true;
            }
        }
        return false;
    };
    if (duplicateTable(tableTreeWidget, relName))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Error");
        msgBox.setInformativeText(tr("Relation: %1 already exists. "
                                     "Change filename and try again.").arg(relName));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    // Read header (attribute names) from newData file
    QFile newData(dt);
    if (!newData.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage(tr("Error while opening Data file: %1").arg(dt));
        return;
    }
    QTextStream in(&newData);
    QString header = in.readLine();

    // Parse new schema file, handle responses
    Types::Return res = sysCat->parseSchemaPath(relName, header, sch);
    switch (res) {
    case Types::Success:
    {
        // Write to schemaPath
        sysCat->writeToSchema(relName);
        break;
    }
    case Types::OpenError:
    {
        statusBar()->showMessage(tr("Error while opening Schema file: %1").arg(sch));
        newData.close();
        return;
    }
    case Types::ParseError:
    {
        statusBar()->showMessage(tr("Error while parsing Schema file: %1").arg(sch));
        newData.close();
        return;
    }
    }

    // Write dataFile after saving its schema
    QFile newFile(dbDir.filePath(relName + ".txt"));
    newFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream dout(&newFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // parse record algorithm
        std::stringstream inLine(line.toStdString());
        QStringList values;
        bool insideQuotes = false;
        QString word;
        char c;
        while (inLine.get(c)) {
            // qDebug() << word;
            if (c == ',') {
                // If no content
                if (word.isEmpty()) {
                    // qDebug() << "Nan";
                    values.append("");
                }
                else if (insideQuotes)
                    word+=c;
                else {
                    values.append(word);
                    word.clear();
                }
            }
            // not so sure about some (unlikely) cases like  ""hello", he said"
            else if (c == '"') {
                if (word.isEmpty())
                    insideQuotes = true;
                else {
                    char p = inLine.peek();
                    // If data ends
                    if (p == ',') {
                        values.append(word);
                        word.clear();
                        insideQuotes = false;
                        inLine.seekg(1, std::ios_base::cur);
                    }
                    // Then it's a '"' inside commillas
                    else
                        word+=c;
                }
            }
            else word+=c;
        }
        // handle last field
        if (!word.isEmpty())
            values.append(word);

        for (qsizetype i = 0; i < values.size(); ++i) {
            const auto &w = values.at(i);
            if (i == values.size() - 1)
                dout << w << Qt::endl;
            else dout << w << "#";
        }
    }
    newData.close();
    newFile.close();

    statusBar()->showMessage(tr("Loaded Relation: %1 successfully.").arg(relName));
    // add new relationForm Widget to tree
    if (!tableTreeWidget->isVisible()) tableTreeWidget->setVisible(true);
    ui->actionNewQuery->setEnabled(true);
    loadTableTree();
}

void Megatron::createRelation()
{
    qDebug() << "Display table to fill in attributes, then add new relationForm Widget to tree, "
                "empty table and query default";
    //
}

void Megatron::createQuery()
{
    bool ok;
    QString label = QInputDialog::getText(this, tr("New Query"),
        tr("Query Name:"), QLineEdit::Normal, "", &ok);
    if (ok) {
        if (label.isEmpty()) label = "Untitled";
        label += " - Query";
        if (!tabWidget->isVisible()) tabWidget->setVisible(true);
        QWidget *query = new QueryForm(tabWidget);
        tabWidget->addTab(query, label);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
        tabWidget->setTabToolTip(tabWidget->currentIndex(), label);
        // To run query from menu option
        QueryForm* currQuery = qobject_cast<QueryForm *>(tabWidget->currentWidget());
        ui->actionRunSelected->setEnabled(true);
        connect(ui->actionRunSelected, &QAction::triggered, currQuery, &QueryForm::runQuery);
        emit messageVisible(false);
    }
}

void Megatron::deleteTabRequested(int index)
{
    tabWidget->removeTab(index);
    if (tabWidget->count() == 0) {
        tabWidget->setVisible(false);
        emit messageVisible(true);
    }
}

void Megatron::switchTabs(int index)
{
    QueryForm* currQuery = qobject_cast<QueryForm *>(tabWidget->currentWidget());
    if (!currQuery)
        ui->actionRunSelected->setEnabled(false);
}

void Megatron::createActions()
{
    ui->actionOpenTable->setShortcut(QKeySequence::Open);
    connect(ui->actionOpenTable, &QAction::triggered, this, [this]() {
        OpenTable dialog(this);
        QString dataFile, schemaFile;
        if (dialog.exec() == QDialog::Accepted) {
            dataFile = dialog.getDataPath();
            schemaFile = dialog.getSchemaPath();
            // qDebug() << dataPath; qDebug() << schemaPath;
            createRelation(dataFile, schemaFile);
        }
    });

    ui->actionNewTable->setShortcut(QKeySequence::New);
    connect(ui->actionNewTable, &QAction::triggered, this, [this](){ createRelation(); });

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &Megatron::deleteTabRequested);
    connect(this, &Megatron::messageVisible, this, &Megatron::handleOpenMessage);

    connect(tabWidget, &QTabWidget::currentChanged, this, &Megatron::switchTabs);

    connect(ui->actionExit, &QAction::triggered, this, &Megatron::close);
    ui->actionExit->setShortcut(tr("Esc"));

    connect(ui->actionNewQuery, &QAction::triggered, this, &Megatron::createQuery);
    ui->actionNewQuery->setShortcut(tr("Ctrl+Q"));

    ui->actionRunSelected->setShortcut(tr("Ctrl+R"));
    ui->actionRunSelected->setEnabled(false);
}

void Megatron::updateActions()
{
    // Future actions may require updating, none atm
}
