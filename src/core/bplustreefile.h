#ifndef BPLUSTREEFILE_H
#define BPLUSTREEFILE_H

#include "file.h"

namespace Core
{
    class BPlusTreeFile : public File
    {
    public:
        BPlusTreeFile(const QString& relationName, bool firstInit = false);
        Types::Return insertRecord() override;
        // TODO:
        Types::Return bulkInsertRecords(const QString&) override;
        Types::Return deleteRecord() override;
        bool autogrow() override;
    private:
        // ClusteredIndex index;
    };
}
#endif // BPLUSTREEFILE_H
