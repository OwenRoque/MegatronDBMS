#ifndef CUSTOMCOMBOBOXDELEGATE_H
#define CUSTOMCOMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include "customindexcombobox.h"

class CustomWidgetDelegate : public QStyledItemDelegate {
public:
    CustomWidgetDelegate(QObject *parent = nullptr,
                         const QList<QPair<QString, QVariant>>& items = QList<QPair<QString, QVariant>>())
        : QStyledItemDelegate(parent), comboBoxItems(items) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        CustomIndexComboBox *editor = new CustomIndexComboBox(parent, comboBoxItems);
        connect(editor, &CustomIndexComboBox::editRequested, this, [this, index](const QSharedPointer<IndexForm>& form) {
            QModelIndex modelIndex = index.sibling(index.row(), index.column());  // Obtén el índice del modelo
            QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());  // Desconstituye para modificar el modelo
            model->setData(modelIndex, QVariant::fromValue(form), Qt::UserRole);
        });
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        CustomIndexComboBox *customComboBox = qobject_cast<CustomIndexComboBox*>(editor);
        QVariant formVariant = index.data(Qt::UserRole);
        if (formVariant.isValid() && !formVariant.isNull()) {
            QSharedPointer<IndexForm> form = formVariant.value<QSharedPointer<IndexForm>>();
            customComboBox->comboBox->setCurrentIndex(customComboBox->comboBox->findData(form->getIndexOption()));
            // Update other UI elements as needed
        } else {
            customComboBox->comboBox->setCurrentIndex(0);  // Handle default state
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        CustomIndexComboBox *customComboBox = qobject_cast<CustomIndexComboBox*>(editor);
        model->setData(index, customComboBox->currentKeyConstraintComboBox(), Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(100, 30);  // Adjust size as needed
    }

    QVariant getCurrentIndexOption(QModelIndex index) const {
        CustomIndexComboBox *customComboBox = dynamic_cast<CustomIndexComboBox*>(index.data(Qt::EditRole).value<QWidget*>());
        return customComboBox->currentKeyConstraintComboBox();
    }

    QVariant getCurrentIndexProperties(QModelIndex index) const {
        CustomIndexComboBox *customComboBox = dynamic_cast<CustomIndexComboBox*>(index.data(Qt::EditRole).value<QWidget*>());
        QSharedPointer<IndexForm> form = customComboBox->currentIndexProperties();
        QList<QVariant> formElements;
        formElements = {form->getIndexName(), form->getIndexOption(), form->getIndexType(), form->getComment()};
        return formElements;
    }

private:
    QList<QPair<QString, QVariant>> comboBoxItems;
};

#endif // CUSTOMCOMBOBOXDELEGATE_H
