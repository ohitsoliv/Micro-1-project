# =============================================================================
# build.ps1 — Build and flash the Snake game without needing 'make'
# Usage:
#   .\build.ps1          — compile only
#   .\build.ps1 -Flash   — compile then upload to Arduino
#   .\build.ps1 -Port COM4 -Flash  — specify COM port (default: COM3)
# =============================================================================
param(
    [switch]$Flash,
    [string]$Port = "COM3"
)

$ErrorActionPreference = "Stop"

# ---- Toolchain paths (Arduino IDE 2.x) -------------------------------------
$AVR_BIN  = "$env:LOCALAPPDATA\Arduino15\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino7\bin"
$GCC      = "$AVR_BIN\avr-gcc.exe"
$OBJCOPY  = "$AVR_BIN\avr-objcopy.exe"
$SIZE     = "$AVR_BIN\avr-size.exe"
$AVRDUDE  = "$env:LOCALAPPDATA\Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1\avrdude.exe"
$AVRDUDE_CONF = "$env:LOCALAPPDATA\Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1\etc\avrdude.conf"

# ---- Project paths ---------------------------------------------------------
$ROOT    = $PSScriptRoot
$SRCDIR  = "$ROOT\src"
$INCDIR  = "$ROOT\include"
$BUILDDIR = "$ROOT\build"

# ---- MCU settings ----------------------------------------------------------
$MCU   = "atmega328p"
$FCPU  = "16000000UL"
$CFLAGS = @("-Os", "-Wall", "-Wextra", "-std=c11", "-mmcu=$MCU", "-DF_CPU=$FCPU", "-I$INCDIR")

# ---- Ensure build directory exists -----------------------------------------
if (-not (Test-Path $BUILDDIR)) { New-Item -ItemType Directory $BUILDDIR | Out-Null }

# ---- Compile each .c file --------------------------------------------------
$sources = Get-ChildItem "$SRCDIR\*.c"
$objects = @()

foreach ($src in $sources) {
    $obj = "$BUILDDIR\$($src.BaseName).o"
    $objects += $obj
    Write-Host "Compiling $($src.Name)..." -ForegroundColor Cyan
    & $GCC @CFLAGS -c -o $obj $src.FullName
    if ($LASTEXITCODE -ne 0) { Write-Error "Compile failed: $($src.Name)"; exit 1 }
}

# ---- Link ------------------------------------------------------------------
$elf = "$BUILDDIR\snake.elf"
Write-Host "Linking..." -ForegroundColor Cyan
& $GCC "-mmcu=$MCU" -o $elf @objects
if ($LASTEXITCODE -ne 0) { Write-Error "Link failed"; exit 1 }

# ---- Convert to HEX --------------------------------------------------------
$hex = "$BUILDDIR\snake.hex"
Write-Host "Generating HEX..." -ForegroundColor Cyan
& $OBJCOPY -O ihex -R .eeprom $elf $hex
if ($LASTEXITCODE -ne 0) { Write-Error "objcopy failed"; exit 1 }

# ---- Print size ------------------------------------------------------------
& $SIZE --mcu=$MCU --format=avr $elf

Write-Host "`nBuild successful: $hex" -ForegroundColor Green

# ---- Flash -----------------------------------------------------------------
if ($Flash) {
    Write-Host "`nFlashing to Arduino on $Port..." -ForegroundColor Yellow
    & $AVRDUDE -C $AVRDUDE_CONF -c arduino -p $MCU -P $Port -b 115200 -U "flash:w:${hex}:i"
    if ($LASTEXITCODE -ne 0) { Write-Error "Flash failed"; exit 1 }
    Write-Host "Flash successful!" -ForegroundColor Green
}
