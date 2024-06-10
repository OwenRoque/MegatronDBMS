#ifndef RECORD_H
#define RECORD_H

#include <QByteArray>
#include <QBitArray>
#include <QDataStream>
#include <QVariant>
#include <QDebug>

namespace Core
{
    class Record
    {
    public:
        virtual ~Record() = default;
        virtual QVariant getField(const QString&, int) const = 0;
        virtual bool setField(const QString&, int, const QVariant&) = 0;
        virtual int size() const = 0;
        virtual QByteArray toBytes() const = 0;
    protected:
        QByteArray data;

    };

    class FLRecord : public Record
    {
    public:
        FLRecord(const QString&, const QStringList&);
        FLRecord(const QString&, const QByteArray&);
        QVariant getField(const QString&, int) const override;
        bool setField(const QString&, int, const QVariant&) override;
        int size() const override;
        QByteArray toBytes() const override;

    };

    class VLRecord : public Record
    {
    public:
        VLRecord(const QString&, const QStringList&);
        VLRecord(const QString&, const QByteArray&);
        QVariant getField(const QString&, int) const override;
        bool setField(const QString&, int, const QVariant&) override;
        int size() const override;
        QByteArray toBytes() const override;

    private:
        QBitArray nullBitmap;

    };
}

#endif // RECORD_H
