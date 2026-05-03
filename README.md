<p align="center">
  <a href="https://bright-sdk.com/"><img src="docs/assets/brightsdk-logo.svg" alt="BrightSDK" height="50"></a>
  <img src="docs/assets/times.svg" height="50" alt="x">
  <a href="https://www.electronjs.org/"><img src="docs/assets/electron-logo.svg" alt="Electron" height="50"></a>
</p>

# BrightSDK plugin for Electron

[![Test](https://github.com/BrightSDK/electron-plugin/actions/workflows/test.yml/badge.svg)](https://github.com/BrightSDK/electron-plugin/actions/workflows/test.yml)
[![Lint](https://github.com/BrightSDK/electron-plugin/actions/workflows/lint.yml/badge.svg)](https://github.com/BrightSDK/electron-plugin/actions/workflows/lint.yml)

A Node.js native addon that wraps the Bright Data SDK DLL and exposes a clean JavaScript API for Electron (and plain Node.js) applications on Windows.

The addon is built against the **N-API stable ABI** (`NAPI_VERSION=8`), so the same compiled `.node` file works across Node.js and Electron versions without rebuilding.

---

## Table of Contents

- [Requirements](#requirements)
- [Obtaining the SDK Distributables](#obtaining-the-sdk-distributables)
- [Building the Addon](#building-the-addon)
- [Installing from GitHub](#installing-from-github)
- [Usage](#usage)
- [API Reference](#api-reference)
  - [Initialisation](#initialisation)
  - [Methods](#methods)
  - [Events](#events)
- [Supported Languages](#supported-languages)
- [Packaging with electron-builder (NSIS)](#packaging-with-electron-builder-nsis)

---

## Requirements

| Requirement | Version / Notes |
|---|---|
| Windows | 10 or later (32-bit or 64-bit) |
| Node.js | 18 or later |
| npm | 9 or later |
| Python | 3.x (required by `node-gyp`) |
| Visual Studio Build Tools | 2019 or later with the **Desktop development with C++** workload |
| node-gyp | Installed globally: `npm install -g node-gyp` |

---

## Obtaining the SDK Distributables

The SDK DLLs and the network updater executable are **not included** in this repository. You must download them from the [Bright Data SDK Dashboard](https://brightdata.com/cp/sdk) and place them in a folder that your app can locate at runtime.

Required files:

| File | Architecture |
|---|---|
| `lum_sdk32.dll` | 32-bit |
| `lum_sdk64.dll` | 64-bit |
| `net_updater32.exe` | 32-bit |
| `net_updater64.exe` | 64-bit |

Place these files in the same directory as your application's executable. The SDK will locate them automatically — see the [Usage](#usage) section for details.

When building with **electron-builder**, use `extraFiles` (not `extraResources`) to copy the DLLs next to the exe:

```json
"extraFiles": [{ "from": "brd_sdk_dist", "to": "." }]
```

---

## Building the Addon

### Standard build (system Node.js)

```bash
cd sdk
npm install
```

The `install` script runs `node-gyp rebuild --arch=x64` automatically.

### Build for a specific Electron version

Since the addon uses the N-API stable ABI, you do not need to rebuild against a specific Electron version, but it is possible. Use `@electron/rebuild` from inside your Electron application:

```bash
cd your-electron-app
node node_modules\@electron\rebuild\lib\cli.js -f -m ../sdk
```

---

## Installing from GitHub

Add the package as a dependency in your `package.json`:

```json
{
  "dependencies": {
    "electron-bright-sdk": "github:BrightSDK/electron-plugin"
  }
}
```

Or install directly:

```bash
npm install github:BrightSDK/electron-plugin
```

After installation, the `install` script in `package.json` automatically compiles the native addon. Make sure the [requirements](#requirements) above are met before running `npm install`.

---

## Usage

Require the module and call `init()` before using any other method:

```js
const brd_sdk = require('electron-bright-sdk');
const path = require('path');

await brd_sdk.init('com.example.myapp', {
    app_path:     app.isPackaged
                    ? path.dirname(app.getPath('exe'))
                    : path.join(__dirname, 'brd_sdk_dist'),
    app_name:     'My App',
    logo_link:    'https://example.com/logo.png',
    skip_consent: false,
});
```

The SDK resolves the DLL path as follows:

1. **`<app_path>/<dll_name>`** — used when `app_path` is provided.
2. **`<exe dir>/<dll_name>`** — fallback when `app_path` is omitted or the DLL is not found there.

The fallback (`exe dir`) works automatically in **packaged builds**, where the executable is your app's own binary. In **dev mode** the executable is Electron's own binary inside `node_modules`, so `app_path` must always be set explicitly to point to wherever you keep the DLLs.

In an Electron app, the typical pattern is:

```js
// In Electron main process:
app_path: app.isPackaged
    ? path.dirname(app.getPath('exe'))   // packaged: next to the .exe — no extra config needed
    : path.join(__dirname, 'brd_sdk_dist') // dev: a local folder you keep the DLLs in
```

---

## API Reference

### Initialisation

#### `init(appId, options) → Promise<void>`

Loads the DLL, registers all callbacks, and initialises the SDK. Must be called before any other method. Resolves on success; rejects with an `Error` on failure.

| Parameter | Type | Required | Description |
|---|---|---|---|
| `appId` | `string` | ✅ | Unique reverse-domain app identifier registered in the Bright Data dashboard (e.g. `com.example.myapp`). |
| `options.app_path` | `string` | | Directory where the SDK DLLs (`lum_sdk32.dll` / `lum_sdk64.dll`) are located. If omitted, falls back to the directory of the running executable — which works in packaged builds (DLLs next to the `.exe`) but **not** in dev mode (where the exe is Electron's own binary). Always set this explicitly when running under Electron. |
| `options.app_name` | `string` | | Human-readable name shown on the consent screen. |
| `options.logo_link` | `string` | | URL of your app's logo for the consent screen. |
| `options.skip_consent` | `boolean` | | When `true`, the consent dialog is never shown automatically. Use this option when you want to show consent at a later time or you use your own consent screen. Default: `false`. |

---

### Methods

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `get_uuid()` | — | `string \| null` | Returns the unique identifier assigned to this SDK installation. Falls back to reading from the install-id file if the DLL returns nothing. |
| `get_tracking_id()` | — | `string` | Returns the tracking ID for the current session. May change across sessions. |
| `get_status()` | — | `{ status_name: string, opt_in: boolean \| null }` | Returns the current service status name and the user's consent state. |
| `get_status_code()` | — | `number` | Returns the raw numeric service status code. |
| `get_status_name()` | — | `string` | Returns the human-readable service status string. |
| `get_opt_in()` | — | `boolean` | Returns `true` if the user has opted in. |
| `is_supported()` | — | `boolean` | Returns `true` if the SDK is supported on the current OS version. |
| `show_consent()` | — | `void` | Opens the built-in consent dialog. No-op if the dialog is already on screen. |
| `opt_in()` | — | `void` | Opts the user in to web indexing directly, without showing the consent dialog. |
| `opt_out()` | — | `void` | Opts the user out of web indexing without showing the consent dialog. |
| `fix_sdk()` | — | `void` | Attempts to repair a broken or out-of-date SDK installation. |
| `close()` | — | `void` | Shuts down the SDK and releases all resources. |
| `check_sdk_state()` | — | `void` | Re-evaluates the SDK state and starts the service if the user has opted in and it is not running. |
| `set_benefit_txt(text)` | `text: string` | `void` | Overrides the benefit sentence on the consent screen. Call before `show_consent()`. |
| `set_lang(locale)` | `locale: string` | `void` | Sets the consent screen language. Call before `show_consent()`. See [Supported Languages](#supported-languages) for valid locale codes. |
| `set_consent_txt_color(argb)` | `argb: string` | `void` | Sets the body text color on the consent screen (`#AARRGGBB`). |
| `set_consent_app_name_color(argb)` | `argb: string` | `void` | Sets the app name color on the consent screen (`#AARRGGBB`). |
| `set_consent_bg_color(argb)` | `argb: string` | `void` | Sets the consent dialog background color (`#AARRGGBB`). |
| `set_consent_btn_color(argb)` | `argb: string` | `void` | Sets the consent button color (`#AARRGGBB`). |

> **Color format:** all color parameters use `#AARRGGBB` (8-digit hex, alpha first). For fully opaque colors prefix your `#rrggbb` value with `FF`, e.g. `#FF1A1A2E`.

---

### Events

`brd_sdk` extends `EventEmitter`. Subscribe using the standard `.on()` method.

| Event | Arguments | Description |
|---|---|---|
| `status_change` | `(code: number, name: string)` | Fired when the service status changes. `name` is one of: `None`, `NotInstalled`, `Installed`, `NotRunning`, `Running`, `Disconnected`, `Blocked`, `Connected`, `Peer`. |
| `choice_change` | `(code: number, name: string)` | Fired when the user's consent choice changes. `name` is `"Agree"` or `"Disagree"`. |
| `dialog_shown` | — | Fired when the consent dialog becomes visible. |
| `dialog_closed` | — | Fired when the consent dialog is dismissed. |

```js
brd_sdk.on('status_change', (code, name) => {
    console.log(`Service: ${name} (${code})`);
});

brd_sdk.on('choice_change', (code, name) => {
    console.log(`Consent: ${name}`);
});

brd_sdk.on('dialog_shown',  () => console.log('Consent dialog opened'));
brd_sdk.on('dialog_closed', () => console.log('Consent dialog closed'));
```

---

## Supported Languages

This is a list of languages supported by the Bright Data's built-in consent screen. If you would like to have a consent screen in a different language, you have to supply your own consent flow.

| Locale code | Language |
|---|---|
| `en-US` | English (default) |
| `de-DE` | German |
| `es-ES` | Spanish |
| `fr-FR` | French |
| `it-IT` | Italian |
| `pt-PT` | Portuguese |
| `ru-RU` | Russian |
| `zh-CN` | Chinese (Simplified) |

---

## Packaging with electron-builder (NSIS)

For a step-by-step guide on bundling the SDK files, triggering the Consent Screen during NSIS installation, adding the required EULA disclosure, and wiring up the uninstall hook, see:

**[docs/electron-builder-nsis-integration.md](docs/electron-builder-nsis-integration.md)**
