#include "record.h"
#include "systemcatalog.h"
#include <algorithm>

Core::FLRecord::FLRecord(const QString &relationName, const QStringList &unformatted)
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto attributes = sc->getAttributes().values(relationName);
    // Sort attributes by position (reason why not using faster methods as equal_rage, constFind)
    std::sort(attributes.begin(), attributes.end(), [](const Core::SystemCatalog::attributeMeta &a,
                                                       const Core::SystemCatalog::attributeMeta &b) {
        return a.position < b.position;
    });
    // Record is created according to the relation's schema specified, but not linked to it
    // no validations of data are made yet
    for (qsizetype i = 0; i < attributes.size(); i++)
    {
        if (attributes[i].autoIncrement == -1)
        {
            switch (attributes[i].type)
            {
            case Types::DataType::TinyInt:
            {
                int temp = unformatted.at(i).toInt();
                qint8 value = static_cast<qint8>(temp);
                data.append(value);
                break;
            }
            case Types::DataType::SmallInt:
            {
                int temp = unformatted.at(i).toInt();
                qint16 value = static_cast<qint16>(temp);
                data.append(value);
                break;
            }
            case Types::DataType::Int:
            {

                // if (attributes[i].isNull == false && unformatted[i] == "") {
                //     qDebug() << attributes[i].attributeName + " is marked as NOT NULL. Record not" ;
                // }
                int value = unformatted.at(i).toInt();
                data.append(value);
                break;
            }
            case Types::DataType::BigInt:
            {
                long temp = unformatted.at(i).toLongLong();
                qint64 value = static_cast<qint64>(temp);
                data.append(value);
                break;
            }
            case Types::DataType::Float:
            {
                float value = unformatted.at(i).toFloat();
                data.append(value);
                break;
            }
            case Types::DataType::Double:
            {
                double value = unformatted.at(i).toDouble();
                data.append(value);
                break;
            }
            case Types::DataType::Bool:
            {
                bool value = (unformatted.at(i).toLower() == "true" || unformatted.at(i) == "1");
                data.append(value);
                break;
            }
            case Types::DataType::Char:
            {
                QByteArray value = unformatted.at(i).toLatin1();
                if (value.size() < attributes[i].length)
                    // fill with trailing spaces
                    value.append(attributes[i].length - value.size(), ' ');
                else if (value.size() > attributes[i].length)
                    // truncate excedent
                    value = value.left(attributes[i].length);
                data.append(value);
                break;
            }
            case Types::DataType::Varchar:
            {
                // Not Allowed, validation already done in interface
                // if allowed, it would be treated as a char
                break;
            }
            }
        }
        else
        {
            // ignore autoIncrement fields. Default starts at 1
            data.append(attributes[i].autoIncrement);
            attributes[i].autoIncrement++;
        }
    }
}

Core::FLRecord::FLRecord(const QString &relationName, const QByteArray &data)
{

}

QVariant Core::FLRecord::getField(const QString& relationName, int position) const
{
    // access records via arithmetic operations
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    // if this method works, replace FLRecord .values.sort method to this one
    // sort from the least to the most recently inserted value
    auto attributes = sc->getAttributes();
    auto [beg, it] = attributes.equal_range(relationName);
    // sum offset
    qsizetype base = 0;
    QVariant result;
    while (it != beg)
    {
        --it;
        if (it->position == position)
        {
            // handle conversion based on type
            switch (it->type)
            {
            case Types::DataType::TinyInt:
            {
                // extract & convert value
                QByteArray rawValue = data.mid(base, sizeof(qint8));
                QDataStream stream(rawValue);
                qint8 value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::SmallInt:
            {
                QByteArray rawValue = data.mid(base, sizeof(qint16));
                QDataStream stream(rawValue);
                qint16 value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Int:
            {
                QByteArray rawValue = data.mid(base, sizeof(int));
                QDataStream stream(rawValue);
                int value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::BigInt:
            {
                QByteArray rawValue = data.mid(base, sizeof(qint64));
                QDataStream stream(rawValue);
                qint64 value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Float:
            {
                QByteArray rawValue = data.mid(base, sizeof(float));
                QDataStream stream(rawValue);
                float value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Double:
            {
                QByteArray rawValue = data.mid(base, sizeof(double));
                QDataStream stream(rawValue);
                double value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Bool:
            {
                QByteArray rawValue = data.mid(base, sizeof(bool));
                QDataStream stream(rawValue);
                bool value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Char:
            {
                QByteArray rawValue = data.mid(base, it->length);
                QDataStream stream(rawValue);
                // QString treated as char[]
                QString value;
                stream >> value;
                result = QVariant(value);
                break;
            }
            case Types::DataType::Varchar:
            {
                // Not Allowed, validation already done in interface
                // if allowed, it would be treated as a char
                break;
            }
            }
            return result;
        }
        else
        {
            // update base
            switch (it->type)
            {
            case Types::DataType::TinyInt:
                base += sizeof(qint8);
                break;
            case Types::DataType::SmallInt:
                base += sizeof(qint16);
                break;
            case Types::DataType::Int:
                base += sizeof(int);
                break;
            case Types::DataType::BigInt:
                base += sizeof(qint64);
                break;
            case Types::DataType::Float:
                base += sizeof(float);
                break;
            case Types::DataType::Double:
                base += sizeof(double);
                break;
            case Types::DataType::Bool:
                base += sizeof(bool);
                break;
            case Types::DataType::Char:
                base += it->length;
                break;
            case Types::DataType::Varchar:
                // Not Allowed, validation already done in interface
                // if allowed, it would be treated as a char
                break;
            }
        }
    }
    // field not found, invalid position
    return QVariant();
}

bool Core::FLRecord::setField(const QString& relationName, int position, const QVariant &rawValue)
{
    // access records via arithmetic operations
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto attributes = sc->getAttributes();
    // if this method works, replace FLRecord .values.sort method to this one
    // sort from the least to the most recently inserted value
    auto [beg, it] = attributes.equal_range(relationName);
    // sum offset
    qsizetype base = 0;
    while (it != beg)
    {
        --it;
        if (it->position == position)
        {
            QByteArray value;
            QDataStream stream(&value, QIODevice::WriteOnly);
            // handle conversion based on type
            switch (it->type)
            {
            case Types::DataType::TinyInt:
            {
                // convert value
                qint8 value = static_cast<qint8>(rawValue.toInt());
                stream << value;
                break;
            }
            case Types::DataType::SmallInt:
            {
                qint16 value = static_cast<qint16>(rawValue.toInt());
                stream << value;
                break;
            }
            case Types::DataType::Int:
            {
                int value = rawValue.toInt();
                stream << value;
                break;
            }
            case Types::DataType::BigInt:
            {
                qint64 value = static_cast<qint64>(rawValue.toLongLong());
                stream << value;
                break;
            }
            case Types::DataType::Float:
            {
                float value = rawValue.toFloat();
                stream << value;
                break;
            }
            case Types::DataType::Double:
            {
                double value = rawValue.toDouble();
                stream << value;
                break;
            }
            case Types::DataType::Bool:
            {
                bool value = rawValue.toBool();
                stream << value;
                break;
            }
            case Types::DataType::Char:
            {
                QByteArray value = rawValue.toString().toLatin1();
                if (value.size() < it->length)
                    // fill with trailing spaces
                    value.append(it->length - value.size(), ' ');
                else if (value.size() > it->length)
                    // truncate excedent
                    value = value.left(it->length);
                stream << value;
                break;
            }
            case Types::DataType::Varchar:
            {
                // Not Allowed, validation already done in interface
                break;
            }
            }
            // replace segment in data
            data.replace(base, value.size(), value);
            return true;
        }
        else
        {
            // update base
            switch (it->type)
            {
            case Types::DataType::TinyInt:
                base += sizeof(qint8);
                break;
            case Types::DataType::SmallInt:
                base += sizeof(qint16);
                break;
            case Types::DataType::Int:
                base += sizeof(int);
                break;
            case Types::DataType::BigInt:
                base += sizeof(qint64);
                break;
            case Types::DataType::Float:
                base += sizeof(float);
                break;
            case Types::DataType::Double:
                base += sizeof(double);
                break;
            case Types::DataType::Bool:
                base += sizeof(bool);
                break;
            case Types::DataType::Char:
                base += it->length;
                break;
            case Types::DataType::Varchar:
                // Not Allowed, validation already done in interface
                break;
            }
        }
    }
    // field not found, invalid position
    return false;
}

int Core::FLRecord::size() const
{
    return data.size();
}

Core::VLRecord::VLRecord(const QString &relationName, const QStringList &data)
{

}

Core::VLRecord::VLRecord(const QString &relationName, const QByteArray &data)
{

}

QVariant Core::VLRecord::getField(const QString& relationName, int index) const
{
    return QVariant();
}

bool Core::VLRecord::setField(const QString& relationName, int index, const QVariant &value)
{
    return true;
}

int Core::VLRecord::size() const
{
    return 0;
}


