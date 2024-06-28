#include "record.h"
#include "systemcatalog.h"
#include <QQueue>
#include <QStringConverter>
#include <limits>

template<Types::DataType T>
static void writeTypedData(QDataStream& stream, const QString& unformatted, Types::RecordFormat rf,
                    Types::Charset charset, quint32 maxByteLength, quint64 autoIncrementValue)
{
    using ValueType = typename Types::DTAlias<T>::type;
    ValueType value;

    if constexpr (std::is_integral_v<ValueType>) {
        if (autoIncrementValue != 0) {
            value = static_cast<ValueType>(autoIncrementValue);
            goto jmp;
        }
    }
    // consider:
    // NULL values in fixed-point signed numbers = numeric_limits<T>::min()
    // NULL values in fixed-point unsigned numbers = numeric_limits<T>::max()
    // NULL values in floating-point numbers = numeric_limits<T>::quiet_NaN()
    if (unformatted == "" || unformatted.isEmpty()) {
        switch (rf) {
        case Types::Fixed:
        {
            if constexpr (std::is_integral_v<ValueType> && std::is_signed_v<ValueType>) {
                value = std::numeric_limits<ValueType>::min();
            }
            else if constexpr (std::is_integral_v<ValueType> && std::is_unsigned_v<ValueType>) {
                value = std::numeric_limits<ValueType>::max();
            }
            else if constexpr (std::is_floating_point_v<ValueType>) {
                value = std::numeric_limits<ValueType>::quiet_NaN();
            }
            break;
        }
        // this case won't ever happen
        case Types::Variable:
            return;
        }
    }
    else {
        if constexpr (std::is_same_v<ValueType, qint8>) {
            value = static_cast<qint8>(unformatted.toShort());
        } else if constexpr (std::is_same_v<ValueType, quint8>) {
            value = static_cast<quint8>(unformatted.toUShort());
        } else if constexpr (std::is_same_v<ValueType, qint16>) {
            value = unformatted.toShort();
        } else if constexpr (std::is_same_v<ValueType, quint16>) {
            value = unformatted.toUShort();
        } else if constexpr (std::is_same_v<ValueType, qint32>) {
            value = unformatted.toInt();
        } else if constexpr (std::is_same_v<ValueType, quint32>) {
            value = unformatted.toUInt();
        } else if constexpr (std::is_same_v<ValueType, qint64>) {
            value = unformatted.toLongLong();
        } else if constexpr (std::is_same_v<ValueType, quint64>) {
            value = unformatted.toULongLong();
        } else if constexpr (std::is_same_v<ValueType, float>) {
            value = unformatted.toFloat();
        } else if constexpr (std::is_same_v<ValueType, double>) {
            value = unformatted.toDouble();
        } else if constexpr (std::is_same_v<ValueType, bool>) {
            value = (unformatted.toLower() == "true" || unformatted == "1");
        } else if constexpr (T == Types::DataType::Char) {
            // handle charset type
            QByteArray value;
            switch (charset) {
            case Types::Latin1:
            {
                auto toLatin1 = QStringEncoder(QStringEncoder::Latin1);
                value = toLatin1(unformatted);
                break;
            }
            case Types::Utf8:
            {
                auto toUtf8 = QStringEncoder(QStringEncoder::Utf8);
                value = toUtf8(unformatted);
                break;
            }
            case Types::Utf16:
            {
                auto toUtf16 = QStringEncoder(QStringEncoder::Utf16);
                value = toUtf16(unformatted);
                break;
            }
            case Types::Utf32:
            {
                auto toUtf32 = QStringEncoder(QStringEncoder::Utf32);
                value = toUtf32(unformatted);
                break;
            }
            }
            if (value.size() < maxByteLength)
                // fill with trailing spaces
                value.append(maxByteLength - value.size(), ' ');
            if (value.size() > maxByteLength)
                // truncate excedent
                value = value.left(maxByteLength);
            // not handling VARCHAR, VARCHAR is not allowed in Fixed-length format
            // and it will be handled differently in Variable-length (offsets-lengths)
            stream.writeRawData(value.constData(), value.size());
            return;
        }
        // ENUM will also be handled specifically
    }
    jmp:
    stream.writeRawData(reinterpret_cast<const char*>(&value), sizeof(value));
}

template<Types::DataType T>
static void writeTypedData(QDataStream& stream, const QVariant& value, Types::Charset charset, int length = 0)
{
    using ValueType = typename Types::DTAlias<T>::type;

    if constexpr (T == Types::DataType::Char) {
        QByteArray rawValue;
        switch (charset) {
        case Types::Latin1:
        {
            auto toLatin1 = QStringEncoder(QStringEncoder::Latin1);
            rawValue = toLatin1(value.toString());
            break;
        }
        case Types::Utf8:
        {
            auto toUtf8 = QStringEncoder(QStringEncoder::Utf8);
            rawValue = toUtf8(value.toString());
            break;
        }
        case Types::Utf16:
        {
            auto toUtf16 = QStringEncoder(QStringEncoder::Utf16);
            rawValue = toUtf16(value.toString());
            break;
        }
        case Types::Utf32:
        {
            auto toUtf32 = QStringEncoder(QStringEncoder::Utf32);
            rawValue = toUtf32(value.toString());
            break;
        }
        }
        if (rawValue.size() < length) {
            // Fill with trailing spaces
            rawValue.append(length - rawValue.size(), ' ');
        } else if (rawValue.size() > length) {
            // Truncate excess
            rawValue = rawValue.left(length);
        }
        stream.writeRawData(rawValue.data(), length);
    } else if constexpr (T == Types::DataType::Varchar) {

    }
    else {
        ValueType rawValue = value.value<ValueType>();
        stream.writeRawData(reinterpret_cast<const char*>(&rawValue), sizeof(ValueType));
    }
}

template<Types::DataType T>
static QVariant readTypedData(QDataStream& stream, Types::Charset charset, int length = 0)
{
    using ValueType = typename Types::DTAlias<T>::type;
    ValueType value;

    QByteArray rawValue;
    if constexpr (std::is_same_v<ValueType, QString>) {
        rawValue.resize(length);
        stream.readRawData(rawValue.data(), length);
        switch (charset) {
        case Types::Charset::Latin1:
        {
            auto fromLatin1 = QStringDecoder(QStringDecoder::Latin1);
            value = fromLatin1(rawValue);
            break;
        }
        case Types::Utf8:
        {
            auto fromUtf8 = QStringDecoder(QStringDecoder::Utf8);
            value = fromUtf8(rawValue);
            break;
        }
        case Types::Utf16:
        {
            auto fromUtf16 = QStringDecoder(QStringDecoder::Utf16);
            value = fromUtf16(rawValue);
            break;
        }
        case Types::Utf32:
            auto fromUtf32 = QStringDecoder(QStringDecoder::Utf32);
            value = fromUtf32(rawValue);
            break;
        }
    }
    else {
        stream.readRawData(reinterpret_cast<char*>(&value), sizeof(ValueType));
    }

    return QVariant(value);
}

// constructor for newly created records from a file
Core::FLRecord::FLRecord(const QString &relationName, const QStringList &unformatted)
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->findRelation(relationName);
    // access from the least recently to most recently inserted attribute
    auto [beg, it] = sc->constFindAttributesFor(relationName);

    QDataStream stream(&data, QIODevice::ReadWrite);
    // Record is created according to the relation's schema specified, but not linked to it
    // no validations of data are made yet

    for (qsizetype i = 0; it != beg; --it, i++)
    {
        switch (it->dataType)
        {
        case Types::DataType::TinyInt:
            writeTypedData<Types::DataType::TinyInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::UTinyInt:
            writeTypedData<Types::DataType::UTinyInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::SmallInt:
            writeTypedData<Types::DataType::SmallInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::USmallInt:
            writeTypedData<Types::DataType::USmallInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Int:
            writeTypedData<Types::DataType::Int>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::UInt:
            writeTypedData<Types::DataType::UInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::BigInt:
            writeTypedData<Types::DataType::BigInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::UBigInt:
            writeTypedData<Types::DataType::UBigInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Float:
            writeTypedData<Types::DataType::Float>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Double:
            writeTypedData<Types::DataType::Double>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Bool:
            writeTypedData<Types::DataType::Bool>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Enum:
        {
            QStringList enumValues = it->columnType.split(',');
            quint8 value;
            // allows 256 enum types, check that enumValues size doesn't exceed
            // this number when creating the relation
            for (quint8 j = 0; i < enumValues.size(); ++j)
                if (unformatted[i] == enumValues[j])
                    value = j;
            stream.writeRawData(reinterpret_cast<const char*>(&value), sizeof(value));
            break;
        }
        case Types::DataType::Char:
            writeTypedData<Types::DataType::Char>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            break;
        case Types::DataType::Varchar:
            // Not Allowed, validation already done in interface, no action needed
            // IDEA: if allowed, would it be treated as a char internally?
            break;
        }
        // handle autoIncrement field cases. ignore autoIncrement unformatted fields. Default starts at 1
        if (it->autoIncrement)
            relation->autoIncrement++;
    }
}

// constructor for pre-existing records on disk
Core::FLRecord::FLRecord(const QString &relationName, const QByteArray &rawData)
{
    // if the record was correctly constructed previously and constraints checked, rawData should be in correct format.
    // Since fixed-length layout doesn't include a header, it doesn't need additional processing
    data = rawData;
}

QVariant Core::FLRecord::getField(const QString& relationName, int position) const
{
    // access records via arithmetic operations
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->constFindRelation(relationName);
    // sort from the least to the most recently inserted value
    // it works since attributes' insertion is made in position order
    // and updated & reordered when an attribute is deleted
    auto [beg, it] = sc->constFindAttributesFor(relationName);
    // sum offset
    qsizetype base = 0;
    QVariant result;
    while (it != beg)
    {
        --it;
        if (it->ordinalPosition == position)
        {
            // handle conversion based on type
            QByteArray rawValue = data.mid(base, it->maxByteLength);
            QDataStream stream(&rawValue, QIODeviceBase::ReadOnly);
            switch (it->dataType)
            {
            case Types::DataType::TinyInt:
                result = readTypedData<Types::DataType::TinyInt>(stream, relation->charset);
                break;
            case Types::DataType::UTinyInt:
                result = readTypedData<Types::DataType::UTinyInt>(stream, relation->charset);
                break;
            case Types::DataType::SmallInt:
                result = readTypedData<Types::DataType::SmallInt>(stream, relation->charset);
                break;
            case Types::DataType::USmallInt:
                result = readTypedData<Types::DataType::USmallInt>(stream, relation->charset);
                break;
            case Types::DataType::Int:
                result = readTypedData<Types::DataType::Int>(stream, relation->charset);
                break;
            case Types::DataType::UInt:
                result = readTypedData<Types::DataType::UInt>(stream, relation->charset);
                break;
            case Types::DataType::BigInt:
                result = readTypedData<Types::DataType::BigInt>(stream, relation->charset);
                break;
            case Types::DataType::UBigInt:
                result = readTypedData<Types::DataType::UBigInt>(stream, relation->charset);
                break;
            case Types::DataType::Float:
                result = readTypedData<Types::DataType::Float>(stream, relation->charset);
                break;
            case Types::DataType::Double:
                result = readTypedData<Types::DataType::Double>(stream, relation->charset);
                break;
            case Types::DataType::Bool:
                result = readTypedData<Types::DataType::Bool>(stream, relation->charset);
                break;
            case Types::DataType::Enum:
            {
                QStringList enumValues = it->columnType.split(',');
                quint8 value;
                stream.readRawData(reinterpret_cast<char*>(&value), sizeof(quint8));
                result = enumValues.at(value);
                break;
            }
            case Types::DataType::Char:
                result = readTypedData<Types::DataType::Char>(stream, relation->charset, it->maxByteLength);
                break;
            case Types::DataType::Varchar:
                // Not Allowed, validation already done in interface
                break;
            }
            return result;
        }
        else
        {
            // update base
            switch (it->dataType)
            {
            case Types::DataType::TinyInt:
            case Types::DataType::UTinyInt:
            case Types::DataType::Enum:
                base += sizeof(qint8);
                break;
            case Types::DataType::SmallInt:
            case Types::DataType::USmallInt:
                base += sizeof(qint16);
                break;
            case Types::DataType::Int:
            case Types::DataType::UInt:
                base += sizeof(qint32);
                break;
            case Types::DataType::BigInt:
            case Types::DataType::UBigInt:
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
                base += it->maxByteLength;
                break;
            case Types::DataType::Varchar:
                // Not Allowed, validation already done in interface
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
    auto relation = sc->constFindRelation(relationName);
    // access from the least recently to most recently inserted attribute
    auto [beg, it] = sc->constFindAttributesFor(relationName);
    // sum offset
    qsizetype base = 0;
    while (it != beg)
    {
        --it;
        if (it->ordinalPosition == position)
        {
            QByteArray value;
            QDataStream stream(&value, QIODevice::WriteOnly);
            // handle conversion based on type
            switch (it->dataType)
            {
            case Types::DataType::TinyInt:
                writeTypedData<Types::DataType::TinyInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::UTinyInt:
                writeTypedData<Types::DataType::UTinyInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::SmallInt:
                writeTypedData<Types::DataType::SmallInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::USmallInt:
                writeTypedData<Types::DataType::USmallInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::Int:
                writeTypedData<Types::DataType::Int>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::UInt:
                writeTypedData<Types::DataType::UInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::BigInt:
                writeTypedData<Types::DataType::BigInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::UBigInt:
                writeTypedData<Types::DataType::UBigInt>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::Float:
                writeTypedData<Types::DataType::Float>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::Double:
                writeTypedData<Types::DataType::Double>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::Bool:
                writeTypedData<Types::DataType::Bool>(stream, rawValue, relation->charset);
                break;
            case Types::DataType::Enum:
            {
                QStringList enumValues = it->columnType.split(',');
                QString strValue = rawValue.toString();
                quint8 value;
                for (quint8 i = 0; i < enumValues.size(); ++i)
                    if (strValue == enumValues[i])
                        value = i;
                stream.writeRawData(reinterpret_cast<const char*>(&value), sizeof(value));
                break;
            }
            case Types::DataType::Char:
                writeTypedData<Types::DataType::Char>(stream, rawValue, relation->charset);
                break;
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
            switch (it->dataType)
            {
            case Types::DataType::TinyInt:
            case Types::DataType::UTinyInt:
            case Types::DataType::Enum:
                base += sizeof(qint8);
                break;
            case Types::DataType::SmallInt:
            case Types::DataType::USmallInt:
                base += sizeof(qint16);
                break;
            case Types::DataType::Int:
            case Types::DataType::UInt:
                base += sizeof(qint32);
                break;
            case Types::DataType::BigInt:
            case Types::DataType::UBigInt:
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
                base += it->maxByteLength;
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

QByteArray Core::FLRecord::toBytes() const
{
    return data;
}

Core::VLRecord::VLRecord(const QString &relationName, const QStringList &unformatted)
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->findRelation(relationName);
     // access from the least recently to most recently inserted attribute
    auto [beg, it] = sc->constFindAttributesFor(relationName);
    // init nullBitMap size
    int size = qCeil(sc->numberOfAttributes(relationName) / 8);
    nullBitmap.resize(size * 8);

    QDataStream stream(&data, QIODevice::ReadWrite);
    // Record is created according to the relation's schema specified, but not linked to it
    // no validations of data are made yet
    // consider NULL values are not stored, but marked as NULL in the bitmap
    // where: 1 = null, 0 = not null
    QQueue<QPair<qint16, qint16>> varcharQueue;

    for (qsizetype i = 0; it != beg; --it, i++)
    {
        if (unformatted.at(i) == "" || unformatted.at(i).isEmpty())
        {
            nullBitmap.setBit(i, 1);
            continue;
        }
        switch (it->dataType)
        {
        case Types::DataType::TinyInt:
        {
            writeTypedData<Types::DataType::TinyInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::UTinyInt:
        {
            writeTypedData<Types::DataType::UTinyInt>(stream, unformatted[i], relation->recordFormat,
                                                     relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::SmallInt:
        {
            writeTypedData<Types::DataType::SmallInt>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::USmallInt:
        {
            writeTypedData<Types::DataType::USmallInt>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Int:
        {
            writeTypedData<Types::DataType::Int>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::UInt:
        {
            writeTypedData<Types::DataType::UInt>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::BigInt:
        {
            writeTypedData<Types::DataType::BigInt>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::UBigInt:
        {
            writeTypedData<Types::DataType::UBigInt>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Float:
        {
            writeTypedData<Types::DataType::Float>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Double:
        {
            writeTypedData<Types::DataType::Double>(stream, unformatted[i], relation->recordFormat,
                                                      relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Bool:
        {
            writeTypedData<Types::DataType::Double>(stream, unformatted[i], relation->recordFormat,
                                                    relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Enum:
        {
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Char:
        {
            writeTypedData<Types::DataType::Char>(stream, unformatted[i], relation->recordFormat,
                                                    relation->charset, it->maxByteLength, relation->autoIncrement);
            nullBitmap.setBit(i, 0);
            break;
        }
        case Types::DataType::Varchar:
        {
            // save in queue <pos. to modify later , unformatIndex>
            qint16 offset = static_cast<qint16>(data.size());
            qint16 length = static_cast<qint16>(unformatted.at(i).size());
            varcharQueue.enqueue(qMakePair(offset, length));
            // offset is a temporal values, it will be overwritten later
            // length value is OK
            stream << offset << length;
            break;
        }
        }
        // handle autoIncrement field cases. ignore autoIncrement unformatted fields. Default starts at 1
        if (it->autoIncrement)
            relation->autoIncrement++;
    }
    while (!varcharQueue.empty())
    {
        // retrieve values from queue
        QPair<qint16, qint16> pair(varcharQueue.dequeue());
        // retrieve the current stream position
        qint16 offset = static_cast<qint16>(stream.device()->pos());
        // set the position where to overwrite (varchar fields)
        stream.device()->seek(pair.first);
        stream.writeRawData(reinterpret_cast<const char*>(&offset), sizeof(offset));
        // come back to the current position of the pointer
        stream.device()->seek(offset);
        // write actual varchar data
        stream << unformatted.at(pair.second);
    }
}

Core::VLRecord::VLRecord(const QString &relationName, const QByteArray &rawData)
{
    // if the record was correctly constructed previously and constraints checked, rawData should be in correct format.
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    // handle record header (nullBitMap)
    int size = qCeil(sc->numberOfAttributes(relationName) / 8);
    nullBitmap.resize(size * 8);
    QByteArray nbmTemporalAlloc = rawData.left(size);
    for (int i = 0; i < nbmTemporalAlloc.size(); ++i) {
        for (int j = 0; j < 8; ++j) {
            nullBitmap.setBit(i * 8 + j, (nbmTemporalAlloc.at(i) & (1 << (7 - j))) != 0);
        }
    }
    data = rawData.mid(size);
}

// index starts at 1
QVariant Core::VLRecord::getField(const QString& relationName, int index) const
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto relation = sc->constFindRelation(relationName);
    // sort from the least to the most recently inserted value
    auto [beg, it] = sc->constFindAttributesFor(relationName);

    // sum offset
    qsizetype base = 0;
    QVariant result;
    while (it != beg)
    {
        --it;
        if (it->ordinalPosition == index)
        {
            if (this->nullBitmap.testBit(index) == true)
            {
                // handle conversion based on type
                QByteArray rawValue;
                if (it->dataType != Types::DataType::Varchar)
                    rawValue = data.mid(base, it->maxByteLength);
                else
                    // size of string offset/length pointer
                    rawValue = data.mid(base, 4);
                QDataStream stream(&rawValue, QIODeviceBase::ReadOnly);
                switch (it->dataType)
                {
                case Types::DataType::TinyInt:
                    result = readTypedData<Types::DataType::TinyInt>(stream, relation->charset);
                    break;
                case Types::DataType::UTinyInt:
                    result = readTypedData<Types::DataType::UTinyInt>(stream, relation->charset);
                    break;
                case Types::DataType::SmallInt:
                    result = readTypedData<Types::DataType::SmallInt>(stream, relation->charset);
                    break;
                case Types::DataType::USmallInt:
                    result = readTypedData<Types::DataType::USmallInt>(stream, relation->charset);
                    break;
                case Types::DataType::Int:
                    result = readTypedData<Types::DataType::Int>(stream, relation->charset);
                    break;
                case Types::DataType::UInt:
                    result = readTypedData<Types::DataType::UInt>(stream, relation->charset);
                    break;
                case Types::DataType::BigInt:
                    result = readTypedData<Types::DataType::BigInt>(stream, relation->charset);
                    break;
                case Types::DataType::UBigInt:
                    result = readTypedData<Types::DataType::UBigInt>(stream, relation->charset);
                    break;
                case Types::DataType::Float:
                    result = readTypedData<Types::DataType::Float>(stream, relation->charset);
                    break;
                case Types::DataType::Double:
                    result = readTypedData<Types::DataType::Double>(stream, relation->charset);
                    break;
                case Types::DataType::Bool:
                    result = readTypedData<Types::DataType::Bool>(stream, relation->charset);
                    break;
                case Types::DataType::Enum:
                {
                    QStringList enumValues = it->columnType.split(',');
                    quint8 value;
                    stream.readRawData(reinterpret_cast<char*>(&value), sizeof(quint8));
                    result = enumValues.at(value);
                    break;
                }
                case Types::DataType::Char:
                    result = readTypedData<Types::DataType::Char>(stream, relation->charset, it->maxByteLength);
                    break;
                case Types::DataType::Varchar:
                {
                    // read string pointer
                    QDataStream ss(rawValue);
                    qint16 offset, length;
                    ss >> offset >> length;
                    QByteArray value = data.mid(offset, length);
                    // move stream pointer
                    stream.device()->seek(offset);
                    result = readTypedData<Types::DataType::Varchar>(stream, relation->charset, length);
                    break;
                }
                }
                return result;
            }
            // specified column/index in the record is null/has no data stored
            else return QVariant();
        }
        else
        {
            // shift base/position pointer in byte array only
            // if is NOT NULL
            if (this->nullBitmap.testBit(index) == true)
            {
                // update base
                switch (it->dataType)
                {
                case Types::DataType::TinyInt:
                case Types::DataType::UTinyInt:
                case Types::DataType::Enum:
                    base += sizeof(qint8);
                    break;
                case Types::DataType::SmallInt:
                case Types::DataType::USmallInt:
                    base += sizeof(qint16);
                    break;
                case Types::DataType::Int:
                case Types::DataType::UInt:
                    base += sizeof(qint32);
                    break;
                case Types::DataType::BigInt:
                case Types::DataType::UBigInt:
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
                    base += it->maxByteLength;
                    break;
                case Types::DataType::Varchar:
                    // size of offset/length string pointer
                    base += 4;
                    break;
                }
            }
        }
    }
    // invalid position
    return QVariant();
}

// TODO:
bool Core::VLRecord::setField(const QString& relationName, int index, const QVariant &value)
{
    return true;
}

int Core::VLRecord::size() const
{
    return nullBitmap.size() + data.size();
}

QByteArray Core::VLRecord::toBytes() const
{
    auto nbmAsBytes = [](const QBitArray& bitArray) -> QByteArray {
        QByteArray byteArray;
        // round to the nearest byte
        int byteCount = (bitArray.size() + 7) / 8;
        byteArray.resize(byteCount);
        byteArray.fill(0);

        for (int i = 0; i < bitArray.size(); ++i) {
            byteArray[i / 8] |= (bitArray.testBit(i) ? 1 : 0) << (7 - (i % 8));
        }

        return byteArray;
    };
    QByteArray nbm = nbmAsBytes(nullBitmap);
    return nbm + data;
}


