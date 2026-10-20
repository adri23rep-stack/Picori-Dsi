E17: ROM-only 72 KiB staging window for the ORIGINAL file-select palette
routine, with 17-frame/color-bank validation. Test with an owned EUR ROM.

sh platform/dsi-engine/tests/run_file_select_e17.sh /path/baserom_eu.gba
sh platform/dsi-engine/tests/run_file_select_e17_sanitized.sh /path/baserom_eu.gba
sh platform/dsi-engine/tests/run_all_e17.sh /path/baserom_eu.gba

These are host tests. The Nintendo DSi binary is not compiled by these tests.
