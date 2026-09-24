# SPDX-License-Identifier: GPL-3.0-only
# Copyright © 2026 Connor Rutberg
#
# Installs Strata (.strata / .str) syntax highlighting into Visual Studio's TextMate engine.
#
# Just run it; it asks for admin itself (approve the UAC prompt):
#     powershell -ExecutionPolicy Bypass -File editors\visualstudio\install.ps1
#
# Uses the same grammar as VS Code (editors/vscode/syntaxes/strata.tmLanguage.json).
# Safe to re-run. Re-run after a VS update (updates can wipe the Extensions folder).

$ErrorActionPreference = 'Stop'

# --- Self-elevate: the target folder lives under Program Files (needs admin) ---
$isAdmin = ([Security.Principal.WindowsPrincipal] `
            [Security.Principal.WindowsIdentity]::GetCurrent() `
           ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "Requesting administrator rights (approve the UAC prompt)..."
    Start-Process powershell -Verb RunAs -ArgumentList @(
        '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`""
    )
    exit
}

$grammar = Join-Path $PSScriptRoot '..\vscode\syntaxes\strata.tmLanguage.json'
if (-not (Test-Path $grammar)) { throw "grammar not found: $grammar (run: python editors\build_grammar.py)" }

$roots = @("$env:ProgramFiles\Microsoft Visual Studio",
           "${env:ProgramFiles(x86)}\Microsoft Visual Studio") | Where-Object { Test-Path $_ }

$installed = 0
foreach ($root in $roots) {
    foreach ($year in Get-ChildItem $root -Directory) {
        foreach ($edition in Get-ChildItem $year.FullName -Directory) {
            $ext = Join-Path $edition.FullName 'Common7\IDE\CommonExtensions\Microsoft\TextMate\Starterkit\Extensions'
            if (Test-Path $ext) {
                # layout VS expects: ...\Extensions\strata\syntaxes\strata.tmLanguage.json
                $dest = Join-Path $ext 'strata\syntaxes'
                New-Item -ItemType Directory -Force -Path $dest | Out-Null
                Copy-Item $grammar $dest -Force
                Write-Host "installed  -> $dest"
                $installed++
            }
        }
    }
}

# Clear the per-version TextMate cache so VS re-reads grammars on next launch.
$vsLocal = Join-Path $env:LOCALAPPDATA 'Microsoft\VisualStudio'
if (Test-Path $vsLocal) {
    foreach ($v in Get-ChildItem $vsLocal -Directory) {
        $cache = Join-Path $v.FullName 'TextMateCache'
        if (Test-Path $cache) {
            try { Remove-Item $cache -Recurse -Force; Write-Host "cleared cache -> $cache" } catch {}
        }
    }
}

Write-Host ""
if ($installed -eq 0) { Write-Host "No Visual Studio installs found under Program Files." }
else { Write-Host "Done ($installed install(s)). Restart Visual Studio, then open a .strata file." }
Write-Host ""
Write-Host "Press Enter to close..."
[void](Read-Host)
