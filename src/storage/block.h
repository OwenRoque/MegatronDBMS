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
        Block(int blockAddress, const QByteArray &data);
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
        void setId(int);
        void setHeader(const Header::BlockType&);
        void setData(const QByteArray&);
        void setSectors(QList<QSharedPointer<Sector>> sec);

    private:
        int blockId;
        Header header;
        QByteArray data;
        QList<QSharedPointer<Sector>> sectors;

    };
}

#endif // BLOCK_H
