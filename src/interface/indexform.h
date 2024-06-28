#ifndef INDEXFORM_H
#define INDEXFORM_H

#include <QDialog>
#include <QTableWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <megatron_types.h>
#include <QSharedPointer>

namespace Ui {
class IndexForm;
}

class IndexForm : public QDialog
{
    Q_OBJECT

public:
    explicit IndexForm(QWidget *parent = nullptr,
                       const Types::KeyConstraintType& opt = Types::KeyConstraintType::None);
    ~IndexForm();
    QString getIndexName() const;
    Types::KeyConstraintType getIndexOption() const;
    Types::IndexType getIndexType() const;
    QString getComment() const;
    void setIndexName(const QString&);
    void setIndexOption(const Types::KeyConstraintType&);
    void setIndexType(const Types::IndexType&);
    void setComment(const QString&);

private:
    Ui::IndexForm *ui;

};

Q_DECLARE_METATYPE(QSharedPointer<IndexForm>)

#endif // INDEXFORM_H
