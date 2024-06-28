#ifndef CUSTOMINDEXCOMBOBOX_H
#define CUSTOMINDEXCOMBOBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include "indexform.h"

class CustomIndexComboBox : public QWidget {
    Q_OBJECT

public:
    CustomIndexComboBox(QWidget *parent = nullptr,
                        const QList<QPair<QString, QVariant>>& items = QList<QPair<QString, QVariant>>())
        : QWidget(parent), form(nullptr)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        comboBox = new QComboBox(this);
        for (const auto &item : items) {
            comboBox->addItem(item.first, item.second);
        }
        button = new QPushButton("Action", this);
        button->setFlat(true);
        layout->addWidget(comboBox);
        layout->addWidget(button);
        setLayout(layout);
        button->setVisible(false);
        connect(comboBox, &QComboBox::currentIndexChanged, this, &CustomIndexComboBox::onComboBoxIndexChanged);
        connect(button, &QPushButton::clicked, this, &CustomIndexComboBox::onButtonClicked);
    }

    ~CustomIndexComboBox() {
        if (form) {
            delete form;
        }
    }

    bool hasIndexDefined() const {
        if (comboBox->currentData().isNull())
            return false;
        return true;
    }

    QVariant currentKeyConstraintComboBox() const {
        return comboBox->currentData();
    }

    QList<QVariant> currentIndexProperties() const {
        QList<QVariant> ret;
        ret.append(QVariant::fromValue(form->getIndexName()));
        ret.append(QVariant::fromValue(form->getIndexOption()));
        ret.append(QVariant::fromValue(form->getIndexType()));
        ret.append(QVariant::fromValue(form->getComment()));
        return ret;
    }

private slots:
    void onComboBoxIndexChanged(int index)
    {
        QVariant data = comboBox->itemData(index);
        if (data.isValid() && !data.isNull()) {
            if (form) {
                delete form;
                form = nullptr;
            }
            form = new IndexForm(this, data.value<Types::KeyConstraintType>());
            connect(form, &QDialog::accepted, this, [this, index]() {
                QString name = form->getIndexName();
                button->setText("[" + name + "]");
                button->setVisible(true);
            });
            form->show();
        } else {
            if (form) {
                delete form;
                form = nullptr;
            }
            button->setText("");
            button->setVisible(false);
        }
    }

    void onButtonClicked()
    {
        if (form)
            form->show();
    }


private:
    QComboBox *comboBox;
    QPushButton *button;
    IndexForm* form;

};

#endif // CUSTOMINDEXCOMBOBOX_H
