
.PHONY: all clean check-prototypes

BUILD_DIR = build
SOURCES   = ${:!find src -name \*.c | sort!}

LIBS      = $(BUILD_DIR)/lib64/nvshim.so \
            $(BUILD_DIR)/lib64/nvshim.debug.so \
            $(BUILD_DIR)/lib32/nvshim.so \
            $(BUILD_DIR)/lib32/nvshim.debug.so

CFLAGS    = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-incompatible-pointer-types-discards-qualifiers \
 -shared -fPIC -Wl,--version-script=src/shim.map -I/usr/local/include

all: $(LIBS)

.for b in 32 64

$(BUILD_DIR)/versions$(b).h:
	./utils/symver.rb glibc-2.17-symbols.$(b) > $(.TARGET)

$(BUILD_DIR)/wrappers$(b).h: utils/prototypes.rb $(SOURCES)
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_h.rb -m$(b) $(SOURCES) > $(.TARGET)

$(BUILD_DIR)/wrappers$(b).c: utils/prototypes.rb
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_c.rb glibc-2.17-symbols.$(b) > $(.TARGET)

$(BUILD_DIR)/lib$(b)/nvshim.so:       $(SOURCES) $(BUILD_DIR)/wrappers$(b).c $(BUILD_DIR)/wrappers$(b).h $(BUILD_DIR)/versions$(b).h
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -O2     -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  -lm -pthread

$(BUILD_DIR)/lib$(b)/nvshim.debug.so: $(BUILD_DIR)/lib$(b)/nvshim.so
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -DDEBUG -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  -lm -pthread

.endfor

check-prototypes:
	./utils/prototype-check.rb | /compat/linux/bin/gcc -x c -std=c99 --sysroot=/compat/linux -o /dev/null -

clean:
.for f in $(LIBS)
.  if exists($f)
	rm $f
.  endif
.endfor
.for b in 32 64
.  for f in $(BUILD_DIR)/wrappers$(b).c $(BUILD_DIR)/wrappers$(b).h $(BUILD_DIR)/versions$(b).h
.    if exists($f)
	rm $f
.    endif
.  endfor
.endfor
