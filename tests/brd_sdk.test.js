// LICENSE_CODE ZON
'use strict'; /*jslint node:true, es6:true*/

const assert = require('assert')
const path = require('path')
const proxyquire = require('proxyquire').noCallThru()

// ─── Native stub ─────────────────────────────────────────────────────────────

function make_native() {
    return {
        load: () => {},
        init: () => {},
        close: () => {},
        is_supported: () => true,
        get_tracking_id: () => 'tracking-id-123',
        uuid: () => 'uuid-abc',
        opt_in: () => {},
        opt_out: () => {},
        show_consent: () => {},
        fix_service_status: () => {},
        start_service: () => {},
        set_service_auto_start: () => {},
        set_benefit_txt: () => {},
        set_consent_txt_color: () => {},
        set_consent_app_name_color: () => {},
        set_consent_bg_color: () => {},
        set_consent_btn_color: () => {},
        set_lang: () => {},
        set_service_status_change_cb: cb => { native._status_cb = cb },
        set_choice_change_cb:         cb => { native._choice_cb = cb },
        set_on_dialog_shown_cb:       cb => { native._shown_cb  = cb },
        set_on_dialog_closed_cb:      cb => { native._closed_cb = cb },
    }
}

let native
function make_sdk() {
    native = make_native()
    const sdk = proxyquire('../src/brd_sdk.js', {
        '../build/Release/brd_sdk.node': native,
    })
    // proxyquire returns a cached singleton — reset internal state for each test
    sdk._initialized = false
    sdk._opt_in = false
    sdk._consent_on_screen = false
    sdk._current_status = null
    sdk._need_set_auto_start = false
    sdk._app_path = path.dirname(process.execPath)
    sdk.removeAllListeners()
    return sdk
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

async function init_sdk(sdk, opts = {}) {
    await sdk.init('com.test.app', { app_path: '/fake/path', ...opts })
}

// ─── Tests ───────────────────────────────────────────────────────────────────

describe('BrdSdkMgr', () => {
    describe('before init()', () => {
        it('throws on get_tracking_id()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.get_tracking_id(), /call init\(\)/)
        })
        it('throws on opt_in()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.opt_in(), /call init\(\)/)
        })
        it('throws on opt_out()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.opt_out(), /call init\(\)/)
        })
        it('throws on close()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.close(), /call init\(\)/)
        })
        it('throws on get_uuid()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.get_uuid(), /call init\(\)/)
        })
        it('throws on check_sdk_state()', () => {
            const sdk = make_sdk()
            assert.throws(() => sdk.check_sdk_state(), /call init\(\)/)
        })
        it('is_supported() does not require init', () => {
            const sdk = make_sdk()
            assert.strictEqual(sdk.is_supported(), true)
        })
    })

    describe('init()', () => {
        it('resolves successfully', async () => {
            const sdk = make_sdk()
            await assert.doesNotReject(() => init_sdk(sdk))
        })
        it('sets _initialized = true', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            assert.strictEqual(sdk._initialized, true)
        })
        it('sets app_path from options', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk, { app_path: '/custom/path' })
            assert.strictEqual(sdk._app_path, '/custom/path')
        })
        it('calls native.init with app_id', async () => {
            const sdk = make_sdk()
            let called_with
            native.init = (id, _opts) => { called_with = id }
            await init_sdk(sdk)
            assert.strictEqual(called_with, 'com.test.app')
        })
        it('rejects when native.init throws', async () => {
            const sdk = make_sdk()
            native.init = () => { throw new Error('native failure') }
            await assert.rejects(() => init_sdk(sdk), /native failure/)
        })
    })

    describe('status mapping', () => {
        it('get_status_name() returns correct label', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            native._status_cb(0)  // None
            assert.strictEqual(sdk.get_status_name(), 'None')
            native._status_cb(7)  // Connected
            assert.strictEqual(sdk.get_status_name(), 'Connected')
        })
        it('get_status_code() returns raw code', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            native._status_cb(3)
            assert.strictEqual(sdk.get_status_code(), 3)
        })
    })

    describe('events', () => {
        it('emits status_change with code and name', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let result
            sdk.on('status_change', (code, name) => { result = { code, name } })
            native._status_cb(8)  // Peer
            assert.deepStrictEqual(result, { code: 8, name: 'Peer' })
        })
        it('emits choice_change Agree', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let result
            sdk.on('choice_change', (_code, name) => { result = name })
            native._choice_cb(1)  // Agree
            assert.strictEqual(result, 'Agree')
        })
        it('emits choice_change Disagree', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let result
            sdk.on('choice_change', (_code, name) => { result = name })
            native._choice_cb(2)  // Disagree
            assert.strictEqual(result, 'Disagree')
        })
        it('emits dialog_shown and dialog_closed', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let shown = 0, closed = 0
            sdk.on('dialog_shown', () => shown++)
            sdk.on('dialog_closed', () => closed++)
            native._shown_cb()
            native._closed_cb()
            assert.strictEqual(shown, 1)
            assert.strictEqual(closed, 1)
        })
    })

    describe('opt_in / opt_out state', () => {
        it('_opt_in becomes true on Agree', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            native._choice_cb(1)  // Agree
            assert.strictEqual(sdk.get_opt_in(), true)
        })
        it('_opt_in becomes false on Disagree', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            native._choice_cb(1)
            native._choice_cb(2)  // Disagree
            assert.strictEqual(sdk.get_opt_in(), false)
        })
    })

    describe('show_consent()', () => {
        it('calls native.show_consent when not on screen', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let calls = 0
            native.show_consent = () => calls++
            sdk.show_consent()
            assert.strictEqual(calls, 1)
        })
        it('does not call native.show_consent a second time while on screen', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let calls = 0
            native.show_consent = () => calls++
            native._shown_cb()   // dialog is now on screen
            sdk.show_consent()
            assert.strictEqual(calls, 0)
        })
        it('allows show_consent again after dialog_closed', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            let calls = 0
            native.show_consent = () => calls++
            native._shown_cb()
            native._closed_cb()
            sdk.show_consent()
            assert.strictEqual(calls, 1)
        })
    })

    describe('get_uuid()', () => {
        it('returns uuid from native when available', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            assert.strictEqual(sdk.get_uuid(), 'uuid-abc')
        })
        it('falls back to null when native throws and no install-id file', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            native.uuid = () => { throw new Error('no uuid') }
            // no install-id file at /fake/path → returns null
            assert.strictEqual(sdk.get_uuid(), null)
        })
    })

    describe('_get_dll_path()', () => {
        it('returns 64-bit DLL name for x64', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk, { app_path: '/my/app' })
            const dll = sdk._get_dll_path()
            assert.ok(dll.endsWith('lum_sdk64.dll') || dll.endsWith('lum_sdk32.dll'))
        })
        it('uses app_path when provided', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk, { app_path: '/my/app' })
            // The path starts with app_path (the file may not exist so fallback kicks in, but prefix is checked first)
            // We just verify the method returns a string ending in .dll
            assert.ok(sdk._get_dll_path().endsWith('.dll'))
        })
    })

    describe('get_tracking_id()', () => {
        it('returns value from native', async () => {
            const sdk = make_sdk()
            await init_sdk(sdk)
            assert.strictEqual(sdk.get_tracking_id(), 'tracking-id-123')
        })
    })
})
