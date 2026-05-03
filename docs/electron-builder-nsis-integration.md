# Electron-builder NSIS Integration Guide

How to bundle the Bright Data SDK and trigger the Consent Screen during an Electron-builder NSIS installation on Windows.

---

## Prerequisites

- Bright Data SDK distributables (`lum_sdk.dll`, `net_updater32.exe`, `brd_config.json`)
- [electron-builder](https://www.electron.build/) configured in your project
- An App ID registered in the [Bright SDK Dashboard](https://bright-sdk.com/cp/docs/sdk/windows)

---

## Step 1 — Bundle the SDK Files

Use `extraResources` in your `package.json` (or `electron-builder.yml`) to copy the three required SDK files into the app's resources directory next to the executable:

```json
"build": {
  "extraResources": [
    { "from": "sdk/lum_sdk.dll",       "to": "." },
    { "from": "sdk/net_updater32.exe", "to": "." },
    { "from": "sdk/brd_config.json",   "to": "." }
  ]
}
```

> **Important:** `lum_sdk.dll`, `net_updater32.exe`, and `brd_config.json` must all be co-located in the same directory at runtime.

In `main.js`, pass the correct path to `brd_sdk.init()` so the native addon resolves the DLLs correctly in the packaged build:

```javascript
const { app } = require('electron')
const path = require('path')
const brd_sdk = require('electron-bright-sdk')

app.whenReady().then(async () => {
  await brd_sdk.init('YOUR_APP_ID', {
    app_path: path.dirname(app.getPath('exe')),  // points to the resources dir in packaged builds
    app_name: 'Your App Name',
  })
})
```

---

## Step 2 — Trigger Consent Screen During NSIS Install

Create a custom NSIS include file and reference it from your electron-builder config:

**`electron-builder.yml`:**
```yaml
nsis:
  include: "build/custom.nsh"
  requestedExecutionLevel: admin
```

**`build/custom.nsh`:**
```nsis
!macro customInstall
  ; Run Bright Data service installer — shows consent dialog if user hasn't seen it
  ExecWait '"$INSTDIR\resources\net_updater32.exe" --install-ui YOUR_APP_ID'
!macroend
```

This displays the Bright Data consent dialog at install time. If the user agrees, the background service is installed automatically.

**Alternative — show consent from within the app on first launch:**

Skip the installer-time dialog and call `show_consent()` programmatically instead:

```javascript
await brd_sdk.init('YOUR_APP_ID', {
  app_path: path.dirname(app.getPath('exe')),
  skip_consent: true,
})

// Show consent on first launch (e.g. after checking your own first-run flag)
brd_sdk.show_consent()
```

Run `net_updater32.exe -h` to see all available `--dlg-*` flags for customising the consent dialog, or set them via `brd_config.json`.

---

## Step 3 — Add EULA Text to the Installer

Include the required Bright Data disclosure in your installer's license page. In electron-builder, point `nsis.licenseFile` to a plain-text file containing:

```
{App name} installs Bright Data components (no execution).
You will be able to view the component details in full before you accept this
offer, as well as being able to turn Bright Data on and off directly from the
'App Settings'. Read more about Bright Data's EULA at https://bright-sdk.com.
```

```yaml
nsis:
  licenseFile: "build/LICENSE_BRIGHTDATA.txt"
```

---

## Step 4 — Uninstall Hook

Add an uninstall macro to the same `custom.nsh` file so the Bright Data service is cleanly removed when the user uninstalls your app:

```nsis
!macro customUninstall
  ExecWait '"$INSTDIR\resources\net_updater32.exe" --uninstall YOUR_APP_ID'
!macroend
```

---

## Minimal `electron-builder.yml` Example

```yaml
appId: com.example.myapp
productName: My App

extraResources:
  - from: sdk/lum_sdk.dll
    to: .
  - from: sdk/net_updater32.exe
    to: .
  - from: sdk/brd_config.json
    to: .

win:
  target: nsis
  requestedExecutionLevel: highestAvailable

nsis:
  oneClick: false
  allowToChangeInstallationDirectory: true
  licenseFile: build/LICENSE_BRIGHTDATA.txt
  include: build/custom.nsh
```

---

## References

- [Windows .NET Integration Guide](https://docs.bright-sdk.com/integration-guides/windows-integration-net) — authoritative install/uninstall steps
- [Electron Sample App](https://github.com/BrightSDK/electron-sample) — full working example with `main.js`, `ipc.js`, `preload.js`
- [electron-builder NSIS docs](https://www.electron.build/nsis)
- [Bright SDK Dashboard](https://bright-sdk.com/cp/docs/sdk/windows)
