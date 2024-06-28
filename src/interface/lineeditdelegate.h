#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QLineEdit>

class LineEditDelegate : public QStyledItemDelegate {
public:
    LineEditDelegate(QObject *parent = nullptr, QValidator *validator = nullptr)
        : QStyledItemDelegate(parent), validator(validator) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QLineEdit *editor = new QLineEdit(parent);
        if (validator) {
            editor->setValidator(validator);
        }
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        QString value = lineEdit->text();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

    QString getCurrentText(QModelIndex index) const {
        return index.model()->data(index, Qt::EditRole).toString();
    }

private:
    QValidator *validator;

};

#endif // LINEEDITDELEGATE_H
