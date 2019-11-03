BUILD_DIR = build
SOURCES   = ${:!find src -name \*.c | sort!}

LIBS      = $(BUILD_DIR)/lib64/nvshim.so \
            $(BUILD_DIR)/lib64/nvshim.debug.so \
            $(BUILD_DIR)/lib32/nvshim.so \
            $(BUILD_DIR)/lib32/nvshim.debug.so

CFLAGS    = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-incompatible-pointer-types-discards-qualifiers \
 -shared -fPIC -Wl,--version-script=src/shim.map -I/usr/local/include

build: $(LIBS)

$(BUILD_DIR)/wrappers.c:
	mkdir -p $(BUILD_DIR)
	cc $(CFLAGS) -o $(BUILD_DIR)/shim.tmp $(SOURCES)
	./utils/wrappergen.rb glibc-2.17-symbols $(BUILD_DIR)/shim.tmp > $(BUILD_DIR)/wrappers.c

.for b in 32 64

$(BUILD_DIR)/lib$(b)/nvshim.so:       $(SOURCES) $(BUILD_DIR)/wrappers.c
	mkdir -p $(BUILD_DIR)/lib$(b)
	clang60 -O3     -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c -lm -pthread

$(BUILD_DIR)/lib$(b)/nvshim.debug.so: $(SOURCES) $(BUILD_DIR)/wrappers.c
	mkdir -p $(BUILD_DIR)/lib$(b)
	clang60 -DDEBUG -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c -lm -pthread

.endfor

clean:
.for f in $(LIBS) $(BUILD_DIR)/shim.tmp $(BUILD_DIR)/wrappers.c
.  if exists($f)
	rm $f
.  endif
.endfor
