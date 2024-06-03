#ifndef MEGATRON_H
#define MEGATRON_H

#include <QMainWindow>
#include <QDir>
#include <QString>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSharedPointer>

#include <database.h>

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
    Megatron(QWidget *parent = nullptr, QString diskPath = QString(),
             QSharedPointer<Storage::DiskController> control = nullptr);
    ~Megatron();

signals:
    void messageVisible(bool);

private slots:
    QWidget* createOpenMessage(QWidget *);
    void handleOpenMessage(bool);
    void loadTableTree();
    void createTable();
    void createQuery();
    void deleteTabRequested(int);
    void switchTabs(int);

private:
    Ui::Megatron *ui;

    QTabWidget *tabWidget;
    QTreeWidget *tableTreeWidget;
    Core::Database database;

    void createActions();
    void updateActions();

};
#endif // MEGATRON_H
