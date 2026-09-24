# Strata syntax highlighting for Visual Studio

Visual Studio has a built-in **TextMate** grammar engine, so no VSIX is needed: the
installer drops the Strata grammar (the same file VS Code uses,
`editors/vscode/syntaxes/strata.tmLanguage.json`) into VS's TextMate folder. The grammar
associates itself with `.strata` and `.str` files via its `fileTypes` field.

## Install (one command)

Run it from a normal PowerShell. The script elevates itself, so approve the **UAC**
prompt; the folder lives under `Program Files`.

```powershell
cd C:\Strata; powershell -ExecutionPolicy Bypass -File editors\visualstudio\install.ps1
```

It copies the grammar into every Visual Studio install it finds, clears the TextMate
cache, and pauses. Then **restart Visual Studio** and open a `.strata` file.

## Manual install

For each VS install, copy the grammar to:

```
<VS>\Common7\IDE\CommonExtensions\Microsoft\TextMate\Starterkit\Extensions\strata\syntaxes\strata.tmLanguage.json
```

e.g. under `C:\Program Files\Microsoft Visual Studio\18\Community\...`. Restart VS.

## Notes

- **Re-run after a VS update**, since updates can remove files from that folder.
- Colorization only (no IntelliSense). `stratac check <file.strata>` gives type errors.
