#include "record.h"
#include "systemcatalog.h"
#include <algorithm>

Core::FLRecord::FLRecord(const QString &relationName, const QStringList &unformatted)
{
    Core::SystemCatalog* sc = &Core::SystemCatalog::getInstance();
    auto attributes = sc->getAttributes().values(relationName);
    // Sort attributes by position
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
            case Types::DataType::Int:
            {

                // if (attributes[i].isNull == false && unformatted[i] == "") {
                //     qDebug() << attributes[i].attributeName + " is marked as NOT NULL. Record not" ;
                // }
                int value = unformatted.at(i).tou();
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
            case Types::DataType::TinyInt:
            {

                break;
            }
            case Types::DataType::Char:
            {
                break;
            }
            case Types::DataType::Varchar:
            {

                break;
            }
            }
        }
        else
        {
            data.append(attributes[i].autoIncrement);
            attributes[i].autoIncrement++;
        }
    }
}
