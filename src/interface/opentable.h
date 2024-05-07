#ifndef OPENTABLE_H
#define OPENTABLE_H

#include <QDialog>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLabel>

namespace Ui {
class OpenTable;
}

class OpenTable : public QDialog
{
    Q_OBJECT

public:
    explicit OpenTable(QWidget *parent = nullptr);
    ~OpenTable();
    QString getDataPath() const;
    QString getSchemaPath() const;

private slots:
    bool loadFile(const QString &, bool);
    void open(bool);

private:
    Ui::OpenTable *ui;
    void error(bool);
    void success(bool, const QString &);
    bool filesLoaded() const;
    QString dataPath;
    QString schemaPath;
};

#endif // OPENTABLE_H
