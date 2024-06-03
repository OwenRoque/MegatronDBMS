#include "database.h"
#include "heapfile.h"
#include "record.h"
#include <QPointer>

Core::Database::Database(QSharedPointer<Storage::DiskController> dc, QString storageFile)
{
    sc = &SystemCatalog::getInstance();
    dm = &DiskManager::getInstance(dc, storageFile);
}

Types::Return Core::Database::createRelation(Core::RelationInput response)
{
    // more validations, handle responses
    auto relations = sc->getRelations();
    // checking duplicate relations
    if (relations.constFind(response.relationName) != relations.constEnd())
        return Types::Return::DuplicateError;

    // Check if a bulk insert operation will be done after creating the relation
    bool bulkInsert = false;
    if (!response.dataPath.isEmpty())
    {
        bulkInsert = true;
        // Extract records and calculate fileSize (overhead included)
        int fileSize;
        QQueue<QPointer<Core::Record>> records;
        QFile newData(response.dataPath);
        if (newData.open(QIODevice::ReadOnly))
        {
            QTextStream in(&newData);
            // ignore header
            QString line = in.readLine();
            while (!in.atEnd())
            {
                QString line = in.readLine();
                // parse record algorithm
                std::stringstream inLine(line.toStdString());
                QStringList values;
                bool insideQuotes = false;
                QString word;
                char c;
                while (inLine.get(c))
                {
                    // qDebug() << word;
                    if (c == ',') {
                        // If no content
                        if (word.isEmpty()) {
                            // qDebug() << "Nan";
                            values.append("");
                        }
                        else if (insideQuotes)
                            word+=c;
                        else {
                            values.append(word);
                            word.clear();
                        }
                    }
                    // not so sure about some (unlikely) cases like  ""hello", he said"
                    else if (c == '"') {
                        if (word.isEmpty())
                            insideQuotes = true;
                        else {
                            char p = inLine.peek();
                            // If data ends
                            if (p == ',') {
                                values.append(word);
                                word.clear();
                                insideQuotes = false;
                                inLine.seekg(1, std::ios_base::cur);
                            }
                            // Then it's a '"' inside commillas
                            else
                                word+=c;
                        }
                    }
                    else word+=c;
                }
                // handle last field
                if (!word.isEmpty())
                    values.append(word);

                for (qsizetype i = 0; i < values.size(); ++i)
                {
                    const auto &w = values.at(i);
                    // create RECORDD
                }
            }
            // run table.bulk<insert
            // as records are created as objects (record.h), fill them one by one
            // when BufferManager is full, write to disk evicted pages
            newData.close();
        }
        else return Types::Return::OpenError;
    }

    sc->insertRelationMetadata(
        response.relationName,
        response.attributes.size(),
        response.fileOrg,
        response.recFormat,
        dm->newFileGroup(response.fileOrg)
    );

    // IMPORTANT: bypassing bufferManager allocation,
    // it will be directly stored on 'cache'
    for (int i = 0; i < response.attributes.size(); i++)
    {
        QString attributeName = std::get<0>(response.attributes.at(i));
        Types::DataType datatype = std::get<1>(response.attributes.at(i));
        int length = std::get<2>(response.attributes.at(i));
        bool isNull = std::get<3>(response.attributes.at(i));
        bool ai = std::get<4>(response.attributes.at(i));
        sc->insertAttributeMetadata(
            response.relationName,
            attributeName,
            datatype,
            length,
            i,
            isNull,
            ai
            );
    }
    switch (response.fileOrg)
    {
    case Types::FileOrganization::Heap:
        this->relations.emplaceBack(QSharedPointer<Core::HeapFile>::create());
    case Types::FileOrganization::Sequential:
    case Types::FileOrganization::Hash:
    case Types::FileOrganization::BPlusTree:
        break;
    }


    // TODO: implement SaveToDisk in each FileOrganization
    // only when pressed 'save' button, CTRL+S



    return Types::Return::Success;
}
