CC := gcc
LINK := gcc
OBJCOPY := objcopy
OBJDUMP := objdump

BUILD_DIR := build
OUTPUT := launch

SRC_DIR := src
INC_DIR := src
FREETYPE_SRC_DIR := lib/freetype/src
FREETYPE_INC_DIR := lib/freetype/include

MAIN_SRC := $(wildcard $(SRC_DIR)/*.c)
MAIN_OBJ := $(addprefix $(BUILD_DIR)/, $(notdir $(MAIN_SRC:.c=.o)))

MAIN_FREETYPE_OBJ := \
	$(BUILD_DIR)/freetype/ftsystem.o \
	$(BUILD_DIR)/freetype/ftinit.o \
	$(BUILD_DIR)/freetype/ftdebug.o \
	$(BUILD_DIR)/freetype/ftbase.o \
	$(BUILD_DIR)/freetype/ftbbox.o \
	$(BUILD_DIR)/freetype/ftglyph.o \
	$(BUILD_DIR)/freetype/ftbdf.o    \
	$(BUILD_DIR)/freetype/ftbitmap.o \
	$(BUILD_DIR)/freetype/ftcid.o    \
	$(BUILD_DIR)/freetype/ftfstype.o \
	$(BUILD_DIR)/freetype/ftgasp.o   \
	$(BUILD_DIR)/freetype/ftgxval.o  \
	$(BUILD_DIR)/freetype/ftmm.o     \
	$(BUILD_DIR)/freetype/ftotval.o  \
	$(BUILD_DIR)/freetype/ftpatent.o \
	$(BUILD_DIR)/freetype/ftpfr.o    \
	$(BUILD_DIR)/freetype/ftstroke.o \
	$(BUILD_DIR)/freetype/ftsynth.o  \
	$(BUILD_DIR)/freetype/fttype1.o  \
	$(BUILD_DIR)/freetype/ftwinfnt.o \
	$(BUILD_DIR)/freetype/ftcache.o \
	$(BUILD_DIR)/freetype/truetype.o \
	$(BUILD_DIR)/freetype/raster.o \
	$(BUILD_DIR)/freetype/sfnt.o \
	$(BUILD_DIR)/freetype/psnames.o \
	$(BUILD_DIR)/freetype/md5.o \
	$(BUILD_DIR)/freetype/smooth.o \
	$(BUILD_DIR)/freetype/dlgwrap.o

OBJ = $(MAIN_OBJ)
OBJ += $(MAIN_FREETYPE_OBJ)

CCARG := -fno-common -Wall
CCARG += -fshort-enums
CCARG += -O2 -fno-lto -ffunction-sections -fdata-sections
CCARG += -I./lib/freetype/include -I./lib/freetype/include/freetype
CCARG += -I./lib/freetype/include/freetype/config -I./lib/freetype/include/freetype/internal
CCARG += -I./lib/freetype/include/freetype/internal/services

LINKARG += -O2 -fno-lto -Wl,--gc-sections

all : makedir \
	$(BUILD_DIR)/$(OUTPUT)

makedir : | $(BUILD_DIR) $(BUILD_DIR)/freetype
$(BUILD_DIR) $(BUILD_DIR)/freetype :
	mkdir $@

$(BUILD_DIR)/$(OUTPUT) : $(OBJ)
	$(CC) $(LINKARG) $^ -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(CCARG) -I./$(INC_DIR) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/freetype/ftbase.o : $(FREETYPE_SRC_DIR)/base/ftbase.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftsystem.o : $(FREETYPE_SRC_DIR)/base/ftsystem.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftinit.o : $(FREETYPE_SRC_DIR)/base/ftinit.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftdebug.o : $(FREETYPE_SRC_DIR)/base/ftdebug.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftbbox.o : $(FREETYPE_SRC_DIR)/base/ftbbox.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftglyph.o : $(FREETYPE_SRC_DIR)/base/ftglyph.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftbdf.o : $(FREETYPE_SRC_DIR)/base/ftbdf.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftbitmap.o : $(FREETYPE_SRC_DIR)/base/ftbitmap.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftcid.o : $(FREETYPE_SRC_DIR)/base/ftcid.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@ 
$(BUILD_DIR)/freetype/ftfstype.o : $(FREETYPE_SRC_DIR)/base/ftfstype.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@ 
$(BUILD_DIR)/freetype/ftgasp.o : $(FREETYPE_SRC_DIR)/base/ftgasp.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@ 
$(BUILD_DIR)/freetype/ftgxval.o : $(FREETYPE_SRC_DIR)/base/ftgxval.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftmm.o : $(FREETYPE_SRC_DIR)/base/ftmm.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftotval.o : $(FREETYPE_SRC_DIR)/base/ftotval.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftpatent.o : $(FREETYPE_SRC_DIR)/base/ftpatent.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftpfr.o : $(FREETYPE_SRC_DIR)/base/ftpfr.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftstroke.o : $(FREETYPE_SRC_DIR)/base/ftstroke.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftsynth.o : $(FREETYPE_SRC_DIR)/base/ftsynth.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/fttype1.o : $(FREETYPE_SRC_DIR)/base/fttype1.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/ftwinfnt.o : $(FREETYPE_SRC_DIR)/base/ftwinfnt.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@
$(BUILD_DIR)/freetype/md5.o : $(FREETYPE_SRC_DIR)/base/md5.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/base/) -c $< -o $@

$(BUILD_DIR)/freetype/ftcache.o : $(FREETYPE_SRC_DIR)/cache/ftcache.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/psnames/) -c $< -o $@

$(BUILD_DIR)/freetype/psnames.o : $(FREETYPE_SRC_DIR)/psnames/psnames.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/psnames/) -c $< -o $@
$(BUILD_DIR)/freetype/raster.o : $(FREETYPE_SRC_DIR)/raster/raster.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/raster/) -c $< -o $@

$(BUILD_DIR)/freetype/sfnt.o : $(FREETYPE_SRC_DIR)/sfnt/sfnt.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/sfnt/) -c $< -o $@
$(BUILD_DIR)/freetype/truetype.o : $(FREETYPE_SRC_DIR)/truetype/truetype.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/truetype/) -c $< -o $@

$(BUILD_DIR)/freetype/smooth.o : $(FREETYPE_SRC_DIR)/smooth/smooth.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/truetype/) -c $< -o $@
$(BUILD_DIR)/freetype/dlgwrap.o : $(FREETYPE_SRC_DIR)/dlg/dlgwrap.c
	$(CC) $(CCARG) $(addprefix -I./, $(FREETYPE_SRC_DIR)/dlg/) -c $< -o $@

clean :
	rm -f -v $(OBJ) $(BUILD_DIR)/$(OUTPUT) $(OBJ:.o=.lst)

.PHONY : clean