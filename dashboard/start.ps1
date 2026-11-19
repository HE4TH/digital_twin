param(
    [ValidatePattern('^[A-Za-z0-9.:-]+$')][string]$Broker='192.168.0.6',
    [ValidateRange(1024,65535)][int]$Port=5050,
    [switch]$Stop,
    [switch]$NoMqtt,
    [string]$PythonPath
)
$ErrorActionPreference='Stop'
Set-Location -LiteralPath $PSScriptRoot
$runtime = Join-Path $PSScriptRoot '.runtime'
New-Item -ItemType Directory -Path $runtime -Force | Out-Null
$statePath = Join-Path $runtime 'background-processes.json'
# Serialize double-clicks and stop requests. The handle is released on exit.
$launcherLock = [IO.File]::Open((Join-Path $runtime 'launcher.lock'), 'OpenOrCreate', 'ReadWrite', 'None')

function Get-OwnedProcess($record) {
    if (-not $record -or -not $record.id) { return $null }
    $process = Get-Process -Id $record.id -ErrorAction SilentlyContinue
    if ($process -and $process.StartTime.ToUniversalTime().Ticks.ToString() -eq $record.started -and
        $process.Path -eq $record.path) { return $process }
}
function Stop-OwnedProcess($record) {
    $process = Get-OwnedProcess $record
    if ($process) {
        Stop-Process -InputObject $process -Force
        $process.WaitForExit()
    }
}
function Save-Processes {
    ConvertTo-Json -InputObject @($script:owned) -Depth 4 |
        Set-Content -LiteralPath $statePath -Encoding UTF8
}
function Start-Server($name, $executable, $arguments) {
    $process = Start-Process -FilePath $executable -ArgumentList $arguments -WorkingDirectory $PSScriptRoot `
        -WindowStyle Hidden -PassThru -RedirectStandardOutput "$runtime/$name.log" -RedirectStandardError "$runtime/$name-error.log"
    $record = [pscustomobject]@{
        name=$name; id=$process.Id; started=$process.StartTime.ToUniversalTime().Ticks.ToString()
        path=$process.Path; port=$Port; broker=$Broker; noMqtt=[bool]$NoMqtt
    }
    $script:owned += $record
    $script:created += $record
    Save-Processes
    return $record
}
function Wait-Server($url, $record) {
    for ($attempt=0; $attempt -lt 30; $attempt++) {
        if (-not (Get-OwnedProcess $record)) { throw "$($record.name) exited. See $runtime/$($record.name)-error.log" }
        try { Invoke-RestMethod $url -TimeoutSec 1 | Out-Null; return } catch {}
        Start-Sleep -Milliseconds 500
    }
    throw "Server did not become ready: $url"
}
$script:owned = @()
$script:created = @()
try {
    if (Test-Path -LiteralPath $statePath) {
        $saved = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
        $script:owned = @(foreach ($record in $saved) {
            if (Get-OwnedProcess $record) { $record }
        })
    }
    if ($Stop) {
        foreach ($record in ($script:owned | Sort-Object name -Descending)) { Stop-OwnedProcess $record }
        $script:owned = @()
        Save-Processes
        Write-Host 'Background dashboard and owned video server stopped.'
        return
    }
    $existing = $script:owned | Where-Object name -eq 'dashboard'
    if ($existing) {
        if ($existing.port -ne $Port -or $existing.broker -ne $Broker -or $existing.noMqtt -ne [bool]$NoMqtt) {
            throw 'A dashboard is running with different settings. Run start.ps1 -Stop first.'
        }
        Write-Host "Already running: http://127.0.0.1:$Port"
        return
    }
    # Do not replace an unrelated server or a previous foreground session.
    $probe = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback, $Port)
    try { $probe.Start() } catch { throw "Port $Port is busy. Close the old server first." } finally { $probe.Stop() }

$python = if ($PythonPath) { (Resolve-Path -LiteralPath $PythonPath).Path } else { Join-Path $PSScriptRoot '.venv/Scripts/python.exe' }
if (-not (Test-Path -LiteralPath $python)) {
    $bundled = Join-Path $env:USERPROFILE '.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
    if (Get-Command py -ErrorAction SilentlyContinue) {
        py -3 -m venv .venv
    } elseif (Test-Path -LiteralPath $bundled) {
        & $bundled -m venv .venv
    } else {
        throw 'Python 3를 설치하고 Python Launcher(py)를 활성화하세요.'
    }
    if ($LASTEXITCODE -ne 0) { throw 'Python 3 설치를 확인하세요.' }
}
if (-not $NoMqtt -and ($PythonPath -or -not (Test-Path '.venv/Lib/site-packages/paho/mqtt/client.py'))) {
    & $python -m pip install -r requirements.txt
    if ($LASTEXITCODE -ne 0) { throw '대시보드 의존성 설치 실패' }
}
if (-not (Test-Path '.runtime/pixel-player.js') -or -not (Test-Path 'pixel/node_modules')) {
    & "$PSScriptRoot/setup_pixel.ps1"
}
$node = (Get-Command node -ErrorAction SilentlyContinue).Source
if (-not $node) { $node = Join-Path $env:USERPROFILE '.cache/codex-runtimes/codex-primary-runtime/dependencies/node/bin/node.exe' }
    $ready = $false
    try { $ready = (Invoke-RestMethod 'http://127.0.0.1:8889' -TimeoutSec 2).service -eq 'underwater-pixel-local' } catch {}
    if (-not $ready) {
        $pixelRecord = Start-Server 'pixel' $node 'pixel/signalling.cjs'
        Wait-Server 'http://127.0.0.1:8889' $pixelRecord
    }
    $arguments = @('-u', 'server.py', '--broker', $Broker, '--port', $Port)
    if ($NoMqtt) { $arguments += '--no-mqtt' }
    $dashboardRecord = Start-Server 'dashboard' $python $arguments
    Wait-Server "http://127.0.0.1:$Port/api/state" $dashboardRecord
    # Windows virtualenv python.exe may be a redirector. Track the actual
    # interpreter as well so stopping never leaves its HTTP listener behind.
    $bridge = Get-Content -LiteralPath "$runtime/bridge.json" -Raw | ConvertFrom-Json
    if ($bridge.process_id -ne $dashboardRecord.id) {
        $worker = Get-Process -Id $bridge.process_id -ErrorAction Stop
        $workerRecord = [pscustomobject]@{
            name='dashboard-worker'; id=$worker.Id; started=$worker.StartTime.ToUniversalTime().Ticks.ToString()
            path=$worker.Path; port=$Port; broker=$Broker; noMqtt=[bool]$NoMqtt
        }
        $script:owned += $workerRecord
        $script:created += $workerRecord
        Save-Processes
    }
    Write-Host "Running in background: http://127.0.0.1:$Port"
    Write-Host 'You can close this window. To stop: start.cmd -Stop'
    Write-Host "Logs: $runtime"
} catch {
    foreach ($record in $script:created) { Stop-OwnedProcess $record }
    $script:owned = @($script:owned | Where-Object { Get-OwnedProcess $_ })
    Save-Processes
    throw
} finally {
    $launcherLock.Dispose()
}
