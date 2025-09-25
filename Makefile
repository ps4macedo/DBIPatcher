CC = gcc

CFLAGS =-std=gnu11 \
	-g \
	-I$(SRCDIR)/inc \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-Wno-sign-compare \
	-Wno-unused-function

LDLIBS =-lzstd

SRCDIR = src
BUILDDIR = build
BINDIR = bin

SOURCES = $(shell find $(SRCDIR) -name '*.c')

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

TARGET = $(BINDIR)/dbipatcher

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILDDIR) $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

clean:
	@rm -rf $(BUILDDIR)

run: $(TARGET)
	@$(TARGET)

CONFIG_FILE ?= config.txt
BASE_LANG_CFG   := $(strip $(shell awk -F= '/^[[:space:]]*base_lang[[:space:]]*=/{gsub(/^[ \t]+|[ \t]+$$/,"",$$2); print $$2}' $(CONFIG_FILE) 2>/dev/null))
TARGET_LANG_CFG := $(strip $(shell awk -F= '/^[[:space:]]*target_lang[[:space:]]*=/{gsub(/^[ \t]+|[ \t]+$$/,"",$$2); print $$2}' $(CONFIG_FILE) 2>/dev/null))
VER_FROM_CFG    := $(strip $(shell awk -F= '/^[[:space:]]*ver[[:space:]]*=/{gsub(/^[ \t]+|[ \t]+$$/,"",$$2); print $$2}'  $(CONFIG_FILE) 2>/dev/null))

DBI_LANG   ?= $(if $(BASE_LANG_CFG),$(BASE_LANG_CFG),ru)
DBI_TARGET ?= $(if $(TARGET_LANG_CFG),$(TARGET_LANG_CFG),en)
DBI_VER    ?= $(if $(VER_FROM_CFG),$(VER_FROM_CFG),810)

DBI_BASE := dbi/DBI.$(DBI_VER).$(DBI_LANG).nro
TMPDIR   := /tmp/DBI_$(DBI_VER)

OUT_DIR := out/dbi
TRANSLATE_SCRIPT := scripts/build_translations.py



PYTHON3 ?= python3
FONTDIR := font
UA_FONT_RAW := $(FONTDIR)/0x7555E0_bundle.ttf
UA_FONT_PATCHED := $(TMPDIR)/font/0x7555E0_bundle_patched.ttf
UA_FONT_ADDR ?= 0x7555E0
UA_FONT_MAX ?= 596378

translate: $(TARGET)
	@if [ -n "$(LANGUAGE)" ]; then \
		DBI_VER=$(DBI_VER) DBI_LANG=$(DBI_LANG) DBI_BASE=$(DBI_BASE) DBI_TMPDIR=$(TMPDIR) OUT_DIR=$(OUT_DIR) MAKE=$(MAKE) $(PYTHON3) $(TRANSLATE_SCRIPT) --lang $(LANGUAGE); \
	else \
		DBI_VER=$(DBI_VER) DBI_LANG=$(DBI_LANG) DBI_BASE=$(DBI_BASE) DBI_TMPDIR=$(TMPDIR) OUT_DIR=$(OUT_DIR) MAKE=$(MAKE) $(PYTHON3) $(TRANSLATE_SCRIPT) --all; \
	fi

translate-lang: $(TARGET)
	@if [ -z "$(LANGUAGE)" ]; then echo "LANGUAGE variable is required, e.g. make translate-lang LANGUAGE=en"; exit 1; fi
	@DBI_VER=$(DBI_VER) DBI_LANG=$(DBI_LANG) DBI_BASE=$(DBI_BASE) DBI_TMPDIR=$(TMPDIR) OUT_DIR=$(OUT_DIR) MAKE=$(MAKE) $(PYTHON3) $(TRANSLATE_SCRIPT) --lang $(LANGUAGE)

translate-core: $(TARGET)
	@rm -rf $(TMPDIR)
	@$(TARGET) --extract $(DBI_BASE) --output $(TMPDIR)
	@$(PYTHON3) -c 'from pathlib import Path; from sys import exit; p = Path("translate/rec6.$(DBI_LANG).txt"); p.exists() or exit(0); data = p.read_bytes(); new = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n"); (data != new) and p.write_bytes(new)'
	@$(PYTHON3) -c 'from pathlib import Path; from sys import exit; p = Path("translate/rec6.$(DBI_TARGET).txt"); p.exists() or exit(0); data = p.read_bytes(); new = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n"); (data != new) and p.write_bytes(new)'
	@$(TARGET) --convert $(TMPDIR)/rec6.bin --output translate/rec6.$(DBI_LANG).txt --keys $(TMPDIR)/keys_$(DBI_LANG).txt
	@$(TARGET) --convert translate/rec6.$(DBI_TARGET).txt --output $(TMPDIR)/rec6.$(DBI_TARGET).bin --keys $(TMPDIR)/keys_$(DBI_TARGET).txt
	@$(TARGET) --patch $(TMPDIR)/rec6.$(DBI_TARGET).bin --binary $(DBI_BASE) --output $(TMPDIR)/bin/DBI.nro --slot 6
ifeq ($(DBI_TARGET),ua)
	@mkdir -p $(dir $(UA_FONT_PATCHED))
	@$(PYTHON3) $(FONTDIR)/mirror_glyph.py $(UA_FONT_RAW) $(UA_FONT_PATCHED)
	@$(PYTHON3) $(FONTDIR)/patch_font.py $(TMPDIR)/bin/DBI.nro $(UA_FONT_PATCHED) $(TMPDIR)/bin/DBI.font.nro $(UA_FONT_ADDR) $(UA_FONT_MAX)
	@$(PYTHON3) -c "import shutil; shutil.move(r'$(TMPDIR)/bin/DBI.font.nro', r'$(TMPDIR)/bin/DBI.nro')"
endif
	@$(PYTHON3) scripts/patch_version.py --file $(TMPDIR)/bin/DBI.nro --lang $(DBI_TARGET)

debug: $(TARGET)
	@valgrind $(TARGET)

.PHONY: all clean translate translate-lang translate-core
