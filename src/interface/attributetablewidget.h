#ifndef ATTRIBUTETABLEWIDGET_H
#define ATTRIBUTETABLEWIDGET_H

#include <QObject>
#include <QTableWidget>
#include <QWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QList>
#include <QVariant>
#include <QIntValidator>
#include <QPainter>
#include <QHeaderView>
#include <megatron_types.h>
#include <megatron_structs.h>

namespace Ui {
class AttributeTableWidget;

// Data to be sent to NewRelation Form via attributePackage
// attributeName, dataType, columnType, constraint, defaultValue, nullable, unsigned, ai, comment
// missing attribute properties will be calculated/obtained later
using attributePackage = QList<std::tuple<QString, Types::DataType, QString, Types::KeyConstraintType, QString, bool, bool, bool, QString>>;

// attributeName, indexName, keyConstraint, indexType, indexComment
using indexPackage = QList<std::tuple<QString, QString, Types::KeyConstraintType, Types::IndexType, QString>>;

}

class AttributeTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    AttributeTableWidget(QWidget* parent = nullptr);

    Ui::attributePackage tableAttributes();
    Ui::indexPackage tableIndexes();

public slots:
    void addRow(const QString& = QString());
    void removeRow();
    void clearContents();

protected:
    void paintEvent(QPaintEvent *event) override {
        QTableWidget::paintEvent(event);

        QFont font = this->font();
        font.setPointSize(13);
        this->setFont(font);
    }

private:
    const QList<QPair<QString, QVariant>> dataTypeComboBoxItems = {
        {"", QVariant()},
        {"TINYINT", QVariant::fromValue(Types::DataType::TinyInt)},
        {"SMALLINT", QVariant::fromValue(Types::DataType::SmallInt)},
        {"INT", QVariant::fromValue(Types::DataType::Int)},
        {"BIGINT", QVariant::fromValue(Types::DataType::BigInt)},
        {"FLOAT", QVariant::fromValue(Types::DataType::Float)},
        {"DOUBLE", QVariant::fromValue(Types::DataType::Double)},
        {"BOOL", QVariant::fromValue(Types::DataType::Bool)},
        {"ENUM", QVariant::fromValue(Types::DataType::Enum)},
        {"CHAR", QVariant::fromValue(Types::DataType::Char)},
        {"VARCHAR", QVariant::fromValue(Types::DataType::Varchar)}
    };
    const QList<QPair<QString, QVariant>> indexOptionComboBoxItems = {
        {"", QVariant()},
        {"PRIMARY", QVariant::fromValue(Types::KeyConstraintType::Primary)},
        {"UNIQUE", QVariant::fromValue(Types::KeyConstraintType::Unique)},
        {"INDEX", QVariant::fromValue(Types::KeyConstraintType::Index)}
    };

    QVariant getItemDelegateContent(int, int, bool) const;

    Ui::attributePackage ap;
    Ui::indexPackage ip;

};

#endif // ATTRIBUTETABLEWIDGET_H
