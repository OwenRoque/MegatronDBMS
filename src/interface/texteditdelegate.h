#ifndef TEXTEDITDELEGATE_H
#define TEXTEDITDELEGATE_H

#include <QStyledItemDelegate>
#include <QTextEdit>

class TextEditDelegate : public QStyledItemDelegate {
public:
    TextEditDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QTextEdit *editor = new QTextEdit(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
        textEdit->setPlainText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
        QString value = textEdit->toPlainText();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

    QString getCurrentText(QModelIndex index) const {
        return index.model()->data(index, Qt::EditRole).toString();
    }

};

#endif // TEXTEDITDELEGATE_H
