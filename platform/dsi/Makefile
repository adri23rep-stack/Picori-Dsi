# Experimental V4 sprite/graphics viewer for Nintendo DSi. Not a playable game.
# Follows the current devkitPro nds-examples / Calico two-pass build layout.
.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM and DEVKITPRO before calling make)
endif

GAME_TITLE := TMC DSi Link V4
GAME_SUBTITLE1 := ROM-backed Link preview
GAME_SUBTITLE2 := No copyrighted game assets

include $(DEVKITARM)/ds_rules

TARGET := tmc-dsi-probe
BUILD := build
SOURCES := source
INCLUDES := source
ARCH := -march=armv5te -mtune=arm946e-s -mthumb
CFLAGS := -O2 -g -Wall -Wextra -ffunction-sections -fdata-sections $(ARCH) $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs $(ARCH) -Wl,-Map,$(notdir $*.map) -Wl,--gc-sections
LIBS := -lfat -lnds9
LIBDIRS := $(LIBNDS) $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/source
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(notdir $(wildcard source/*.c))
export LD := $(CC)
export OFILES := $(CFILES:.c=.o)
export INCLUDE := -I$(CURDIR)/source $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

# DSi-enhanced header; experimental and not verified on a physical console.
$(TARGET).dsi: $(TARGET).nds
	ndstool -c $@ -9 $(TARGET).elf -7 $(CALICO)/bin/ds7_maine.elf \
	  -b $(CALICO)/share/nds-icon.bmp \
	  "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	  -g TMCD 00 "TMC DSi Stage4" -u 00030004

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).dsi $(TARGET).lst

else
$(OUTPUT).nds: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPSDIR)/*.d
endif
