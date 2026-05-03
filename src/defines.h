#pragma once
#include <node_api.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#define do_return(...) do { __VA_ARGS__; return; } while (0)

#define VA_ARG_CALL(last_arg, call) \
do { \
    va_list ap; \
    va_start(ap, last_arg); \
    call; \
    va_end(ap); \
} while (0)

#define _BEGIN_EXTERN_C extern "C" {
#define _END_EXTERN_C }

static inline napi_value _znode_str_latin1(napi_env env, const char *s)
    { napi_value v; napi_create_string_latin1(env, s, NAPI_AUTO_LENGTH, &v); return v; }
static inline napi_value _znode_str_utf8(napi_env env, const char *s)
    { napi_value v; napi_create_string_utf8(env, s ? s : "", NAPI_AUTO_LENGTH, &v); return v; }
static inline napi_value _znode_int32(napi_env env, int32_t n)
    { napi_value v; napi_create_int32(env, n, &v); return v; }
static inline napi_value _znode_double(napi_env env, double d)
    { napi_value v; napi_create_double(env, d, &v); return v; }
static inline napi_value _znode_uint32(napi_env env, uint32_t n)
    { napi_value v; napi_create_uint32(env, n, &v); return v; }
static inline napi_value _znode_null(napi_env env)
    { napi_value v; napi_get_null(env, &v); return v; }
static inline napi_value _znode_undefined(napi_env env)
    { napi_value v; napi_get_undefined(env, &v); return v; }
static inline napi_value _znode_bool(napi_env env, bool b)
    { napi_value v; napi_get_boolean(env, b, &v); return v; }

#define ZNODE_ASCII(str)  _znode_str_latin1(env, str)
#define ZNODE_UTF8(str)   _znode_str_utf8(env, str)
#define ZNODE_INT32(n)    _znode_int32(env, (int32_t)(n))
#define ZNODE_INT64(d)    _znode_double(env, (double)(d))
#define ZNODE_UINT32(n)   _znode_uint32(env, (uint32_t)(n))
#define ZNODE_NULL        _znode_null(env)
#define ZNODE_UNDEFINED   _znode_undefined(env)
#define ZNODE_TRUE        _znode_bool(env, true)
#define ZNODE_FALSE       _znode_bool(env, false)

#define ZNODE_ERROR(message) \
    do { napi_throw_error(env, NULL, message); return NULL; } while(0)

static inline void znode_error(napi_env env, int last_err, const char *fmt, ...)
{
    int err_code = GetLastError(), len;
    char buf[1024];
    VA_ARG_CALL(fmt, (len = vsnprintf(buf, sizeof(buf), fmt, ap)));
    if (last_err)
    {
        wchar_t *msg = nullptr;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&msg, 0, NULL);
        snprintf(buf + len, sizeof(buf) - len, " %ls", msg);
        LocalFree(msg);
    }
    napi_throw_error(env, NULL, buf);
}
#define ZNODE_ERROR_FMT(last_err, fmt, ...) \
    do { znode_error(env, last_err, fmt, __VA_ARGS__); return NULL; } while(0)

#define ZNODE_CHECK(condition, message) \
    do { if (!(condition)) { napi_throw_error(env, NULL, message); return NULL; } } while(0)

#define ZNODE_CHECK_ERROR(condition, message)     ZNODE_CHECK(condition, message)
#define ZNODE_CHECK_TYPEERROR(condition, message) ZNODE_CHECK(condition, message)

#define ZNODE_CHECK_TYPEERROR_BOOL(value) \
    { napi_valuetype _t; napi_typeof(env, value, &_t); \
      ZNODE_CHECK(_t == napi_boolean, "Wrong type of value " #value ": boolean expected"); }

#define ZNODE_CHECK_TYPEERROR_STRING(value) \
    { napi_valuetype _t; napi_typeof(env, value, &_t); \
      ZNODE_CHECK(_t == napi_string, "Wrong type of value " #value ": string expected"); }

#define ZNODE_DECLARE_FUNCTION(name) \
    { napi_value _fn; \
      napi_create_function(env, #name, NAPI_AUTO_LENGTH, znode_##name, NULL, &_fn); \
      napi_set_named_property(env, exports, #name, _fn); }

#define ZNODE_FUNCTION(name) \
    static napi_value znode_##name(napi_env env, napi_callback_info info)

#define ZNODE_INIT_FUNCTION(name) \
    static napi_value name(napi_env env, napi_value exports)

#define ZNODE_MAX_ARGS 8

#define ZNODE_ARG_BEGIN_NOCONTEXT \
    size_t argc = ZNODE_MAX_ARGS; \
    napi_value args[ZNODE_MAX_ARGS] = {}; \
    napi_get_cb_info(env, info, &argc, args, NULL, NULL); \
    int arg_index = 0;

#define ZNODE_ARG_BEGIN ZNODE_ARG_BEGIN_NOCONTEXT

#define ZNODE_ARG_END \
    ZNODE_CHECK((int)argc == arg_index, "Wrong number of arguments")

#define ZNODE_ARG_BOOL(name) \
    bool name = false; napi_get_value_bool(env, args[arg_index++], &name)

#define ZNODE_ARG_INT(name) \
    { napi_valuetype _t; napi_typeof(env, args[arg_index], &_t); \
      ZNODE_CHECK(_t == napi_number, "Wrong type of argument " #name ": integer expected"); } \
    int32_t name = 0; napi_get_value_int32(env, args[arg_index++], &name)

#define ZNODE_ARG_STRING(name) \
    { napi_valuetype _t; napi_typeof(env, args[arg_index], &_t); \
      ZNODE_CHECK(_t == napi_string, "Wrong type of argument " #name ": string expected"); } \
    char name##_buf[4096] = {}; \
    size_t name##_len = 0; \
    napi_get_value_string_utf8(env, args[arg_index++], name##_buf, sizeof(name##_buf), &name##_len); \
    char *name = name##_buf

#define ZNODE_ARG_OBJECT_OPT(name) \
    napi_value name = NULL; \
    if ((int)argc > arg_index) { \
        napi_valuetype _t_##name; \
        napi_typeof(env, args[arg_index], &_t_##name); \
        if (_t_##name == napi_object) name = args[arg_index++]; \
    }

#define ZNODE_ARG_FUNCTION(name) \
    { napi_valuetype _t; napi_typeof(env, args[arg_index], &_t); \
      ZNODE_CHECK(_t == napi_function, "Wrong type of argument " #name ": function expected"); } \
    napi_value name##_v8 = args[arg_index++]

#define ZNODE_RETURN(value) \
    do { napi_value _rv = (value); return _rv; } while(0)

#define ZNODE_BEGIN_INIT
#define ZNODE_BEGIN