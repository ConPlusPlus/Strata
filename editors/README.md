# Editor support

Syntax highlighting for Strata (`.strata`, `.str`). One TextMate grammar drives both
editors.

| Editor | Install | Admin? |
|---|---|---|
| VS Code | `powershell -ExecutionPolicy Bypass -File editors\vscode\install.ps1`, then *Reload Window* | no |
| Visual Studio | `powershell -ExecutionPolicy Bypass -File editors\visualstudio\install.ps1`, then restart VS | yes (UAC prompt) |

## Changing the grammar

The grammar is generated, so don't edit the JSON by hand:

1. Edit `build_grammar.py`. Its word lists mirror `compiler/src/lexer.strata` (keywords) and
   `compiler/src/checker.strata` (types, built-ins); update them when the language changes.
2. `python editors/build_grammar.py` writes `vscode/syntaxes/strata.tmLanguage.json`.
3. Re-run the installer(s).
