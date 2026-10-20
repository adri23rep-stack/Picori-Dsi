E3 host smoke tests:
  sh platform/dsi-engine/tests/run_host_test.sh
  sh platform/dsi-engine/tests/run_host_test.sh /path/to/your/own/eu.gba

Does not build an NDS or prove that AgbMain works.
Runs source files src/main.c, src/common.c, src/flags.c unmodified.
