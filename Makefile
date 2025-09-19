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

translate: $(TARGET)
	@$(TARGET) --extract $(DBI_BASE) --output $(TMPDIR)
	@$(TARGET) --convert $(TMPDIR)/rec6.bin --output translate/rec6.$(DBI_LANG).txt --keys $(TMPDIR)/keys_$(DBI_LANG).txt
	@$(TARGET) --convert translate/rec6.$(DBI_TARGET).txt --output $(TMPDIR)/rec6.$(DBI_TARGET).bin --keys $(TMPDIR)/keys_$(DBI_TARGET).txt
	@$(TARGET) --patch $(TMPDIR)/rec6.$(DBI_TARGET).bin --binary $(DBI_BASE) --output $(TMPDIR)/bin/DBI.nro --slot 6

debug: $(TARGET)
	@valgrind $(TARGET)

.PHONY: all clean translate
