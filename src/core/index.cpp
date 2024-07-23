#include "index.h"

Core::Index::Index(const QList<SystemCatalog::indexMeta> &metadata)
    : metadata(metadata) {}

QList<Core::SystemCatalog::indexMeta> Core::Index::getMetaData() const
{
    return metadata;
}

int Core::Index::getIndexColumnCount() const
{
    return metadata.size();
}

QString Core::Index::getIndexName() const
{
    return metadata.at(0).indexName;
}
