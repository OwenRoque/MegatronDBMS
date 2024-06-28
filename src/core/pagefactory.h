#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include <QSharedPointer>
#include "page.h"

namespace Core
{
    class PageFactory
    {
    public:
        virtual QSharedPointer<Page> createPage(QSharedPointer<Storage::Block> block) const = 0;
        virtual QSharedPointer<Page> createPage(Storage::Block::Header::BlockType type, int pageId) const = 0;
        virtual QSharedPointer<Page> createPage(Storage::Block::Header::BlockType type, int pageId, int recordSize) const = 0;
    };

    class DataPageFactory : public PageFactory
    {
        QSharedPointer<Page> createPage(QSharedPointer<Storage::Block> block) const override {
            auto header = block->getHeader();
            if (header.type == Storage::Block::Header::DataFixed) {
                return QSharedPointer<UnpackedDataPage>::create(block);
            } else if (header.type == Storage::Block::Header::DataVariable) {
                return QSharedPointer<SlottedPage>::create(block);
            }
            return nullptr;
        }
        QSharedPointer<Page> createPage(Storage::Block::Header::BlockType type, int pageId) const override {
            if (type == Storage::Block::Header::DataVariable) {
                return QSharedPointer<SlottedPage>::create(pageId);
            }
            return nullptr;
        }
        QSharedPointer<Page> createPage(Storage::Block::Header::BlockType type, int pageId, int recordSize) const override {
            if (type == Storage::Block::Header::DataFixed) {
                return QSharedPointer<UnpackedDataPage>::create(pageId, recordSize);
            }
            return nullptr;
        }
    };

    // define IndexPageFactory for index pages
    class IndexPageFactory : public PageFactory {
    public:
        QSharedPointer<Page> createPage(QSharedPointer<Storage::Block> block) const override {
            auto header = block->getHeader();
            // if (header.type == Storage::Block::Header::IndexInternal) {
            //     return QSharedPointer<IndexInternalPage>::create(block);
            // } else if (header.type == Storage::Block::Header::IndexLeaf) {
            //     return QSharedPointer<IndexLeafPage>::create(block);
            // }
            return nullptr;
        }
    };
}

#endif // PAGEFACTORY_H
