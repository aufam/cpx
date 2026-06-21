module;

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/time.h>
#include <cstring>
#include <netinet/in.h>

#if __has_include(<postgresql/libpq-fe.h>)
#    include <postgresql/libpq-fe.h>
#elif __has_include(<libpq-fe.h>)
#    include <libpq-fe.h>
#else
#    error "Cannot find libpq-fe.h"
#endif

export module cpx.postgres;
export import cpx.sql;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/sql/postgres.h"
}
