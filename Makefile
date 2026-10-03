TARGET      := ScarletSkips
BUILD       := build
SOURCES     := source
INCLUDES    := include

APP_TITLE       := Scarlet Skips
APP_DESCRIPTION := A stylish jump-rope challenge
APP_AUTHOR      := Scarlet Skips
APP_ICON        := assets/icon.png
APP_PRODUCT_CODE := CTR-H-SKIP
APP_UNIQUE_ID   := 0xF5A11

include $(DEVKITPRO)/devkitARM/base_rules

ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS      := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) -D__3DS__
CFLAGS      += -I$(CURDIR)/$(INCLUDES)
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -specs=3dsx.specs $(ARCH) -Wl,--gc-sections
LIBS        := -lcitro2d -lcitro3d -lctru -lm

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES := $(CFILES:.c=.o)

.PHONY: all clean cia 3dsx

all: 3dsx cia

3dsx: $(OUTPUT).3dsx
cia: $(OUTPUT).cia

$(BUILD):
	@mkdir -p $@

$(BUILD)/%.o: %.c | $(BUILD)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OUTPUT).elf: $(addprefix $(BUILD)/,$(OFILES))
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OUTPUT).smdh: $(APP_ICON)
	bannertool makesmdh -s "$(APP_TITLE)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f visible,allow3d -o $@

$(OUTPUT).3dsx: $(OUTPUT).elf $(OUTPUT).smdh
	3dsxtool --smdh=$(OUTPUT).smdh $(OUTPUT).elf $@

$(OUTPUT).cia: $(OUTPUT).elf $(OUTPUT).smdh
	makerom -f cia -o $@ -target t -exefslogo -elf $(OUTPUT).elf -icon $(OUTPUT).smdh -desc app:4 -major 1 -minor 0 -micro 0 -DAPP_ENCRYPTED=false -DAPP_USE_ON_SD=true -DAPP_TITLE="$(APP_TITLE)" -DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" -DAPP_UNIQUE_ID=$(APP_UNIQUE_ID)

clean:
	rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).3dsx $(OUTPUT).cia $(OUTPUT).smdh

-include $(addprefix $(BUILD)/,$(OFILES:.o=.d))
