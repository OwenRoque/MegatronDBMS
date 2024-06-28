#ifndef PAGE_H
#define PAGE_H

#include <QSharedPointer>
#include <QByteArray>
#include <QBitArray>
#include <QList>
#include <block.h>
#include "record.h"

namespace Core
{

    class Page
    {
    public:
        Page(QSharedPointer<Storage::Block> block);
        Page(int);
        virtual ~Page() = 0;
        int getId() const;
        virtual QSharedPointer<Storage::Block> toBlock() = 0;
        virtual bool canInsert() const = 0;

    protected:
        int id;

    };

    struct rowId
    {
        quint16 page_id;
        quint16 slot_id;
    };

    struct slotEntry
    {
        qint16 offset;
        qint16 length;
        bool operator==(const slotEntry &other) const {
            return offset == other.offset && length == other.length;
        }

        bool operator!=(const slotEntry &other) const {
            return !(*this == other);
        }
    };


    class DataPage : public Page
    {
    public:
        DataPage(QSharedPointer<Storage::Block> block);
        DataPage(int);
        virtual bool addRecord(const Core::Record&) = 0;
        virtual bool deleteRecord(const quint16&) = 0;
        virtual QByteArray findRecord(const quint16&) = 0;

    };

    class UnpackedDataPage : public DataPage
    {
    public:
        UnpackedDataPage(QSharedPointer<Storage::Block> block);
        UnpackedDataPage(int, int);
        bool addRecord(const Core::Record&) override;
        bool deleteRecord(const quint16&) override;
        QByteArray findRecord(const quint16&) override;
        QSharedPointer<Storage::Block> toBlock() override;

    private:
        quint16 numberOfSlots;
        quint16 recordSize;
        QBitArray bitmap;
        QByteArray data;

    };

    class SlottedPage : public DataPage
    {
    public:
        SlottedPage(QSharedPointer<Storage::Block> block);
        SlottedPage(int);
        bool addRecord(const Core::Record&) override;
        bool deleteRecord(const quint16&) override;
        QByteArray findRecord(const quint16&) override;
        QSharedPointer<Storage::Block> toBlock() override;

    private:
        quint16 numberOfSlots;
        QPair<quint16, quint16> freeSpacePointer;
        QList<slotEntry> slotArray;
        QByteArray data;

    };

    // index pages TODO:

}

#endif // PAGE_H
