param([string]$Serial)
$ErrorActionPreference = 'Stop'
$apk = Join-Path $PSScriptRoot 'MqttVehicle-arm64.apk'
$manifestPath = Join-Path $PSScriptRoot 'release.json'
if (!(Test-Path -LiteralPath $apk)) { throw 'Extract the complete Release ZIP first. MqttVehicle-arm64.apk must be beside this script.' }
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if ((Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.apk_sha256) { throw 'APK checksum mismatch. Download and extract the Release ZIP again.' }
$adb = Join-Path $PSScriptRoot 'platform-tools\adb.exe'
if (!(Test-Path -LiteralPath $adb)) {
    Write-Host 'ADB is required. It will be downloaded from Google into this folder.'
    Write-Host 'Review the Android SDK terms at https://developer.android.com/tools/releases/platform-tools'
    $accept = Read-Host 'If you have reviewed and accept those terms, type YES (otherwise press Enter to stop)'
    if ($accept -cne 'YES') { throw 'Stopped. You may obtain platform-tools from Google yourself and place the folder beside this script.' }
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $download = Join-Path $PSScriptRoot 'platform-tools-download.zip'
    Invoke-WebRequest -UseBasicParsing 'https://dl.google.com/android/repository/platform-tools-latest-windows.zip' -OutFile $download
    Expand-Archive -LiteralPath $download -DestinationPath $PSScriptRoot -Force
    if (!(Test-Path -LiteralPath $adb)) { throw 'ADB download was incomplete.' }
}
$lines = @(& $adb devices)
if ($LASTEXITCODE -ne 0) { throw 'ADB could not list devices.' }
$devices = @($lines | ForEach-Object {
    if ($_ -match '^(\S+)\s+device$') { $Matches[1] }
})
if (!$Serial) {
    if ($devices.Count -ne 1) {
        $lines | Write-Host
        throw 'Connect one Quest with a USB data cable, enable Developer Mode, and allow USB debugging in the headset. For multiple devices, use -Serial.'
    }
    $Serial = $devices[0]
}
if ($Serial -notin $devices) { throw 'The selected device is not authorized or connected.' }
$model = ((& $adb -s $Serial shell getprop ro.product.model) -join '').Trim()
if ($LASTEXITCODE -ne 0 -or $model -notmatch 'Quest\s*3') {
    throw "Expected Quest 3 or Quest 3S; connected model: $model"
}
Write-Host "Installing D-Mix Underwater on $model. Keep the USB cable connected."
& $adb -s $Serial install -r $apk
if ($LASTEXITCODE -ne 0) { throw 'APK installation failed. See the ADB error above.' }
Write-Host 'Installation complete. In the headset, open D-Mix Underwater under Unknown Sources.'
Write-Host 'The app has not been started automatically. RC control still requires Wi-Fi and the Pi receiver.'
