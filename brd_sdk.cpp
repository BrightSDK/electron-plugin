#include <node_api.h>
#include <windows.h>
#include "defines.h"

typedef void (WINAPI *brd_sdk_choice_change_t)(int);
typedef void (WINAPI *brd_sdk_service_status_change_t)(int);
typedef void (WINAPI *brd_sdk_on_dialog_shown_t)(void);
typedef void (WINAPI *brd_sdk_on_dialog_closed_t)(void);

typedef int (WINAPI *brd_sdk_is_supported_t)(void);
typedef void (WINAPI *brd_sdk_init_t)(void);
typedef void (WINAPI *brd_sdk_show_consent_t)(void);
typedef void (WINAPI *brd_sdk_opt_out_t)(void);
typedef void (WINAPI *brd_sdk_opt_in_t)(void);
typedef int (WINAPI *brd_sdk_get_consent_choice_t)(void);
typedef void (WINAPI *brd_sdk_close_t)(void);
typedef void (WINAPI *brd_sdk_set_choice_change_cb_t)(
    brd_sdk_choice_change_t);
typedef void (WINAPI *brd_sdk_set_skip_consent_on_init_t)(BOOLEAN);
typedef void (WINAPI *brd_sdk_set_service_status_change_cb_t)(
    brd_sdk_service_status_change_t);
typedef void (WINAPI *brd_sdk_set_on_dialog_shown_cb_t)(
    brd_sdk_on_dialog_shown_t);
typedef void (WINAPI *brd_sdk_set_on_dialog_closed_cb_t)(
    brd_sdk_on_dialog_closed_t);
typedef void (WINAPI *brd_sdk_fix_service_status_t)(void);
typedef void (WINAPI *brd_sdk_set_service_auto_start_t)(int);
typedef void (WINAPI *brd_sdk_stop_service_t)(void);
typedef void (WINAPI *brd_sdk_start_service_t)(void);
typedef void (WINAPI *brd_sdk_notify_install_t)(void);
typedef void (WINAPI *brd_sdk_set_appid_t)(char *);
typedef void (WINAPI *brd_sdk_set_app_name_t)(char *);
typedef void (WINAPI *brd_sdk_set_lang_t)(char *);
typedef void (WINAPI *brd_sdk_set_logo_link_t)(char *);
typedef void (WINAPI *brd_sdk_set_benefit_txt_t)(char *);
typedef void (WINAPI *brd_sdk_set_benefit_t)(char *);
typedef void (WINAPI *brd_sdk_set_bg_color_t)(char *);
typedef void (WINAPI *brd_sdk_set_btn_color_t)(char *);
typedef void (WINAPI *brd_sdk_set_txt_color_t)(char *);
typedef void (WINAPI *brd_sdk_set_app_name_color_t)(char *);
typedef void (WINAPI *brd_sdk_set_agree_btn_t)(char *);
typedef void (WINAPI *brd_sdk_set_disagree_btn_t)(char *);
typedef void (WINAPI *brd_sdk_set_campaign_t)(char *);
typedef char *(WINAPI *brd_sdk_get_uuid_t)(void);
typedef void (WINAPI *brd_sdk_set_tracking_id_t)(char* tracking_id);
typedef char *(WINAPI *brd_sdk_get_tracking_id_t)(void);

static brd_sdk_is_supported_t p_brd_sdk_is_supported;
static brd_sdk_init_t p_brd_sdk_init;
static brd_sdk_show_consent_t p_brd_sdk_show_consent;
static brd_sdk_opt_out_t p_brd_sdk_opt_out;
static brd_sdk_opt_in_t p_brd_sdk_opt_in;
static brd_sdk_get_consent_choice_t p_brd_sdk_get_consent_choice;
static brd_sdk_close_t p_brd_sdk_close;
static brd_sdk_set_choice_change_cb_t p_brd_sdk_set_choice_change_cb;
static brd_sdk_set_skip_consent_on_init_t p_brd_sdk_set_skip_consent_on_init;
static brd_sdk_set_service_status_change_cb_t p_brd_sdk_set_service_status_change_cb;
static brd_sdk_set_on_dialog_shown_cb_t p_brd_sdk_set_on_dialog_shown_cb;
static brd_sdk_set_on_dialog_closed_cb_t p_brd_sdk_set_on_dialog_closed_cb;
static brd_sdk_fix_service_status_t p_brd_sdk_fix_service_status;
static brd_sdk_set_service_auto_start_t p_brd_sdk_set_service_auto_start;
static brd_sdk_stop_service_t p_brd_sdk_stop_service;
static brd_sdk_start_service_t p_brd_sdk_start_service;
static brd_sdk_notify_install_t p_brd_sdk_notify_install;
static brd_sdk_set_appid_t p_brd_sdk_set_appid;
static brd_sdk_set_app_name_t p_brd_sdk_set_app_name;
static brd_sdk_set_lang_t p_brd_sdk_set_lang;
static brd_sdk_set_logo_link_t p_brd_sdk_set_logo_link;
static brd_sdk_set_benefit_txt_t p_brd_sdk_set_benefit_txt;
static brd_sdk_set_benefit_t p_brd_sdk_set_benefit;
static brd_sdk_set_bg_color_t p_brd_sdk_set_bg_color;
static brd_sdk_set_btn_color_t p_brd_sdk_set_btn_color;
static brd_sdk_set_txt_color_t p_brd_sdk_set_txt_color;
static brd_sdk_set_app_name_color_t p_brd_sdk_set_app_name_color;
static brd_sdk_set_agree_btn_t p_brd_sdk_set_agree_btn;
static brd_sdk_set_disagree_btn_t p_brd_sdk_set_disagree_btn;
static brd_sdk_set_campaign_t p_brd_sdk_set_campaign;
static brd_sdk_get_uuid_t p_brd_sdk_get_uuid;
static brd_sdk_set_tracking_id_t p_brd_sdk_set_tracking_id;
static brd_sdk_get_tracking_id_t p_brd_sdk_get_tracking_id;
static int brd_sdk_loaded;

ZNODE_FUNCTION(load)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(path);
    ZNODE_ARG_END;
    SetErrorMode(0);
    HMODULE h = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!h)
        ZNODE_ERROR_FMT(1, "brd_sdk failed loading dll (%s):", path);
#define LOAD_PROC(fn, fn_sym) \
    if (!(p_##fn = (fn##_t)GetProcAddress(h, fn_sym))) \
        ZNODE_ERROR_FMT(1, "brd_sdk failed loading func %s (%s):", #fn, fn_sym)
#define LOAD_PROC_OPT(fn, fn_sym) \
    p_##fn = (fn##_t)GetProcAddress(h, fn_sym)
#if _WIN64
    LOAD_PROC(brd_sdk_is_supported, "lum_sdk_is_supported_c");
    LOAD_PROC(brd_sdk_init, "brd_sdk_init_c");
    LOAD_PROC(brd_sdk_show_consent, "brd_sdk_show_consent_c");
    LOAD_PROC(brd_sdk_opt_out, "brd_sdk_opt_out_c");
    LOAD_PROC(brd_sdk_opt_in, "brd_sdk_opt_in_c");
    LOAD_PROC(brd_sdk_get_consent_choice, "brd_sdk_get_consent_choice_c");
    LOAD_PROC(brd_sdk_close, "brd_sdk_close_c");
    LOAD_PROC(brd_sdk_set_choice_change_cb,
        "brd_sdk_set_choice_change_cb_c");
    LOAD_PROC(brd_sdk_set_skip_consent_on_init,
        "brd_sdk_set_skip_consent_on_init_c");
    LOAD_PROC(brd_sdk_set_service_status_change_cb,
        "brd_sdk_set_service_status_change_cb_c");
    LOAD_PROC(brd_sdk_set_on_dialog_shown_cb,
        "brd_sdk_set_on_dialog_shown_cb_c");
    LOAD_PROC(brd_sdk_set_on_dialog_closed_cb,
        "brd_sdk_set_on_dialog_closed_cb_c");
    LOAD_PROC(brd_sdk_fix_service_status, "brd_sdk_fix_service_status_c");
    LOAD_PROC(brd_sdk_set_service_auto_start,
        "brd_sdk_set_service_auto_start_c");
    LOAD_PROC(brd_sdk_stop_service, "brd_sdk_stop_service_c");
    LOAD_PROC(brd_sdk_start_service, "brd_sdk_start_service_c");
    LOAD_PROC(brd_sdk_notify_install, "brd_sdk_notify_install_c");
    LOAD_PROC(brd_sdk_set_appid, "brd_sdk_set_appid_c");
    LOAD_PROC(brd_sdk_set_app_name, "brd_sdk_set_app_name_c");
    LOAD_PROC(brd_sdk_set_lang, "brd_sdk_set_lang_c");
    LOAD_PROC(brd_sdk_set_logo_link, "brd_sdk_set_logo_link_c");
    LOAD_PROC(brd_sdk_set_benefit_txt, "brd_sdk_set_benefit_txt_c");
    LOAD_PROC(brd_sdk_set_benefit, "brd_sdk_set_benefit_c");
    LOAD_PROC(brd_sdk_set_bg_color, "brd_sdk_set_bg_color_c");
    LOAD_PROC(brd_sdk_set_btn_color, "brd_sdk_set_btn_color_c");
    LOAD_PROC(brd_sdk_set_txt_color, "brd_sdk_set_txt_color_c");
    LOAD_PROC(brd_sdk_set_app_name_color, "brd_sdk_set_app_name_color_c");
    LOAD_PROC(brd_sdk_set_agree_btn, "brd_sdk_set_agree_btn_c");
    LOAD_PROC(brd_sdk_set_disagree_btn, "brd_sdk_set_disagree_btn_c");
    LOAD_PROC(brd_sdk_set_campaign, "brd_sdk_set_campaign_c");
    LOAD_PROC(brd_sdk_get_uuid, "brd_sdk_get_uuid_c");
    LOAD_PROC(brd_sdk_get_tracking_id, "brd_sdk_get_tracking_id_c");
    LOAD_PROC(brd_sdk_set_tracking_id, "brd_sdk_set_tracking_id_c");
#else
    LOAD_PROC(brd_sdk_is_supported, "_lum_sdk_is_supported_c@0");
    LOAD_PROC(brd_sdk_init, "_brd_sdk_init_c@0");
    LOAD_PROC(brd_sdk_show_consent, "_brd_sdk_show_consent_c@0");
    LOAD_PROC(brd_sdk_opt_out, "_brd_sdk_opt_out_c@0");
    LOAD_PROC(brd_sdk_opt_in, "_brd_sdk_opt_in_c@0");
    LOAD_PROC(brd_sdk_get_consent_choice, "_brd_sdk_get_consent_choice_c@0");
    LOAD_PROC(brd_sdk_close, "_brd_sdk_close_c@0");
    LOAD_PROC(brd_sdk_set_choice_change_cb,
        "_brd_sdk_set_choice_change_cb_c@4");
    LOAD_PROC(brd_sdk_set_skip_consent_on_init,
        "_brd_sdk_set_skip_consent_on_init_c@4");
    LOAD_PROC(brd_sdk_set_service_status_change_cb,
        "_brd_sdk_set_service_status_change_cb_c@4");
    LOAD_PROC(brd_sdk_set_on_dialog_shown_cb,
        "_brd_sdk_set_on_dialog_shown_cb_c@4");
    LOAD_PROC(brd_sdk_set_on_dialog_closed_cb,
        "_brd_sdk_set_on_dialog_closed_cb_c@4");
    LOAD_PROC(brd_sdk_fix_service_status, "_brd_sdk_fix_service_status_c@0");
    LOAD_PROC(brd_sdk_set_service_auto_start,
        "_brd_sdk_set_service_auto_start_c@4");
    LOAD_PROC(brd_sdk_stop_service, "_brd_sdk_stop_service_c@0");
    LOAD_PROC(brd_sdk_start_service, "_brd_sdk_start_service_c@0");
    LOAD_PROC(brd_sdk_notify_install, "_brd_sdk_notify_install_c@0");
    LOAD_PROC(brd_sdk_set_appid, "_brd_sdk_set_appid_c@4");
    LOAD_PROC(brd_sdk_set_app_name, "_brd_sdk_set_app_name_c@4");
    LOAD_PROC(brd_sdk_set_lang, "_brd_sdk_set_lang_c@4");
    LOAD_PROC(brd_sdk_set_logo_link, "_brd_sdk_set_logo_link_c@4");
    LOAD_PROC(brd_sdk_set_benefit_txt, "_brd_sdk_set_benefit_txt_c@4");
    LOAD_PROC(brd_sdk_set_benefit, "_brd_sdk_set_benefit_c@4");
    LOAD_PROC(brd_sdk_set_bg_color, "_brd_sdk_set_bg_color_c@4");
    LOAD_PROC(brd_sdk_set_btn_color, "_brd_sdk_set_btn_color_c@4");
    LOAD_PROC(brd_sdk_set_txt_color, "_brd_sdk_set_txt_color_c@4");
    LOAD_PROC(brd_sdk_set_app_name_color, "_brd_sdk_set_app_name_color_c@4");
    LOAD_PROC(brd_sdk_set_agree_btn, "_brd_sdk_set_agree_btn_c@4");
    LOAD_PROC(brd_sdk_set_disagree_btn, "_brd_sdk_set_disagree_btn_c@4");
    LOAD_PROC(brd_sdk_set_campaign, "_brd_sdk_set_campaign_c@4");
    LOAD_PROC(brd_sdk_get_uuid, "_brd_sdk_get_uuid_c@0");
    LOAD_PROC(brd_sdk_get_tracking_id, "_brd_sdk_get_tracking_id_c@0");
    LOAD_PROC(brd_sdk_set_tracking_id, "_brd_sdk_set_tracking_id_c@4");
#endif
    brd_sdk_loaded = 1;
    return ZNODE_INT32(1);
}

ZNODE_FUNCTION(init)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(appid);
    ZNODE_ARG_OBJECT_OPT(opt);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_appid(appid);
    if (!opt)
        goto Exit;
    {
        bool has = false;
        napi_has_named_property(env, opt, "skip_consent", &has);
        if (has)
        {
            napi_value v; napi_get_named_property(env, opt, "skip_consent", &v);
            bool b = false; napi_get_value_bool(env, v, &b);
            p_brd_sdk_set_skip_consent_on_init(b);
        }
    }
#define SET_OPT(n) \
    { bool has = false; napi_has_named_property(env, opt, #n, &has); \
      if (has) { \
        napi_value v; napi_get_named_property(env, opt, #n, &v); \
        char n##_buf[4096]; size_t n##_len; \
        napi_get_value_string_utf8(env, v, n##_buf, sizeof(n##_buf), &n##_len); \
        p_brd_sdk_set_##n(n##_buf); \
      } }
    SET_OPT(app_name);
    SET_OPT(lang);
    SET_OPT(logo_link);
    SET_OPT(benefit_txt);
    SET_OPT(benefit);
    SET_OPT(bg_color);
    SET_OPT(btn_color);
    SET_OPT(txt_color);
    SET_OPT(agree_btn);
    SET_OPT(disagree_btn);
    SET_OPT(campaign);
#undef SET_OPT
Exit:
    p_brd_sdk_init();
    return ZNODE_INT32(0);
}

// ─── Async init ───────────────────────────────────────────────────────────────
// Runs the blocking p_brd_sdk_init() on a libuv thread-pool thread so the
// Node.js event loop (and Electron UI) stays responsive during the ~5-8s init.

struct AsyncInitData {
    napi_async_work work;
    napi_deferred   deferred;
    char            error[512];
};

ZNODE_FUNCTION(init_async)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(appid);
    ZNODE_ARG_OBJECT_OPT(opt);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");

    // Apply all synchronous setters now (safe on main thread before async work).
    p_brd_sdk_set_appid(appid);
    if (opt)
    {
        {
            bool has = false;
            napi_has_named_property(env, opt, "skip_consent", &has);
            if (has)
            {
                napi_value v; napi_get_named_property(env, opt, "skip_consent", &v);
                bool b = false; napi_get_value_bool(env, v, &b);
                p_brd_sdk_set_skip_consent_on_init(b);
            }
        }
#define SET_OPT(n) \
        { bool has = false; napi_has_named_property(env, opt, #n, &has); \
          if (has) { \
            napi_value v; napi_get_named_property(env, opt, #n, &v); \
            char n##_buf[4096]; size_t n##_len; \
            napi_get_value_string_utf8(env, v, n##_buf, sizeof(n##_buf), &n##_len); \
            p_brd_sdk_set_##n(n##_buf); \
          } }
        SET_OPT(app_name);
        SET_OPT(lang);
        SET_OPT(logo_link);
        SET_OPT(benefit_txt);
        SET_OPT(benefit);
        SET_OPT(bg_color);
        SET_OPT(btn_color);
        SET_OPT(txt_color);
        SET_OPT(agree_btn);
        SET_OPT(disagree_btn);
        SET_OPT(campaign);
#undef SET_OPT
    }

    // Create promise + async work.
    napi_value promise;
    AsyncInitData *data = new AsyncInitData{};
    napi_create_promise(env, &data->deferred, &promise);

    napi_value resource_name;
    napi_create_string_utf8(env, "brd_sdk_init", NAPI_AUTO_LENGTH, &resource_name);
    napi_create_async_work(env, NULL, resource_name,
        // execute — runs on libuv thread pool, NO V8/NAPI calls allowed
        [](napi_env, void *raw) {
            AsyncInitData *d = static_cast<AsyncInitData *>(raw);
            __try { p_brd_sdk_init(); }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                snprintf(d->error, sizeof(d->error),
                    "brd_sdk_init raised SEH exception 0x%08lX",
                    GetExceptionCode());
            }
        },
        // complete — back on the main thread, V8/NAPI allowed
        [](napi_env env, napi_status status, void *raw) {
            AsyncInitData *d = static_cast<AsyncInitData *>(raw);
            if (d->error[0]) {
                napi_value err;
                napi_create_string_utf8(env, d->error, NAPI_AUTO_LENGTH, &err);
                napi_reject_deferred(env, d->deferred, err);
            } else {
                napi_resolve_deferred(env, d->deferred, _znode_int32(env, 0));
            }
            napi_delete_async_work(env, d->work);
            delete d;
        },
        data, &data->work);
    napi_queue_async_work(env, data->work);
    return promise;
}

ZNODE_FUNCTION(is_supported)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    napi_value result;
    napi_get_boolean(env, p_brd_sdk_is_supported() != 0, &result);
    return result;
}

ZNODE_FUNCTION(show_consent)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_show_consent();
    return ZNODE_NULL;
}

ZNODE_FUNCTION(get_consent)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    return ZNODE_INT32(p_brd_sdk_get_consent_choice());
}

ZNODE_FUNCTION(opt_out)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_opt_out();
    return ZNODE_NULL;
}

ZNODE_FUNCTION(opt_in)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_opt_in();
    return ZNODE_NULL;
}

ZNODE_FUNCTION(close)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_close();
    ZNODE_RETURN(0);
}

static napi_threadsafe_function choice_change_tsfn;
static void _on_choice_change_cb(napi_env env, napi_value js_cb, void *ctx, void *data)
{
    napi_value argv[1] = {ZNODE_INT32((int)(intptr_t)data)};
    napi_call_function(env, ZNODE_NULL, js_cb, 1, argv, NULL);
}
static void on_choice_change_cb(int choice)
{
    napi_call_threadsafe_function(choice_change_tsfn, (void *)(intptr_t)choice, napi_tsfn_nonblocking);
}

ZNODE_FUNCTION(set_choice_change_cb)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    napi_value cb_v8 = args[arg_index++];
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    napi_valuetype t; napi_typeof(env, cb_v8, &t);
    if (choice_change_tsfn) { napi_release_threadsafe_function(choice_change_tsfn, napi_tsfn_release); choice_change_tsfn = NULL; }
    if (t == napi_function)
    {
        napi_value name; napi_create_string_utf8(env, "choice_change_cb", NAPI_AUTO_LENGTH, &name);
        napi_create_threadsafe_function(env, cb_v8, NULL, name, 0, 1, NULL, NULL, NULL, _on_choice_change_cb, &choice_change_tsfn);
        napi_unref_threadsafe_function(env, choice_change_tsfn);
        p_brd_sdk_set_choice_change_cb((brd_sdk_choice_change_t)on_choice_change_cb);
    }
    else
        p_brd_sdk_set_choice_change_cb(NULL);
    return ZNODE_INT32(0);
}

static napi_threadsafe_function service_status_change_tsfn;
static void _on_service_status_change_cb(napi_env env, napi_value js_cb, void *ctx, void *data)
{
    napi_value argv[1] = {ZNODE_INT32((int)(intptr_t)data)};
    napi_call_function(env, ZNODE_NULL, js_cb, 1, argv, NULL);
}
static void on_service_status_change_cb(int service_status)
{
    napi_call_threadsafe_function(service_status_change_tsfn, (void *)(intptr_t)service_status, napi_tsfn_nonblocking);
}

ZNODE_FUNCTION(set_service_status_change_cb)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    napi_value cb_v8 = args[arg_index++];
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    napi_valuetype t; napi_typeof(env, cb_v8, &t);
    if (service_status_change_tsfn) { napi_release_threadsafe_function(service_status_change_tsfn, napi_tsfn_release); service_status_change_tsfn = NULL; }
    if (t == napi_function)
    {
        napi_value name; napi_create_string_utf8(env, "service_status_change_cb", NAPI_AUTO_LENGTH, &name);
        napi_create_threadsafe_function(env, cb_v8, NULL, name, 0, 1, NULL, NULL, NULL, _on_service_status_change_cb, &service_status_change_tsfn);
        napi_unref_threadsafe_function(env, service_status_change_tsfn);
        p_brd_sdk_set_service_status_change_cb((brd_sdk_service_status_change_t)on_service_status_change_cb);
    }
    else
        p_brd_sdk_set_service_status_change_cb(NULL);
    return ZNODE_INT32(0);
}

static napi_threadsafe_function on_dialog_closed_tsfn;
static void _on_on_dialog_closed_cb(napi_env env, napi_value js_cb, void *ctx, void *data)
{
    napi_call_function(env, ZNODE_NULL, js_cb, 0, NULL, NULL);
}
static void on_on_dialog_closed_cb()
{
    napi_call_threadsafe_function(on_dialog_closed_tsfn, NULL, napi_tsfn_nonblocking);
}

ZNODE_FUNCTION(set_on_dialog_closed_cb)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    napi_value cb_v8 = args[arg_index++];
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    napi_valuetype t; napi_typeof(env, cb_v8, &t);
    if (on_dialog_closed_tsfn) { napi_release_threadsafe_function(on_dialog_closed_tsfn, napi_tsfn_release); on_dialog_closed_tsfn = NULL; }
    if (t == napi_function)
    {
        napi_value name; napi_create_string_utf8(env, "on_dialog_closed_cb", NAPI_AUTO_LENGTH, &name);
        napi_create_threadsafe_function(env, cb_v8, NULL, name, 0, 1, NULL, NULL, NULL, _on_on_dialog_closed_cb, &on_dialog_closed_tsfn);
        napi_unref_threadsafe_function(env, on_dialog_closed_tsfn);
        p_brd_sdk_set_on_dialog_closed_cb((brd_sdk_on_dialog_closed_t)on_on_dialog_closed_cb);
    }
    else
        p_brd_sdk_set_on_dialog_closed_cb(NULL);
    return ZNODE_INT32(0);
}

static napi_threadsafe_function on_dialog_shown_tsfn;
static void _on_on_dialog_shown_cb(napi_env env, napi_value js_cb, void *ctx, void *data)
{
    napi_call_function(env, ZNODE_NULL, js_cb, 0, NULL, NULL);
}
static void on_on_dialog_shown_cb()
{
    napi_call_threadsafe_function(on_dialog_shown_tsfn, NULL, napi_tsfn_nonblocking);
}

ZNODE_FUNCTION(set_on_dialog_shown_cb)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    napi_value cb_v8 = args[arg_index++];
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    napi_valuetype t; napi_typeof(env, cb_v8, &t);
    if (on_dialog_shown_tsfn) { napi_release_threadsafe_function(on_dialog_shown_tsfn, napi_tsfn_release); on_dialog_shown_tsfn = NULL; }
    if (t == napi_function)
    {
        napi_value name; napi_create_string_utf8(env, "on_dialog_shown_cb", NAPI_AUTO_LENGTH, &name);
        napi_create_threadsafe_function(env, cb_v8, NULL, name, 0, 1, NULL, NULL, NULL, _on_on_dialog_shown_cb, &on_dialog_shown_tsfn);
        napi_unref_threadsafe_function(env, on_dialog_shown_tsfn);
        p_brd_sdk_set_on_dialog_shown_cb((brd_sdk_on_dialog_shown_t)on_on_dialog_shown_cb);
    }
    else
        p_brd_sdk_set_on_dialog_shown_cb(NULL);
    return ZNODE_INT32(0);
}

ZNODE_FUNCTION(fix_service_status)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_fix_service_status();
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_service_auto_start)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_BOOL(enable);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_service_auto_start(enable ? 1 : 0);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(stop_service)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_stop_service();
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(start_service)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_start_service();
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(notify_install)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_notify_install();
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(uuid)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    ZNODE_RETURN(ZNODE_UTF8(p_brd_sdk_get_uuid()));
}

ZNODE_FUNCTION(get_tracking_id)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    ZNODE_RETURN(ZNODE_UTF8(p_brd_sdk_get_tracking_id()));
}

ZNODE_FUNCTION(set_tracking_id)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(tracking_id);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_tracking_id(tracking_id);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_benefit_txt)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(benefit_txt);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_benefit(benefit_txt);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_lang)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(lang);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_lang(lang);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_consent_txt_color)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(txt_color);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_txt_color(txt_color);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_consent_app_name_color)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(app_name_color);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_app_name_color(app_name_color);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_consent_bg_color)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(bg_color);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_bg_color(bg_color);
    ZNODE_RETURN(0);
}

ZNODE_FUNCTION(set_consent_btn_color)
{
    ZNODE_ARG_BEGIN_NOCONTEXT;
    ZNODE_ARG_STRING(btn_color);
    ZNODE_ARG_END;
    if (!brd_sdk_loaded)
        ZNODE_ERROR("brd_sdk not loaded");
    p_brd_sdk_set_btn_color(btn_color);
    ZNODE_RETURN(0);
}

ZNODE_INIT_FUNCTION(brd_sdk)
{
    ZNODE_BEGIN_INIT;
    ZNODE_DECLARE_FUNCTION(load);
    ZNODE_DECLARE_FUNCTION(init);
    ZNODE_DECLARE_FUNCTION(init_async);
    ZNODE_DECLARE_FUNCTION(is_supported);
    ZNODE_DECLARE_FUNCTION(show_consent);
    ZNODE_DECLARE_FUNCTION(get_consent);
    ZNODE_DECLARE_FUNCTION(opt_out);
    ZNODE_DECLARE_FUNCTION(opt_in);
    ZNODE_DECLARE_FUNCTION(close);
    ZNODE_DECLARE_FUNCTION(set_choice_change_cb);
    ZNODE_DECLARE_FUNCTION(set_service_status_change_cb);
    ZNODE_DECLARE_FUNCTION(set_on_dialog_shown_cb);
    ZNODE_DECLARE_FUNCTION(set_on_dialog_closed_cb);
    ZNODE_DECLARE_FUNCTION(fix_service_status);
    ZNODE_DECLARE_FUNCTION(set_service_auto_start);
    ZNODE_DECLARE_FUNCTION(stop_service);
    ZNODE_DECLARE_FUNCTION(start_service);
    ZNODE_DECLARE_FUNCTION(notify_install);
    ZNODE_DECLARE_FUNCTION(uuid);
    ZNODE_DECLARE_FUNCTION(set_tracking_id);
    ZNODE_DECLARE_FUNCTION(get_tracking_id);
    ZNODE_DECLARE_FUNCTION(set_benefit_txt);
    ZNODE_DECLARE_FUNCTION(set_lang);
    ZNODE_DECLARE_FUNCTION(set_consent_txt_color);
    ZNODE_DECLARE_FUNCTION(set_consent_app_name_color);
    ZNODE_DECLARE_FUNCTION(set_consent_bg_color);
    ZNODE_DECLARE_FUNCTION(set_consent_btn_color);
    return exports;
}

NAPI_MODULE(brd_sdk, brd_sdk)

