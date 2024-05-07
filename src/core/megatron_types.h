#ifndef MEGATRON_TYPES_H
#define MEGATRON_TYPES_H

#include "qobjectdefs.h"
#include "qtmetamacros.h"

namespace Types
{
    Q_NAMESPACE
    enum Return {
        OpenError,
        ParseError,
        Success
    };
    Q_ENUM_NS(Return)

    enum QueryClauses {
        SelectAll = 'A',
        SelectCustom = 'C',
        Where = 'W',
        SelectInto = 'I'
    };
    Q_ENUM_NS(QueryClauses)
}

#endif // MEGATRON_TYPES_H
