#ifndef NEWRELATION_H
#define NEWRELATION_H

#include <QTableWidgetItem>
#include <QDialog>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QVariant>
#include <QStandardItemModel>
#include <QSet>
#include <megatron_types.h>
#include <megatron_structs.h>
#include "attributetablewidget.h"

namespace Ui {
class NewRelation;
}

class NewRelation : public QDialog
{
    Q_OBJECT

public:
    explicit NewRelation(QWidget *parent = nullptr);
    ~NewRelation();
    Core::RelationInput getDataPackage() const;

private slots:
    // for file upload
    bool loadFile(const QString&);
    void open();
    // general validation
    bool validate();

private:
    Ui::NewRelation *ui;
    AttributeTableWidget *tableWidget;

    void error();
    void success(const QString&, const QStringList&);

    // Data to be sent to Database via DataPackage
    QString relationName;
    QString dataPath;
    // attributeName, dataType, columnType, maxCharLen, defaultValue, nullable, unsigned, ai, constraint, comment
    // missing attribute properties will be calculated internally, or are already included in this struct
    QList<std::tuple<QString, Types::DataType, QString, quint16, QString,
                     bool, bool, bool, Types::KeyConstraintType, QString>> attributes;
    // indexName, attributeName, indexType, idxProperties
    QList<std::tuple<QString, QString, Types::IndexType, Core::IndexProperties>> indexes;
    Types::FileOrganization fileOrg;
    Types::RecordFormat recFormat;
    Types::Charset charset;
};

#endif // NEWRELATION_H
