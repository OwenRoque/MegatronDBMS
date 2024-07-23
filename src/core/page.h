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
    using page_id_t = qint32;
    using slot_id_t = qint16;

    static constexpr qint32 INVALID_PAGE_ID = -1;

    class Page
    {
    public:
        Page(QSharedPointer<Storage::Block> block);
        Page(page_id_t);
        virtual ~Page() = 0;
        page_id_t getId() const;
        virtual QSharedPointer<Storage::Block> toBlock() = 0;

    protected:
        page_id_t id;

    };

    struct rowId
    {
        page_id_t p_id;
        slot_id_t s_id;
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
        virtual bool deleteRecord(const slot_id_t&) = 0;
        virtual QByteArray findRecord(const slot_id_t&) = 0;
        virtual quint8 getFreeSpace() const = 0;

    };

    class UnpackedDataPage : public DataPage
    {
    public:
        UnpackedDataPage(QSharedPointer<Storage::Block> block);
        UnpackedDataPage(int, int);
        bool addRecord(const Core::Record&) override;
        bool deleteRecord(const slot_id_t&) override;
        QByteArray findRecord(const slot_id_t&) override;
        quint8 getFreeSpace() const override;
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
        bool deleteRecord(const slot_id_t&) override;
        QByteArray findRecord(const slot_id_t&) override;
        quint8 getFreeSpace() const override;
        QSharedPointer<Storage::Block> toBlock() override;

    private:
        quint16 numberOfSlots;
        QPair<quint16, quint16> freeSpacePointer;
        QList<slotEntry> slotArray;
        QByteArray data;

    };

    class FreePage : public Page
    {
    public:
        FreePage(page_id_t id) : Page(id) {}
        QSharedPointer<Storage::Block> toBlock() override {
            // Since this is a free page, it simply returns a null pointer or an empty implementation,
            // although it is expected this page type will never be written to disk
            return QSharedPointer<Storage::Block>(nullptr);
        }

    };

    // index pages TODO:
    // https://github.com/zhiyiYo/cmu15445-fall2020/blob/4a07c246ba1d1d47e31f4306ff8b7bbda0dbd04e/src/include/storage/page/b_plus_tree_page.h#L23
    class IndexPage : public virtual Page
    {

    };

    class InternalIndexPage : public IndexPage
    {

    };

    class LeafIndexPage : public IndexPage
    {

    };

}

#endif // PAGE_H
