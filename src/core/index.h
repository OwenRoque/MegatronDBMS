#ifndef INDEX_H
#define INDEX_H

#include "systemcatalog.h"
#include "page.h"

namespace Core
{
    using record_id = Core::rowId;
    /**
     * @brief The index structure majorly maintains information on the schema of the
     * schema of the underlying table and the mapping relation between index key
     * and tuple key, and provides an abstracted way for the external world to
     * interact with the underlying index implementation without exposing
     * the actual implementation's interface.
     */
    class Index
    {
    public:
        /**
         * @brief creates a new Index interface
         */
        Index(const QList<Core::SystemCatalog::indexMeta>& metadata);
        /**
         * @brief getMetaData
         * @return Return the metadata object associated with the index
         */
        QList<Core::SystemCatalog::indexMeta> getMetaData() const;
        /**
         * @brief getIndexColumnCount
         * @return Returns the number of columns inside index key (not in tuple key)
         */
        int getIndexColumnCount() const;
        /**
         * @brief getIndexName
         * @return Returns the name of the index
         */
        QString getIndexName() const;

        /**
         * @brief creates an index entry with the given record (key)
         * @param record record object to create its linked index entry
         * @param rid rowId/recordId, location of the record in data file (value)
         */
        virtual void insertEntry(Core::Record& record, record_id rid) = 0;
        /**
         * @brief deletes an index entry with the given record (key)
         * @param record record object to locate & delete its linked index entry
         * @param rid rowId/recordId, location of the record in data file (value)
         */
        virtual void deleteEntry(Core::Record& record, record_id rid) = 0;
        /**
         * @brief searches index entries which match the predicate
         * @param record record object to create its linked index entry
         * @param result list of record ids
         */
        virtual void scanKey(Core::Record& record, QList<record_id>& result) = 0;

    private:
        // stores the key attributes of the index, retrieved from the system catalog
        QList<Core::SystemCatalog::indexMeta> metadata;

    };
}

#endif // INDEX_H
