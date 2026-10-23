$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path $PSScriptRoot -Parent
$taskOutput = Join-Path $taskRoot '.fork-checks'
New-Item -ItemType Directory -Path $taskOutput -Force | Out-Null
$taskDependencies = Join-Path $taskOutput 'deps'
$taskTomlHeader = Join-Path $taskDependencies 'toml++/toml.hpp'
if (!(Test-Path -LiteralPath $taskTomlHeader)) {
  New-Item -ItemType Directory -Path (Split-Path $taskTomlHeader) -Force | Out-Null
  Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/marzer/tomlplusplus/v3.4.0/toml.hpp' -OutFile $taskTomlHeader
}
if ((Get-FileHash -LiteralPath $taskTomlHeader -Algorithm SHA256).Hash -ne '6B5172AD4DD6519AEC67B919181FA7A38A2234131E5B2AFA232DFE444819783E') {
  throw 'Unexpected toml++ v3.4.0 header checksum'
}
$taskSdkRoot = 'C:/Program Files (x86)/Windows Kits/10'
$taskSdkVersion = (Get-ChildItem -LiteralPath "$taskSdkRoot/Include" -Directory | Sort-Object Name -Descending | Select-Object -First 1).Name
$taskInclude = "$taskSdkRoot/Include/$taskSdkVersion"
$taskExecutable = Join-Path $taskOutput 'neovide-smoke.exe'
$taskObject = Join-Path $taskOutput 'neovide-smoke.obj'
$taskSource = Join-Path $taskRoot 'Tests/neovide_smoke.cpp'
$taskLog = Join-Path $taskOutput 'compile.log'
if (Get-Command cl -ErrorAction SilentlyContinue) {
  & cl /nologo /std:c++latest /WX /EHsc /DNOMINMAX /DWIN32_LEAN_AND_MEAN "/I$taskInclude/cppwinrt" "/I$taskDependencies" $taskSource "/Fe$taskExecutable" "/Fo$taskObject" /link WindowsApp.lib user32.lib 2>&1 | Set-Content -LiteralPath $taskLog
} else {
  & clang++ -std=c++20 -O2 -fms-extensions -Wno-nonportable-include-path -static-libstdc++ -static-libgcc -DNOMINMAX -DWIN32_LEAN_AND_MEAN -I "$taskInclude/cppwinrt" -I $taskDependencies -idirafter "$taskInclude/um" -idirafter "$taskInclude/shared" -idirafter "$taskInclude/winrt" $taskSource -o $taskExecutable "$taskSdkRoot/Lib/$taskSdkVersion/um/x64/WindowsApp.lib" -lole32 -loleaut32 -luuid 2>&1 | Set-Content -LiteralPath $taskLog
}
if ($LASTEXITCODE -ne 0) { Get-Content -LiteralPath $taskLog -Tail 45; throw 'Neovide policy/C++WinRT check did not compile' }
& $taskExecutable
if ($LASTEXITCODE -ne 0) { throw 'Neovide policy checks failed' }
Write-Output 'Codex/Neovide theme policies, TOML settings and exact C++/WinRT transition header checks passed'
