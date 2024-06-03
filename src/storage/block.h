#ifndef BLOCK_H
#define BLOCK_H

#include "sector.h"
#include <QByteArray>

namespace Storage
{
    class Block : public Utility
    {
    public:
        Block() = default;
        ~Block() = default;
        Block(int blockAddress, QByteArray &data);
        void setSectors(QList<QSharedPointer<Sector>> sec);
        // Utility interface
        Space getSpace() const override;
        // getters
        int getBlockId() const;
        struct Header
        {
            // data = d (fixed/variable),
            // index = i (leaf/internal/root included)
            // free = f
            enum BlockType : quint8
            {
                DataFixed,
                DataVariable,
                IndexInternal,
                IndexLeaf,
                Free
            };
            BlockType type;
        };
        Header getHeader() const;
        QByteArray getData() const;
        void setHeader(Header::BlockType);
        void setData(const QByteArray&);
        QSharedPointer<Sector> getSector(int index);

    private:
        int blockId;
        Header header;
        QByteArray data;
        QList<QSharedPointer<Sector>> sectors;

    };
}

#endif // BLOCK_H
