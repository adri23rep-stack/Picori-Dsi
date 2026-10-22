# Stage E24: guarded original GameTask TRANSITION+INIT+first InitRoom prefix;
# fail-closed immediately before unported room, audio and entity services.
# Native devkitARM compilation and melonDS execution require validation on PC.
.SUFFIXES:
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM and DEVKITPRO before calling make)
endif
STAGE_DIR := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
ROOT_DIR := $(abspath $(STAGE_DIR)/../..)
include $(DEVKITARM)/ds_rules

GAME_TITLE := TMC DSi Engine E39
GAME_SUBTITLE1 := Original game source
GAME_SUBTITLE2 := Original Link / EU ROM
TARGET := tmc-dsi-engine-e29
BUILD := build
ARCH := -march=armv5te -mtune=arm946e-s -mthumb
# The original port uses old-style C declarations. GCC 15 defaults to GNU23,
# where void f() means no arguments; use GNU11 to retain upstream C semantics.
CFLAGS := -std=gnu11 -O2 -g -w -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
  $(ARCH) -DPC_PORT -DTMC_DSI -DTMC_DSI_SAFE_GAME_BOOT -DTMC_DSI_E26_ORIGINAL_MESSAGE -DTMC_DSI_E28_ORIGINAL_INITROOM -DTMC_DSI_E29_ENTITY_CORE -DTMC_DSI_E30_PREP -DTMC_DSI_E31_LIFECYCLE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
  -include $(ROOT_DIR)/include/region.h $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs $(ARCH) -Wl,-Map,$(notdir $*.map) -Wl,--gc-sections \
  -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=DispReset -Wl,--wrap=MemClear -Wl,--wrap=zMallocInit -Wl,--wrap=MessageInitialize -Wl,--wrap=SetFade -Wl,--wrap=EraseAllEntities -Wl,--wrap=InitRoom -Wl,--wrap=ExecuteScript -Wl,--wrap=MemFill32
LIBS := -lfat -lnds9
LIBDIRS := $(LIBNDS) $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(STAGE_DIR)/$(TARGET)
export VPATH := $(STAGE_DIR)/source:$(STAGE_DIR)/generated:$(ROOT_DIR)/src:$(ROOT_DIR)/src/object:$(ROOT_DIR)/src/data:$(ROOT_DIR)/platform/dsi/source
export DEPSDIR := $(STAGE_DIR)/$(BUILD)
# Original main/common/flags/title compiled unmodified. The title asset bridge
# loads ROM bytes into GBA shadow; E7 displays BG2, E8/E9 draw the
# forest; E10 decodes original OBJ records; E11 executes the original
# title-screen object state machine with a bounded title-only entity pool.
# E19: original title-to-file-select handoff, isolated RAM-only slots,
# real file-select asset groups 5/6/15 + palette 9 and preferences.
# Cross-palette RAM copying is supported. FileSelectTask remains uncalled.
# E24 executes Transition and Init, plus a ONE-SHOT safe prefix of the
# ORIGINAL GameMain_InitRoom using src/game.c with DSi-only compile guards.
# The remaining room/graphics/entity/save systems are NOT implemented.
# The real screen stays on the title; this is not a playable build.
CFILES := archway.c dsi_room_archway_e39.c dsi_room_archway_gfx_e39.c dsi_room_link_origin_e39.c dsi_room_geometry_e38.c dsi_room_collision_e36.c dsi_room_link_e35.c tmc_link_sprite.c dsi_room_bed_gfx_e34.c dsi_room_objects_e33.c dsi_room_object_tick_e33.c dsi_priority_e32.c dsi_lifecycle_services_e31.c dsi_lifecycle_state_e31.c dsi_layer_resolve_e31.c dsi_room_spawn_e31.c dsi_entity_search_probe_e30.c dsi_aux_player_probe_e30.c dsi_manager_pool_probe_e30.c dsi_room_spawn_audit_e30.c dsi_room_entity_params_core_e30.c dsi_room_registry_probe_e30.c dsi_room_entity_register_core_e30.c dsi_entity_allocator_core_e30.c dsi_room_entity_list_core_e30.c dsi_room_entity_spawn_core_e30.c dsi_room_property_native_e30.c dsi_room_property_data_e30.c dsi_room_properties_e30.c dsi_entity_core_e29.c dsi_entity_state_e29.c dsi_collision_core_e29.c dsi_collision_state_e29.c dsi_sprite_layer_e29.c dsi_player_state_e29.c dsi_e30_gate.c areaMetadata.c message.c dsi_area_headers.c dsi_room_resources.c dsi_entity_lists.c dsi_player_preflight.c dsi_room_preview.c gameUtils.c fade.c game.c playerUtils.c save.c dsi_game_native.c dsi_room_tiles.c dsi_room_maps.c dsi_room_probe.c dsi_game_bootstrap.c dsi_game_transition.c boot_dsi.c main.c common.c flags.c title.c tmc_rom_stream.c dsi_gba_bus.c dsi_engine_state.c dsi_rom_refs.c dsi_title_trace.c dsi_title_assets.c dsi_gba_renderer.c dsi_title_composite.c dsi_title_live.c dsi_title_sprites.c dsi_title_object_gfx.c dsi_title_entities.c dsi_title_runner.c dsi_file_select_probe.c dsi_file_select_palette.c fileselect.c titleScreenObject.c objectDefinitions.c
export LD := $(CC)
export OFILES := $(CFILES:.c=.o)
export INCLUDE := -I$(STAGE_DIR)/source -I$(ROOT_DIR) -I$(ROOT_DIR)/include \
  -I$(ROOT_DIR)/port -I$(ROOT_DIR)/build/EU -I$(ROOT_DIR)/platform/dsi/source \
  $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(STAGE_DIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
.PHONY: all clean test-host test-title test-assets test-render test-e8 audit-title
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@python3 $(STAGE_DIR)/tools/generate_lifecycle_e31.py $(ROOT_DIR) $(STAGE_DIR)/generated
	@sh $(STAGE_DIR)/tools/generate_entity_core_e29.sh $(ROOT_DIR)/src/entity.c $(STAGE_DIR)/generated/dsi_entity_core_e29.c
	@sh $(STAGE_DIR)/tools/generate_collision_core_e29.sh $(ROOT_DIR)/src/collision.c $(STAGE_DIR)/generated/dsi_collision_core_e29.c
	@sh $(STAGE_DIR)/tools/generate_sprite_layer_e29.sh $(ROOT_DIR)/port/port_linked_stubs.c $(STAGE_DIR)/generated/dsi_sprite_layer_e29.c
	@sh $(STAGE_DIR)/tools/generate_room_entity_core_e30.sh $(ROOT_DIR)/src/room.c $(STAGE_DIR)/generated/dsi_room_entity_list_core_e30.c $(STAGE_DIR)/generated/dsi_room_entity_spawn_core_e30.c $(STAGE_DIR)/generated/dsi_room_entity_params_core_e30.c
	@sh $(STAGE_DIR)/tools/generate_entity_allocator_e30.sh $(ROOT_DIR)/src/entity.c $(ROOT_DIR)/src/room.c $(STAGE_DIR)/generated/dsi_entity_allocator_core_e30.c $(STAGE_DIR)/generated/dsi_room_entity_register_core_e30.c
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(STAGE_DIR)/Makefile
clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).map
test-host:
	@sh $(STAGE_DIR)/tests/run_host_test.sh
	@sh $(STAGE_DIR)/tests/run_title_e5.sh
test-title:
	@sh $(STAGE_DIR)/tests/run_title_e5.sh
test-assets:
	@sh $(STAGE_DIR)/tests/run_title_e6.sh "$(ROM)"
test-render:
	@sh $(STAGE_DIR)/tests/run_renderer_e7.sh "$(ROM)"

test-e8:
	@sh $(STAGE_DIR)/tests/run_title_e8.sh "$(ROM)"

# Host-only audit: compile ORIGINAL title/main and list the missing link dependencies.
audit-title:
	@sh $(STAGE_DIR)/tools/audit_title_host.sh
else
$(OUTPUT).nds: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPSDIR)/*.d
endif

.PHONY: test-e9
test-e9:
	@sh $(STAGE_DIR)/tests/run_title_e9.sh "$(ROM)"

.PHONY: test-e10
test-e10:
	@sh $(STAGE_DIR)/tests/run_title_e10.sh "$(ROM)"

.PHONY: test-e11 test-e11-sanitized
test-e11:
	@sh $(STAGE_DIR)/tests/run_title_e11.sh "$(ROM)"
test-e11-sanitized:
	@sh $(STAGE_DIR)/tests/run_title_e11_sanitized.sh "$(ROM)"

.PHONY: test-e12 test-e12-sanitized
test-e12:
	@sh $(STAGE_DIR)/tests/run_title_e12.sh "$(ROM)"
test-e12-sanitized:
	@sh $(STAGE_DIR)/tests/run_title_e12_sanitized.sh "$(ROM)"

.PHONY: test-e14
test-e13:
	@sh $(STAGE_DIR)/tests/run_title_e13.sh "$(ROM)"

.PHONY: test-e13-sanitized
test-e13-sanitized:
	@sh $(STAGE_DIR)/tests/run_title_e13_sanitized.sh "$(ROM)"

.PHONY: test-e14 test-e14-sanitized
test-e14:
	@sh $(STAGE_DIR)/tests/run_file_select_e14.sh "$(ROM)"
test-e14-sanitized:
	@sh $(STAGE_DIR)/tests/run_file_select_e14_sanitized.sh "$(ROM)"

.PHONY: test-e15 test-e15-sanitized test-all-e15
# PC host-only tests; supply ROM=/path/to/your-own-baserom_eu.gba
test-e15:
	@sh $(STAGE_DIR)/tests/run_file_select_e15.sh "$(ROM)"
test-e15-sanitized:
	@sh $(STAGE_DIR)/tests/run_file_select_e15_sanitized.sh "$(ROM)"
test-all-e15:
	@sh $(STAGE_DIR)/tests/run_all_e15.sh "$(ROM)"

.PHONY: test-e16 test-all-e16 test-e16-sanitized
# Includes the genuine original file-select asset handoff plus cross-bank RAM
# copy regression, without ever touching a persistent save file.
test-e16:
	@sh $(STAGE_DIR)/tests/run_file_select_e16.sh "$(ROM)"
	@sh $(STAGE_DIR)/tests/run_bus_e16.sh "$(ROM)"
test-all-e16:
	@sh $(STAGE_DIR)/tests/run_all_e16.sh "$(ROM)"
test-e16-sanitized:
	@sh $(STAGE_DIR)/tests/run_file_select_e16_sanitized.sh "$(ROM)"
	@DSI_E16_SANITIZE=1 sh $(STAGE_DIR)/tests/run_bus_e16.sh "$(ROM)"

.PHONY: test-e18 test-e18-audit
test-e18:
	@sh $(STAGE_DIR)/tests/run_game_bootstrap_e18.sh "$(ROM)"
test-e18-audit:
	@sh $(STAGE_DIR)/tools/audit_game_e18.sh

.PHONY: test-e19 test-e19-sanitized
test-e19:
	@sh $(STAGE_DIR)/tests/run_game_transition_e19.sh "$(ROM)"
test-e19-sanitized:
	@DSI_E19_SANITIZE=1 sh $(STAGE_DIR)/tests/run_game_transition_e19.sh "$(ROM)"

.PHONY: test-all-e19
test-all-e19:
	@sh $(STAGE_DIR)/tests/run_all_e19.sh "$(ROM)"

.PHONY: test-e20 test-e20-sanitized test-all-e20
test-e20:
	@sh $(STAGE_DIR)/tests/run_room_probe_e20.sh "$(ROM)"
test-e20-sanitized:
	@DSI_E20_SANITIZE=1 sh $(STAGE_DIR)/tests/run_room_probe_e20.sh "$(ROM)"
test-all-e20:
	@sh $(STAGE_DIR)/tests/run_all_e19.sh "$(ROM)"
	@sh $(STAGE_DIR)/tests/run_room_probe_e20.sh "$(ROM)"
	@DSI_E20_SANITIZE=1 sh $(STAGE_DIR)/tests/run_room_probe_e20.sh "$(ROM)"

.PHONY: test-e21 test-e21-sanitized test-all-e21
test-e21:
	@sh $(STAGE_DIR)/tests/run_room_maps_e21.sh "$(ROM)"
test-e21-sanitized:
	@DSI_E21_SANITIZE=1 sh $(STAGE_DIR)/tests/run_room_maps_e21.sh "$(ROM)"
test-all-e21:
	@$(MAKE) --no-print-directory test-all-e20 ROM="$(ROM)"
	@$(MAKE) --no-print-directory test-e21 ROM="$(ROM)"
	@$(MAKE) --no-print-directory test-e21-sanitized ROM="$(ROM)"

.PHONY: test-e22 test-e22-sanitized test-all-e22
test-e22:
	@sh $(STAGE_DIR)/tests/run_room_tiles_e22.sh "$(ROM)"
test-e22-sanitized:
	@DSI_E22_SANITIZE=1 sh $(STAGE_DIR)/tests/run_room_tiles_e22.sh "$(ROM)"
test-all-e22:
	@$(MAKE) --no-print-directory test-all-e21 ROM="$(ROM)"
	@$(MAKE) --no-print-directory test-e22 ROM="$(ROM)"
	@$(MAKE) --no-print-directory test-e22-sanitized ROM="$(ROM)"

.PHONY: test-e23 test-e23-sanitized
test-e23:
	@sh $(STAGE_DIR)/tests/run_game_native_e23.sh "$(ROM)"
test-e23-sanitized:
	@DSI_E23_SANITIZE=1 sh $(STAGE_DIR)/tests/run_game_native_e23.sh "$(ROM)"

.PHONY: test-e24 test-e24-sanitized
test-e24:
	@sh $(STAGE_DIR)/tests/run_room_prefix_e24.sh "$(ROM)"
test-e24-sanitized:
	@DSI_E24_SANITIZE=1 sh $(STAGE_DIR)/tests/run_room_prefix_e24.sh "$(ROM)"

# E25 host-only framebuffer tests. ROM required only at runtime on the DS.
.PHONY: test-e25
test-e25:
	@sh $(STAGE_DIR)/tests/run_room_preview_e25.sh
