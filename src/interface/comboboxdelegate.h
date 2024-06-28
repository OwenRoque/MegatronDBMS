#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>
#include <QLineEdit>
#include <megatron_types.h>

class ComboBoxDelegate : public QStyledItemDelegate {
public:
    ComboBoxDelegate(QObject *parent = nullptr,
                     const QList<QPair<QString, QVariant>> &items = QList<QPair<QString, QVariant>>())
        : QStyledItemDelegate(parent), items(items) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QComboBox *editor = new QComboBox(parent);
        for (const auto &item : items) {
            editor->addItem(item.first, item.second);
        }
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QVariant value = index.model()->data(index, Qt::EditRole);
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int idx = comboBox->findData(value);
        if (idx >= 0) {
            comboBox->setCurrentIndex(idx);
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QVariant value = comboBox->currentData();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

    QString getCurrentText(const QModelIndex &index) const {
        QVariant value = index.model()->data(index, Qt::EditRole);
        for (const auto &item : items) {
            if (item.second == value) {
                return item.first;
            }
        }
        return QString();
    }

    QVariant getCurrentData(const QModelIndex &index) const {
        return index.model()->data(index, Qt::EditRole);
    }

private:
    QList<QPair<QString, QVariant>> items;

};

#endif // COMBOBOXDELEGATE_H
