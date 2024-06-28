#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include "page.h"

namespace Memory {

class BufferManager : public QObject
{
    Q_OBJECT
public:
    static BufferManager& getInstance(int size = 0)
    {
        static BufferManager singleton(size);
        return singleton;
    }
    int getPoolSize() const;
    // QSharedPointer<Core::Page>;

private:
    BufferManager(int size = 0);

    Q_DISABLE_COPY(BufferManager);
};

} // namespace Memory

#endif // BUFFERMANAGER_H
