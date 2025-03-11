# Classics Patch localization

This directory is used for creating localizations for new strings from **Classics Patch** in order to translate it into other languages. These localizations should be installed alongside official game localizations, which have been archived here:  
https://github.com/DreamyCecil/SE1-Localization

Each localization sits under its own folder named after a language code according to the [ISO 639-2 standard](https://en.wikipedia.org/wiki/List_of_ISO_639-2_codes) in uppercase (e.g. "English" becomes "ENG").

Localization creation tools are located under the `ExtractionTool/` directory, which is a **Depend** utility that ships with **Serious Sam: The Second Encounter v1.07** by default.

# How to create new languages

1. Run `CreateLocalization.bat` script and type in the language code (e.g. `HRV`). This script will copy all the files that need to be translated from English and also create empty translation tables.
2. Run `Update<lang code>.bat` script (e.g. `UpdateHRV.bat`) that the previous script has generated in order to fill translation tables with new strings.

# How to update existing languages

Run `Update<lang code>.bat` script in order to fill translation tables with new strings and remove ones that don't exist anymore.

> [!IMPORTANT]
> In order to update translation tables, you need to compile the project under the `Release_TSE107` configuration to allow the extraction tool to scan the binaries for strings!  
> Alternatively, to scan binaries of an existing build (packaged in a `.zip` file), place all the files from the `Bin/` directory under `Bin/Release_TSE107` of the `SuperProject` repository.

# How to assemble GRO packages with localization

Run `Assemble<lang code>.bat` script in order to pack localization into a GRO package ready for distribution alongside the patch.

> [!IMPORTANT]
> You need to specify [7-Zip](https://www.7-zip.org/) installation directory in the **PATH** environment variable for the assembly script to work!
