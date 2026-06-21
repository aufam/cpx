module;

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cstring>
#include <mysql/mysql.h>

export module cpx.mysql;
export import cpx.sql;

#undef CPX_EXPORT
#define CPX_EXPORT export

extern "C++" {
#include "cpx/sql/mysql.h"
}
