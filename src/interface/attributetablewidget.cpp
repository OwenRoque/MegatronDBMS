#include "attributetablewidget.h"
#include "lineeditdelegate.h"
#include "comboboxdelegate.h"
#include "checkboxdelegate.h"
#include "texteditdelegate.h"
#include "customindexcombobox.h"

AttributeTableWidget::AttributeTableWidget(QWidget *parent)
{
    qRegisterMetaType<QSharedPointer<IndexForm>>();
    setColumnCount(9);
    QStringList headers = { "Attribute Name", "Type", "Length", "Index", "Default", "Unsigned", "Null", "AI", "Comment" };
    setHorizontalHeaderLabels(headers);
    setColumnWidth(0, 200);
    setColumnWidth(1, 150);
    setColumnWidth(2, 125);
    setColumnWidth(3, 150);
    setColumnWidth(4, 125);
    setColumnWidth(5, 80);
    setColumnWidth(6, 80);
    setColumnWidth(7, 80);
    setColumnWidth(8, 100);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setDefaultSectionSize(85);

    // Set delegates for specific columns
    setItemDelegateForColumn(0, new LineEditDelegate(this));
    setItemDelegateForColumn(1, new ComboBoxDelegate(this, dataTypeComboBoxItems));
    setItemDelegateForColumn(2, new LineEditDelegate(this));    // new QIntValidator(this)
    // column 3 is an static widget, not using delegate
    setItemDelegateForColumn(4, new LineEditDelegate(this));
    setItemDelegateForColumn(5, new CheckBoxDelegate(this));
    setItemDelegateForColumn(6, new CheckBoxDelegate(this));
    setItemDelegateForColumn(7, new CheckBoxDelegate(this));
    setItemDelegateForColumn(8, new TextEditDelegate(this));
}

void AttributeTableWidget::addRow(const QString& attributeName)
{
    int currentRow = QTableWidget::currentRow();
    QTableWidget::insertRow(currentRow + 1);
    QTableWidget::setCurrentCell(currentRow + 1, 0);

    // Use the corresponding delegates to configure the editor for this cell
    for (int col = 0; col < columnCount(); ++col) {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        setItem(currentRow + 1, col, newItem);

        QStyledItemDelegate *delegate = nullptr;
        if (col == 0) {
            delegate = new LineEditDelegate(this);
            newItem->setText(attributeName);
        } else if (col == 1) {
            delegate = new ComboBoxDelegate(this, dataTypeComboBoxItems);
        } else if (col == 2) {
            delegate = new LineEditDelegate(this); // new QIntValidator(this)
        } else if (col == 3) {
            // This will replace previous item setted in the cell
            QWidget* widget = new CustomIndexComboBox(this, indexOptionComboBoxItems);
            setCellWidget(currentRow + 1, col, widget);
        } else if (col == 4) {
            delegate = new LineEditDelegate(this);
        } else if (col == 5 || col == 6 || col == 7) {
            delegate = new CheckBoxDelegate(this);
        } else if (col == 8) {
            delegate = new TextEditDelegate(this);
        }

        if (delegate) {
            setItemDelegateForColumn(col, delegate);
        }
    }
}

void AttributeTableWidget::removeRow()
{
    int currentRow = QTableWidget::currentRow();
    if (currentRow != 1)
        QTableWidget::removeRow(currentRow);
}

void AttributeTableWidget::clearContents()
{
    QTableWidget::clearContents();
    QTableWidget::setRowCount(0);
}

QVariant AttributeTableWidget::getItemDelegateContent(int row, int column, bool getFirstValue = true) const
{
    QModelIndex index = model()->index(row, column);
    QStyledItemDelegate *delegate = qobject_cast<QStyledItemDelegate *>(itemDelegateForIndex(index));
    // Handle content retrieval from widget at column 4
    if (column == 3) {
        QWidget *widget = cellWidget(row, column);
        if (widget) {
            CustomIndexComboBox* custom = qobject_cast<CustomIndexComboBox*>(widget);
            if (custom->hasIndexDefined()) {
                if (getFirstValue)
                    return custom->currentKeyConstraintComboBox();
                else
                    return custom->currentIndexProperties();
            }
            else return QVariant::fromValue(Types::KeyConstraintType::None);
        }
    }
    else if (delegate)
    {
        if (auto comboBoxDelegate = dynamic_cast<ComboBoxDelegate*>(delegate)) {
            if (getFirstValue)
                return comboBoxDelegate->getCurrentText(index);
            else
                return comboBoxDelegate->getCurrentData(index);
        }
        else if (auto lineEditDelegate = dynamic_cast<LineEditDelegate*>(delegate)) {
            return lineEditDelegate->getCurrentText(index);
        } else if (auto checkBoxDelegate = dynamic_cast<CheckBoxDelegate*>(delegate)) {
            return checkBoxDelegate->getCurrentChecked(index);
        } else if (auto textEditDelegate = dynamic_cast<TextEditDelegate*>(delegate)) {
            return textEditDelegate->getCurrentText(index);
        }
    }
    // If no specific delegate or type is found, return the default data
    return index.data(Qt::EditRole);
}

Ui::attributePackage AttributeTableWidget::tableAttributes()
{
    ap.clear();
    for (qsizetype row = 0; row < QTableWidget::rowCount(); ++row)
    {
        QString nameValue = getItemDelegateContent(row, 0).value<QString>();
        Types::DataType typeValue = getItemDelegateContent(row, 1, false).value<Types::DataType>();
        QString columnValue = getItemDelegateContent(row, 2, true).value<QString>();
        Types::KeyConstraintType keyValue = getItemDelegateContent(row, 3, true).value<Types::KeyConstraintType>();
        QString defaultValue = getItemDelegateContent(row, 4).value<QString>();
        bool unsignedValue = getItemDelegateContent(row, 5).value<bool>();
        bool nullValue = getItemDelegateContent(row, 6).value<bool>();
        bool aiValue = getItemDelegateContent(row, 7).value<bool>();
        QString commentValue = getItemDelegateContent(row, 8).value<QString>();
        ap.append(std::make_tuple(nameValue, typeValue, columnValue, keyValue, defaultValue, unsignedValue, nullValue, aiValue, commentValue));
    }
    return ap;
}

Ui::indexPackage AttributeTableWidget::tableIndexes()
{
    ip.clear();
    for (qsizetype row = 0; row < QTableWidget::rowCount(); ++row)
    {
        QString nameValue = getItemDelegateContent(row, 0).value<QString>();
        QString indexNameValue; Types::KeyConstraintType indexKeyType; Types::IndexType indexTypeValue; QString commentValue;
        QVariant indexContents = getItemDelegateContent(row, 3, false);
        if (indexContents.canConvert<QList<QVariant>>() && !indexContents.isNull()) {
            QList<QVariant> list = indexContents.value<QList<QVariant>>();
            indexNameValue = list.at(0).value<QString>();
            indexKeyType = list.at(1).value<Types::KeyConstraintType>();
            indexTypeValue = list.at(2).value<Types::IndexType>();
            commentValue = list.at(3).value<QString>();
            ip.append(std::make_tuple(nameValue, indexNameValue, indexKeyType, indexTypeValue, commentValue));
        }
    }
    return ip;
}
