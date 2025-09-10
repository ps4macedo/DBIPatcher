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

translate-810: $(TARGET)
	@$(TARGET) --extract dbi/DBI.810.ru.nro --output /tmp/DBI_810
	@$(TARGET) --convert /tmp/DBI_810/rec6.bin --output translate/rec6.810.ru.txt --keys /tmp/DBI_810/keys_ru.txt
	@$(TARGET) --convert translate/rec6.810.en.txt --output /tmp/DBI_810/rec6.en.bin --keys /tmp/DBI_810/keys_en.txt
	@$(TARGET) --patch /tmp/DBI_810/rec6.en.bin --binary dbi/DBI.810.ru.nro --output /tmp/DBI_810/bin/DBI.810.en.nro --slot 6 

debug: $(TARGET)
	@valgrind $(TARGET)

.PHONY: all clean
