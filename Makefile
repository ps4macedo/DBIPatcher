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

LANG_FROM_CFG := $(strip $(shell awk -F= '/^[[:space:]]*lang[[:space:]]*=/{gsub(/^[ \t]+|[ \t]+$$/,"",$$2); print $$2}' $(CONFIG_FILE) 2>/dev/null))
VER_FROM_CFG  := $(strip $(shell awk -F= '/^[[:space:]]*ver[[:space:]]*=/{gsub(/^[ \t]+|[ \t]+$$/,"",$$2); print $$2}'  $(CONFIG_FILE) 2>/dev/null))

DBI_LANG    ?= $(if $(LANG_FROM_CFG),$(LANG_FROM_CFG),en)
DBI_VER ?= $(if $(VER_FROM_CFG),$(VER_FROM_CFG),810)

DBI_ORIG := dbi/DBI.$(DBI_VER).ru.nro
TMPDIR   := /tmp/DBI_$(DBI_VER)

translate: $(TARGET)
	@$(TARGET) --extract $(DBI_ORIG) --output $(TMPDIR)
	@$(TARGET) --convert $(TMPDIR)/rec6.bin --output translate/rec6.ru.txt --keys $(TMPDIR)/keys_ru.txt
	@$(TARGET) --convert translate/rec6.$(DBI_LANG).txt --output $(TMPDIR)/rec6.$(DBI_LANG).bin --keys $(TMPDIR)/keys_$(DBI_LANG).txt
	@$(TARGET) --patch $(TMPDIR)/rec6.$(DBI_LANG).bin --binary $(DBI_ORIG) --output $(TMPDIR)/bin/DBI.nro --slot 6

debug: $(TARGET)
	@valgrind $(TARGET)

.PHONY: all clean translate
