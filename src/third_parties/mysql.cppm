module;

#include <cpx/sql/sql.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/serde/error.h>
#include <cpx/module.h>

#include <cstring>
#include <mysql/mysql.h>

export module cpx.mysql;
export import cpx.sql;

extern "C++" {
#include "cpx/sql/mysql.h"
}
