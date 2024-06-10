#include "megatron.h"
#include "./ui_megatron.h"
#include "newrelation.h"
#include "queryform.h"

#include <QDebug>
#include <QScrollArea>
#include <QTabWidget>
#include <QInputDialog>

Megatron::Megatron(QWidget *parent, QString diskPath, QSharedPointer<Storage::DiskController> control,
                   bool firstInit)
    : QMainWindow(parent), ui(new Ui::Megatron)
{
    ui->setupUi(this);
    QString storagePath(diskPath + "/" + "storage.bin");
    database = Core::Database(control, storagePath, firstInit);

    tabWidget = ui->tabWidget;
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);
    tableTreeWidget = ui->treeWidget;

    tabWidget->setVisible(false);
    tableTreeWidget->setVisible(true);
    loadTableTree();

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBar->insertWidget(ui->actionExit, spacer);

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

QWidget* Megatron::createOpenMessage(QWidget *parent)
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
                "margin-bottom:12px; "
                "margin-left:0px; "
                "margin-right:0px; "
                "-qt-block-indent:0; text-indent:0px;\">"
                "Table &gt; New Table (Ctrl + N)"
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
                "Select a Table from the Left Tree"
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
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relations = sc->listRelationKeys();
    for (auto i = relations.begin(), end = relations.end(); i != end; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(tableTreeWidget);
        QFont font;
        font.setPointSize(11);
        item->setFont(0, font);
        item->setText(0, *i);
    }
}

void Megatron::createTable()
{
    NewRelation dialog(this);
    Core::RelationInput response;
    if (dialog.exec() == QDialog::Accepted)
    {
        response = dialog.getDataPackage();
        // send to database
        switch (database.createRelation(response))
        {
        case Types::Return::Success:
        {
            statusBar()->showMessage(tr("Loaded Relation: %1 successfully.").arg(response.relationName));
            // add new relationForm Widget to tree
            if (!tableTreeWidget->isVisible()) tableTreeWidget->setVisible(true);
            ui->actionNewQuery->setEnabled(true);
            loadTableTree();
        }
        case Types::Return::DuplicateError:
        {
            QMessageBox::warning(this, "Error", tr("Relation: %1 already exists. "
                                                   "Change the name of the relation and try again.").arg(response.relationName));
            return;
        }
        case Types::Return::OpenError:
            QMessageBox::warning(this, "Error", tr("Data File: %1 could not be opened.").arg(response.dataPath));
            return;
        case Types::Return::ParseError:
            QMessageBox::warning(this, "Error", tr("Data File: %1 could not be parsed correctly.").arg(response.dataPath));
            return;
        }
    }
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
    ui->actionNewTable->setShortcut(QKeySequence::New);
    connect(ui->actionNewTable, &QAction::triggered, this, &Megatron::createTable);

    // ui->actionOpenTable->setShortcut(QKeySequence::Open);
    // connect(ui->actionOpenTable, &QAction::triggered, this, &Megatron::openTable);

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
