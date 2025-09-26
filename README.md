# How to build?
- Add the latest Russian version NRO file to the dbi folder as **`DBI.ver.base_lang.nro`**.
- Add the target language you want to translate into as **rec6.`target_lang`.txt** and set `ver` and `target_lang` in the config.txt file.
  - Set `target_lang=all` to build patches for every `translate/rec6.*.txt` except Russian; each run produces one artifact per language.
  - GitHub Actions now publishes a release per push with all artifacts attached and an auto-generated changelog using the short commit hash.
  - Ukrainian glyph mirroring uses `font/0x7555E0_bundle.ttf`; regenerate it with `font/mirror_glyph.py` and `font/patch_font.py` (requires `python3` and the `zstandard` module).
  - Placeholders in the original and translated strings must match.
  - Please ensure that the total compressed size does not exceed **`14,863 bytes`**.
    - In the Actions tab, check the build & Patch job to see the compression size of rec6.bin. (e.g. patched 11769 B/ 14863 B)
- Check the **Actions** tab to download the raw `DBI.<lang>.nro` artifact.

## Continuous Integration

- GitHub Actions runs the translation build for every push and pull request using the same Make targets as local builds. Successful pushes automatically publish a release containing the raw `.nro` artifacts per language alongside a short-hash changelog.
- To add a new language, fork the repo, drop a `translate/rec6.<lang>.txt` file with your translation, and open a pull request. The workflow will validate the patch and surface the resulting `.nro` for review.



## Version `target_lang` values

The following values are recognized (case-insensitive, aliases included).  
Depending on the mapping, the final 20-byte version string will be patched automatically.

- **Korean**: `ko`, `kr`, `kor`, `korea`, `korean` → `KR`
- **Japanese**: `ja`, `jp`, `jpn`, `japan`, `japanese` → `JP`
- **Chinese (Simplified)**: `zhcn`, `zh-cn`, `cn`, `chs`, `zhhans`, `hans` → `ZHCN`
- **Chinese (Traditional)**: `zhtw`, `zh-tw`, `tw`, `cht`, `zhhant`, `hant` → `ZHTW`
- **English (US)**: `en`, `us`, `en-us`, `enus`, `american` → `EN`
- **English (UK)**: `uk`, `gb`, `engb`, `en-gb`, `british` → `ENGB`
- **French**: `fr`, `french`, `france` → `FR`
- **French (Canada)**: `frca`, `fr-ca`, `canadianfrench` → `FRCA`
- **German**: `de`, `german`, `germany`, `deu`, `ger` → `DE`
- **Italian**: `it`, `italian`, `italy`, `ita` → `IT`
- **Dutch**: `nl`, `dutch`, `netherlands`, `holland`, `nld` → `NL`
- **Spanish (Spain)**: `es`, `spanish`, `spain`, `espanol`, `castellano`, `spa` → `ES`
- **Spanish (Latin America)**: `es419`, `es-419`, `latamspanish` → `ES419`
- **Portuguese (Portugal)**: `pt`, `portuguese`, `portugal`, `por` → `PT`
- **Portuguese (Brazil)**: `ptbr`, `pt-br`, `brazil`, `brazilian`, `br` → `PTBR`
- **Polish**: `pl`, `polish`, `poland`, `pol` → `PL`
- **Russian**: `ru`, `russian`, `russia`, `rus` → `RU`
- **Ukrainian**: `ua`, `ukr`, `uk-ua`, `ukraine` → `UA`

### Fallback
- Any other `target_lang` values will fall back to their **first 4 characters** and be padded with NUL bytes to exactly 20 bytes.
- Example: `target_lang=example` → patched as "DBI: `ver`-**`exam`** FW: `hos_ver`-`nand_type`"

# DBI Multi-lang Translation

This repository contains an English translation for the DBI homebrew application (version 810) for Nintendo Switch.

Was updating my Switch installation and wanted to bump DBI version from original 500-something. So, fired up Ghidra, 
investigated suspicious chunk of referenced memory and sure enough, there was naive xor cipher on compressed strings.

With cyrillic obviously being inferior to latin alphabet (:D) and taking mostly two bytes per character, there was no 
issue in fiting english texts to available space.

I do not intend to maintain this repo and play cat-and-mouse games with DBI author. Original version was working fine 
for years, so hopefuly there will be at least one similiary stable version released to this point. Would be of course
cool if he stopped playing princess and released multilanguage version, yet I dont expect that.

Also, I guess we will finally see for sure if there is any console bricking code and if he wants to exercise it.

Everything should be hopefuly clear from code. AI-generated generic readme follows.

## Important Disclaimers

### Author Controversy
This translation is provided independently and is not affiliated with or endorsed by the original DBI author. Users should be aware of ongoing community discussions regarding the original software and make informed decisions about its use.

### Official Release Info
- The latest official upstream build that still ships with English is [DBI 658](https://github.com/rashevskyv/dbi/releases/tag/658).

### Author Warning on Independent Translations
> 04 Jul 2023
> **Me:** Will you mind if the translation is added as an NRO patch?
> **duckbill:** If you want to add it as a patch go ahead, but warn users that there may be surprises. 486 is the last one without checks. Nothing destructive will happen. It will just start forgetting to do what was asked, and the further it goes, the more often it will happen. Stribog + part under RSA4096. Cryptography is my diploma specialty :) By the way, no one has managed to crack the protection in ebookdroid yet since 2011 despite 8M+ installs.

### Version Compatibility
- This tooling has only been verified against DBI 810. Later releases removed the embedded translation payload and relocated the font assets to address 0x75A5E0, so this pipeline will not work beyond version 810 without additional reverse engineering.

### Backup Your Console
**Before using any homebrew software, create a complete backup of your console:**
- NAND backup
- Console keys (prod.keys, title.keys)
- SD card contents

Store these backups in multiple secure locations. Console bricks can and do happen.

### No Warranties
This translation is provided as-is with no guarantees whatsoever. The author of this translation accepts no responsibility for any damage, data loss, console bricks, account bans, or other issues that may arise from using this modified software. Use at your own risk.

### Potential Countermeasures
The original author may implement measures in future DBI releases to detect or prevent these translations from functioning. This repository may become obsolete without warning.

### Maintenance Notice
This repository is not actively maintained. Future DBI updates will likely break compatibility, and no fixes are planned. The community is free to fork, modify, and distribute this work as needed.

## Technical Notes

### Testing Status
This translation has received limited testing beyond basic functionality:
- SD card browsing
- Application installation via MTP

Simply just clicked through menus and everything seemed to be reasonably working. No immediate console combustion.

### Translation Method
The translation was generated primarily through automated tools (Perplexity AI) with manual corrections (because I dont speak Russian, obviously). 
Some translations may be imprecise or contextually incorrect. Just linked the english/russian readmes for context.

### Code Quality
This is experimental software built on previous vibe-coded python porn. The codebase is functional but not production-ready.

### String Placeholder Matching
When modifying translations, ensure string placeholders match between original and translated files. Use the `--keys` parameter 
and diff the resulting files to identify critical changes that could break functionality. This is just basic test, best
would be of course manualy checking all strings.

## Usage

### Quick Start (Version 810)
```bash
git clone <repository-url>
cd <repository-directory>
make translate
```

To build a single language and place the NRO in `out/dbi/`, run:

```bash
make translate-lang LANGUAGE=<code>
```

The helper script `scripts/build_translations.py` powers these targets, so you can also run it directly if you need custom workflows (see `--help`).

### Manual Usage
The `dbipatcher` utility provides several operations:

```
Usage: ./bin/dbipatcher [OPTIONS]

Options:
  -b, --binary FILE      Input binary file to patch
  -p, --patch FILE       Patch file to apply
  -o, --output FILE      Output file or directory
  -k, --keys FILE        Output file or directory
  -s, --slot NUMBER      Slot index for patch application
  -e, --extract FILE     Extract payloads from a DBI binary
  -c, --convert FILE     Convert payload or translation file
  -h, --help             Display this help message

Examples:
  # Extract payloads from DBI.nro into folder DBI_extract
     ./bin/dbipatcher --extract DBI.nro --output DBI_extract

  # Convert extracted payload 6.bin into an editable text file
     ./bin/dbipatcher --convert DBI_extract/6.bin --output translation.txt --keys keylist.txt

  # Convert edited translations back into binary form
     ./bin/dbipatcher --convert translation.txt --output DBI_extract/6.bin --keys keylist.txt

  # Apply patch 6.bin to DBI.nro at slot 6 and write patched binary
     ./bin/dbipatcher --patch 6.bin --binary DBI.nro --slot 6 --output DBI.patched.nro
```

## Legal Notice

This translation is distributed for educational and interoperability purposes. Users are responsible for complying with applicable laws and terms of service in their jurisdiction.

## License

This translation work is released into the public domain. The original DBI software remains under its original license terms.
