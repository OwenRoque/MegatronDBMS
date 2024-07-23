#ifndef GENERIC_COMPARATOR_H
#define GENERIC_COMPARATOR_H

#include <QList>
#include <QString>
#include "generic_key.h"

namespace Core
{
    template <typename T>
    int compare(const T& lhs, const T& rhs) {
        if (lhs < rhs) {
            return -1;
        }
        if (lhs > rhs) {
            return 1;
        }
        return 0;
    }

    template<>
    int compare<QString>(const QString& lhs, const QString& rhs) {
        return lhs.compare(rhs);
    }

    template <typename::Types::DataType T>
    class GenericComparator
    {
    public:
        // copy constructor
        GenericComparator(const GenericComparator& other) : attributeNames(other.attributeNames) {}
        // explicit constuctor
        explicit GenericComparator(const QList<QString>& keySchema) : attributeNames(keySchema) {}
        inline int operator()(const GenericKey<T>& lhs, const GenericKey<T>& rhs) const
        {
            const auto &lhs_value = lhs.GetValue();
            const auto &rhs_value = rhs.GetValue();
            return Compare(lhs_value, rhs_value);
        }

    private:
        QList<QString> attributeNames;

    };
}


#endif // GENERIC_COMPARATOR_H
