#ifndef MEGATRON_H
#define MEGATRON_H

#include <QMainWindow>
#include <QDir>
#include <QString>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSharedPointer>

#include "systemcatalog.h"
#include "diskcontroller.h"

class QTabWidget;
class QTreeWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class Megatron;
}
QT_END_NAMESPACE

class Megatron : public QMainWindow
{
    Q_OBJECT

public:
    Megatron(QWidget *parent = nullptr, QSharedPointer<DiskController> control = nullptr);
    ~Megatron();

signals:
    void messageVisible(bool);

private slots:
    QWidget* createOpenMessage(QWidget *);
    void handleOpenMessage(bool);
    void loadTableTree();
    void createRelation(const QString &, const QString &);   // Using file
    void createRelation();                                   // From scratch
    void createQuery();
    void deleteTabRequested(int);
    void switchTabs(int);

private:
    Ui::Megatron *ui;
    QDir dbDir;
    QTabWidget *tabWidget;
    QTreeWidget *tableTreeWidget;
    SystemCatalog *sysCat;
    QSharedPointer<DiskController> controller;

    void createActions();
    void updateActions();
    // friend bool is_empty(std::fstream &);
};
#endif // MEGATRON_H
