// LICENSE_CODE ZON
'use strict'; /*jslint node:true, es6:true*/

// ─── Native binding ──────────────────────────────────────────────────────────
const _native = require('./build/Release/brd_sdk.node');
const { EventEmitter } = require('events');
const path = require('path');
const fs = require('fs');
const crypto = require('crypto');

const SERVICE_STATUS = [
    'None',
    'NotInstalled',
    'Installed',
    'NotRunning',
    'Running',
    'Disconnected',
    'Blocked',
    'Connected',
    'Peer',
];
const service_status =
    SERVICE_STATUS.reduce((acc, curr, idx) => (acc[curr] = idx, acc), {});
_native.service_status = service_status;
_native.get_status = status => SERVICE_STATUS[status];

const CHOICE = ['None', 'Agree', 'Disagree'];
_native.choice = CHOICE.reduce((acc, curr, idx) => (acc[curr] = idx, acc), {});
_native.get_choice = choice => CHOICE[choice];

class BrdSdkMgr extends EventEmitter {
    constructor() {
        super();
        this._opt_in = false;
        this._consent_on_screen = false;
        this._current_status = null;
        this._app_path = path.dirname(process.execPath);
        this._initialized = false;
    }
    init(app_id, options = {}) {
        return new Promise((resolve, reject) => {
            setImmediate(() => {
                try {
                    if (options.app_path)
                        this._app_path = options.app_path;

                    _native.load(this._get_dll_path());

                    const init_opts = { skip_consent: options.skip_consent !== false };
                    if (options.app_name)  init_opts.app_name  = options.app_name;
                    if (options.logo_link) init_opts.logo_link = options.logo_link;
                    if (options.lang)      init_opts.lang      = options.lang;

                    _native.set_service_status_change_cb(s  => this._on_service_status_change(s));
                    _native.set_choice_change_cb(        c  => this._on_choice_change(c));
                    _native.set_on_dialog_shown_cb(      () => this._on_dialog_shown());
                    _native.set_on_dialog_closed_cb(     () => this._on_dialog_closed());

                    _native.init(app_id, init_opts);

                    if (options.benefit_txt)                   _native.set_benefit_txt(options.benefit_txt);
                    if (options.consent_txt_color)             _native.set_consent_txt_color(options.consent_txt_color);
                    if (options.consent_app_name_color)        _native.set_consent_app_name_color(options.consent_app_name_color);
                    if (options.consent_bg_color)              _native.set_consent_bg_color(options.consent_bg_color);
                    if (options.consent_btn_color)             _native.set_consent_btn_color(options.consent_btn_color);

                    this._initialized = true;
                    this._need_set_auto_start = false;
                    this.check_sdk_state();
                    resolve();
                } catch(err) {
                    reject(err);
                }
            });
        });
    }
    fix_sdk() {
        this._assert_init();
        if (!this._opt_in) {
            this.show_consent();
            return;
        }
        if (this._is_running())
            return;
        if (s === _native.service_status.NotInstalled
            || s === _native.service_status.Installed
            || s === _native.service_status.NotRunning) {
            this._enable();
        } else {
            _native.fix_service_status();
        }
    }
    is_supported() {
        return _native.is_supported();
    }
    get_tracking_id() {
        this._assert_init();
        return _native.get_tracking_id();
    }
    set_benefit_txt(val) {
        this._assert_init();
        _native.set_benefit_txt(val);
    }
    set_consent_txt_color(val) {
        this._assert_init();
        _native.set_consent_txt_color(val);
    }
    set_consent_app_name_color(val) {
        this._assert_init();
        _native.set_consent_app_name_color(val);
    }
    set_consent_bg_color(val) {
        this._assert_init();
        _native.set_consent_bg_color(val);
    }
    set_consent_btn_color(val) {
        this._assert_init();
        _native.set_consent_btn_color(val);
    }
    set_lang(val) {
        this._assert_init();
        _native.set_lang(val);
    }
    opt_out() {
        this._assert_init();
        _native.opt_out();
    }
    opt_in() {
        this._assert_init();
        _native.opt_in();
    }
    close() {
        this._assert_init();
        _native.close();
    }
    get_uuid() {
        this._assert_init();
        try {
            const uuid = _native.uuid();
            if (uuid) return uuid;
        } catch(_e) {}
        return this._read_uuid_from_file();
    }
    check_sdk_state() {
        this._assert_init();
        if (!this._opt_in)
            return;
        if (!this._is_running())
            this._enable();
    }
    _enable() {
        _native.start_service();
        if (this._current_status > _native.service_status.Running)
            _native.set_service_auto_start(true);
        else
            this._need_set_auto_start = true;
    }
    _is_running() {
        return this._initialized
            && this._current_status === _native.service_status.Running;
    }
    get_status_code() { return this._current_status; }
    get_status_name() { return _native.get_status(this._current_status); }
    get_opt_in()      { return this._opt_in; }

    _get_dll_path() {
        const dll_name = process.arch === 'x64' ? 'lum_sdk64.dll' : 'lum_sdk32.dll';

        if (this._app_path) {
            const p = path.join(this._app_path, dll_name);
            if (fs.existsSync(p))
                return p;
        }

        // Fallback: directory that contains the executable.
        return path.join(path.dirname(process.execPath), dll_name);
    }
    _assert_init() {
        if (!this._initialized)
            throw new Error('BrdSdkMgr: call init() before using the API');
    }
    show_consent() {
        if (!this._consent_on_screen)
            _native.show_consent();
    }
    _on_service_status_change(status) {
        this._current_status = status;
        const name = _native.get_status(status);
        if (this._need_set_auto_start
            && status > _native.service_status.NotRunning) {
            this._need_set_auto_start = false;
            _native.set_service_auto_start(true);
        }
        this.emit('status_change', status, name);
    }
    _on_choice_change(choice) {
        switch (choice) {
        case _native.choice.None:
        case _native.choice.Disagree:
            this._opt_in = false;
            break;
        case _native.choice.Agree:
            this._opt_in = true;
            this.check_sdk_state();
            break;
        }
        this.emit('choice_change', choice, _native.get_choice(choice));
    }
    _on_dialog_shown() {
        this._consent_on_screen = true;
        this.emit('dialog_shown');
    }
    _on_dialog_closed() {
        this._consent_on_screen = false;
        this.emit('dialog_closed');
    }
    _read_uuid_from_file() {
        try {
            const h = crypto.createHash('sha1')
                .update(this._app_path).digest('hex');
            const candidates = [
                path.join(
                    process.env.ProgramData || 'C:\\ProgramData',
                    'BrightData', h, 'lum_sdk_install_id'),
                path.join(this._app_path, 'luminati', 'lum_sdk_install_id'),
            ];
            for (const candidate of candidates) {
                if (!fs.existsSync(candidate)) continue;
                const content = fs.readFileSync(candidate, 'utf8');
                const m = content.match(/^([a-f0-9]+):/);
                if (m) return `sdk-win-${m[1]}`;
            }
        } catch(_e) {}
        return null;
    }
}

const instance = new BrdSdkMgr();
module.exports = instance;
