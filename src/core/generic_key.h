    #ifndef GENERIC_KEY_H
#define GENERIC_KEY_H

#include "megatron_types.h"

namespace Core
{
    template <Types::DataType T>
    class GenericKey
    {
    public:
        using ValueType = typename Types::DTAlias<T>::type;

        /**
         * @brief setValue (setter)
         */
        inline void setValue(const ValueType& v)
        {
            value = v;
        }
        /**
         * @return value (getter)
         */
        inline ValueType getValue() const
        {
            return value;
        }

    private:
        ValueType value;

    };
}

#endif // GENERIC_KEY_H
