
CFLAGS   =-std=c99 -shared -fPIC -lm -pthread -Wl,--version-script=src/shim.map -I/usr/local/include
SOURCES  =${:!find src -name \*.c!}
BUILD_DIR=build

all: clean build

build: $(BUILD_DIR)/lib64/nvshim.so $(BUILD_DIR)/lib64/nvshim.debug.so $(BUILD_DIR)/lib32/nvshim.so $(BUILD_DIR)/lib32/nvshim.debug.so

build_dir:
	mkdir -p $(BUILD_DIR)/lib32
	mkdir -p $(BUILD_DIR)/lib64

$(BUILD_DIR)/wrappers.c: build_dir
	cc $(CFLAGS) -o $(BUILD_DIR)/shim.tmp $(SOURCES)
	./utils/wrappergen.rb required_symbols.396.24 $(BUILD_DIR)/shim.tmp > $(BUILD_DIR)/wrappers.c
	rm $(BUILD_DIR)/shim.tmp

$(BUILD_DIR)/lib64/nvshim.so:       build_dir $(BUILD_DIR)/wrappers.c
	cc -O3     -m64 $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c

$(BUILD_DIR)/lib64/nvshim.debug.so: build_dir $(BUILD_DIR)/wrappers.c
	cc -DDEBUG -m64 $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c

$(BUILD_DIR)/lib32/nvshim.so:       build_dir $(BUILD_DIR)/wrappers.c
	cc -O3     -m32 $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c

$(BUILD_DIR)/lib32/nvshim.debug.so: build_dir $(BUILD_DIR)/wrappers.c
	cc -DDEBUG -m32 $(CFLAGS) -o $(.TARGET) $(SOURCES) $(BUILD_DIR)/wrappers.c

clean:
.for f in shim.tmp wrappers.c lib64/nvshim.so lib64/nvshim.debug.so lib32/nvshim.so lib32/nvshim.debug.so
.if exists($(BUILD_DIR)/$f)
	rm $(BUILD_DIR)/$f
.endif
.endfor
