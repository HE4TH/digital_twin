$ErrorActionPreference='Stop'
$node = (Get-Command node -ErrorAction SilentlyContinue).Source
if (-not $node) {
    $node = Join-Path $env:USERPROFILE '.cache/codex-runtimes/codex-primary-runtime/dependencies/node/bin/node.exe'
}
if (-not (Test-Path -LiteralPath $node)) { throw 'Node.js 22 LTS를 설치하세요.' }
$npm = Join-Path $PSScriptRoot '.runtime/npm/package/bin/npm-cli.js'
if (-not (Test-Path -LiteralPath $npm)) {
    New-Item -ItemType Directory -Force (Join-Path $PSScriptRoot '.runtime/npm') | Out-Null
    $archive = Join-Path $PSScriptRoot '.runtime/npm.tgz'
    Invoke-WebRequest 'https://registry.npmjs.org/npm/-/npm-10.9.2.tgz' -OutFile $archive
    tar -xzf $archive -C (Join-Path $PSScriptRoot '.runtime/npm')
    if ($LASTEXITCODE -ne 0) { throw 'npm 압축 해제 실패' }
}
Push-Location (Join-Path $PSScriptRoot 'pixel')
try {
    & $node $npm ci --ignore-scripts
    if ($LASTEXITCODE -ne 0) { throw 'Pixel Streaming 의존성 설치 실패' }
    & $node 'node_modules/esbuild/bin/esbuild' player.js --bundle --outfile=../.runtime/pixel-player.js --format=iife
    if ($LASTEXITCODE -ne 0) { throw '영상 플레이어 빌드 실패' }
} finally { Pop-Location }
