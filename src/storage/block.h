#ifndef BLOCK_H
#define BLOCK_H

#include "sector.h"
#include <QByteArray>

namespace Storage
{
    using block_id_t = qint32;

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
        struct Header
        {
            // data = d (fixed/variable),
            // index = i (leaf/internal/root included)
            // free = f
            enum BlockType : quint8
            {
                Free,
                DataFixed,
                DataVariable,
                IndexInternal,
                IndexLeaf
            };
            BlockType type;
        };
        Header getHeader() const;
        QByteArray getData() const;
        QSharedPointer<Sector> getSector(int);
        void setId(block_id_t);
        void setHeader(const Header::BlockType&);
        void setData(const QByteArray&);
        void setSectors(QList<QSharedPointer<Sector>> sec);

    private:
        block_id_t id;
        qint64 address;
        Header header;
        QByteArray data;
        QList<QSharedPointer<Sector>> sectors;

    };
}

#endif // BLOCK_H
