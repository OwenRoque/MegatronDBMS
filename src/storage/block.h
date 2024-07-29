#ifndef BLOCK_H
#define BLOCK_H

#include "sector.h"
#include <QByteArray>

namespace Storage
{
    using block_id_t = qint32;

    enum class BlockType : quint8
    {
        FreePage,
        InvalidType,
        UnpackedPage,
        SlottedPage,
        BPT_InternalPage,
        BPT_LeafPage
    };

    class Block : public Utility
    {
    public:
        Block() = default;
        ~Block() = default;
        Block(block_id_t blockId, const QByteArray &data);
        // Utility interface
        Space getSpace() const override;
        // getters
        block_id_t getId() const;
        BlockType getHeader() const;
        QByteArray getData() const;
        QSharedPointer<Sector> getSector(int);
        void setId(block_id_t);
        void setHeader(const BlockType&);
        void setData(const QByteArray&);
        void setSectors(QList<QSharedPointer<Sector>> sec);

    private:
        // header data
        BlockType blockType;
        // payload
        QByteArray data;
        // metadata
        qint64 address;
        block_id_t id;
        QList<QSharedPointer<Sector>> sectors;

    };
}

#endif // BLOCK_H
