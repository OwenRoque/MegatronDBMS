#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>
#include <QCheckBox>
#include <QMouseEvent>

class CheckBoxDelegate : public QStyledItemDelegate {
public:
    CheckBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        bool value = index.model()->data(index, Qt::EditRole).toBool();
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        checkBox->setChecked(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        bool value = checkBox->isChecked();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        editor->setGeometry(option.rect);
    }

    bool getCurrentChecked(QModelIndex index) const {
        return index.model()->data(index, Qt::EditRole).toBool();
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

        QStyleOptionButton checkBoxOption;
        checkBoxOption.state |= QStyle::State_Enabled;
        checkBoxOption.state |= checked ? QStyle::State_On : QStyle::State_Off;

        // Center the QCheckBox in the cell
        QRect rect = option.rect;
        int checkboxSize = 20; // desired size
        rect.setWidth(checkboxSize);
        rect.setHeight(checkboxSize);
        rect.moveCenter(option.rect.center());

        checkBoxOption.rect = rect;

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(30, 30); // desired size
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick)) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() != Qt::LeftButton || !option.rect.contains(mouseEvent->pos())) {
                return false;
            }
            if (event->type() == QEvent::MouseButtonDblClick) {
                return true;
            }
        } else if (event->type() != QEvent::KeyPress) {
            return false;
        }

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() != Qt::Key_Space && keyEvent->key() != Qt::Key_Select) {
                return false;
            }
        }

        bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
        return model->setData(index, !checked, Qt::EditRole);
    }
};


#endif // CHECKBOXDELEGATE_H
