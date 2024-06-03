#include "file.h"

Core::File::File(QSharedPointer<SystemCatalog> sc, QSharedPointer<DiskManager> dm) : syscat(sc), sm(dm) {}
