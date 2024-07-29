#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include <QSharedPointer>
#include "page.h"

namespace Core
{
    class PageFactory
    {
    public:
        // constructor for non-empty pages (data pages or index pages)
        // TODO: add index pages standart constructors here
        static QSharedPointer<Page> createPage(QSharedPointer<Storage::Block> block) {
            auto header = block->getHeader();
            switch (header) {
            case Storage::BlockType::UnpackedPage:
                return QSharedPointer<Page>(new UnpackedDataPage(block));
            case Storage::BlockType::SlottedPage:
                return QSharedPointer<Page>(new SlottedPage(block));
            case Storage::BlockType::FreePage:
                return QSharedPointer<Page>(new FreePage(block->getId()));
            default:
                throw std::invalid_argument("Tipo de bloque no soportado");
            }
        }

        // constructor for empty variable-length page
        static QSharedPointer<Page> createSlottedPage(int pageId) {
            return QSharedPointer<SlottedPage>::create(pageId);
        }

        // constructor for empty fixed-length page
        static QSharedPointer<Page> createUnpackedPage(int pageId, int recordSize) {
            return QSharedPointer<UnpackedDataPage>::create(pageId, recordSize);
        }

        // add more constructor for index pages here...
    };

}

#endif // PAGEFACTORY_H
