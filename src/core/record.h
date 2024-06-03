#ifndef RECORD_H
#define RECORD_H

#include <QByteArray>
#include <QBitArray>
#include <QVariant>
#include <QDebug>

namespace Core
{
    class Record
    {
    public:
        virtual ~Record() = default;
        virtual QVariant getField(int index) const = 0;
        virtual bool setField(int index, const QVariant& value) = 0;
        virtual int size() const = 0;
    protected:
        QByteArray data;

    };

    class FLRecord : public Record
    {
    public:
        FLRecord(const QString& relationName, const QStringList& unformatted);
        QVariant getField(int index) const override;
        bool setField(int index, const QVariant& value) override;
        int size() const override;

    };

    class VLRecord : public Record
    {
    public:
        VLRecord(const QString& relationName, const QStringList& data);
        QVariant getField(int index) const override;
        bool setField(int index, const QVariant& value) override;
        int size() const override;

    private:
        QBitArray nullBitmap;

    };
}

#endif // RECORD_H
