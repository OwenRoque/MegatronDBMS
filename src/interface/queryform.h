#ifndef QUERYFORM_H
#define QUERYFORM_H

#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>
#include <QWidget>

namespace Ui {
class QueryForm;
}

class QueryForm : public QWidget
{
    Q_OBJECT

public:
    explicit QueryForm(QWidget *parent = nullptr);
    ~QueryForm();
    // To display MessageBox::warning
    void warning(const QString& message, QWidget* parent = nullptr);
    // Just superficial validation
    bool validateForm();
    // Generate sequence to follow
    QString generateExecutionPlan();
    bool executeExecutionPlan(const QString& plan);

signals:
    void refreshUi();

private slots:
    void insertRecord();
    void deleteRecord();
    void clear();

public slots:
    // query logic
    void runQuery();

private:
    Ui::QueryForm *ui;
    QLineEdit* attrInput;
    QLineEdit* tableInput;
    QCheckBox* whereClause;
    QLineEdit* columnInput;
    QComboBox* comparisonOperator;
    QLineEdit* firstCond;
    QLineEdit* secondCond;
    QCheckBox* selectIntoClause;
    QLineEdit* newTableInput;
    QTableWidget* tableWidget;

    // Actual execution according to response
    bool exec(const QString &tableName);
    bool exec(const QString &newTableName, const QString &tableName);
    bool exec(const QString &tableName, const QString &field, int optor,
              const QString &condition);
    bool exec(const QString &tableName, const QString &field, int optor,
              const QString &condition1, const QString &condition2);
    bool exec(const QString &tableName, const QString &field, int optor);
    bool exec(const QString &newTableName, const QString &tableName,
              const QString &field, int optor, const QString &condition);
    bool exec(const QString &newTableName, const QString &tableName,
              const QString &field, int optor, const QString &condition1,
              const QString &condition2);
    bool exec(const QString &newTableName, const QString &tableName,
              const QString &field, int optor);

    bool exec(const QStringList &attributes, const QString &tableName);
    bool exec(const QStringList &attributes, const QString &newTableName,
              const QString &tableName);
    bool exec(const QStringList &attributes, const QString &tableName,
              const QString &field, int optor, const QString &condition);
    bool exec(const QStringList &attributes, const QString &tableName,
              const QString &field, int optor, const QString &condition1,
              const QString &condition2);
    bool exec(const QStringList &attributes, const QString &tableName,
              const QString &field, int optor);
    bool exec(const QStringList &attributes, const QString &newTableName,
              const QString &tableName, const QString &field, int optor,
              const QString &condition);
    bool exec(const QStringList &attributes, const QString &newTableName,
              const QString &tableName, const QString &field, int optor,
              const QString &condition1, const QString &condition2);
    bool exec(const QStringList &attributes, const QString &newTableName,
              const QString &tableName, const QString &field, int optor);

    void createActions();
};

#endif // QUERYFORM_H
