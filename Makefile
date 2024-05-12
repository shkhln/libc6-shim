
.PHONY: all clean check-prototypes

BUILD_DIR = build
SOURCES   = ${:!find src -name \*.c | sort!}

LIBS      = $(BUILD_DIR)/lib64/libc6.so \
            $(BUILD_DIR)/lib64/libc6-debug.so \
            $(BUILD_DIR)/lib32/libc6.so \
            $(BUILD_DIR)/lib32/libc6-debug.so

CFLAGS    = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-incompatible-pointer-types-discards-qualifiers \
 -shared -fPIC -Wl,-soname,librt.so.1 -Wl,--version-script=src/shim.map -I/usr/local/include

CCTYPE != $(CC) --version | grep -q clang && echo clang || true
.if $(CCTYPE) == "clang"
CFLAGS32  = -mstack-alignment=16 -mstackrealign # helps with movaps (?) crashes in steamclient.so a bit
.endif

LDFLAGS64 = -lexecinfo -lm -pthread
LDFLAGS32 = -lexecinfo -lm -pthread -Wl,-z,notext # "relocation R_386_PC32 cannot be used against symbol _setjmp"

GCC_VER ?= 9

.if exists(/usr/local/lib/gcc${GCC_VER})
LIBS += $(BUILD_DIR)/lib64/fakecxxrt.so
LIBS += $(BUILD_DIR)/lib32/fakecxxrt.so
.endif

all: $(LIBS) lib32 lib64

.for b in 32 64

lib$(b):
	ln -s $(BUILD_DIR)/lib$(b) lib$(b)

$(BUILD_DIR)/versions$(b).h:
	mkdir -p $(BUILD_DIR)
	./utils/symver.rb src/glibc-symbols.$(b) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/wrappers$(b).h: src/prototypes.rb $(SOURCES)
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_h.rb -m$(b) $(SOURCES) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/wrappers$(b).c: src/prototypes.rb
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_c.rb src/glibc-symbols.$(b) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/lib$(b)/libc6.so:       $(SOURCES) $(BUILD_DIR)/wrappers$(b).c $(BUILD_DIR)/wrappers$(b).h $(BUILD_DIR)/versions$(b).h $(BUILD_DIR)/lib$(b)/dummy-librt.so
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -O2     -m$(b) $(CFLAGS) ${CFLAGS$(b)} -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  $(BUILD_DIR)/lib$(b)/dummy-librt.so \
	  ${LDFLAGS$(b)}

$(BUILD_DIR)/lib$(b)/libc6-debug.so: $(BUILD_DIR)/lib$(b)/libc6.so $(BUILD_DIR)/lib$(b)/dummy-librt.so
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -DDEBUG -m$(b) $(CFLAGS) ${CFLAGS$(b)} -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  $(BUILD_DIR)/lib$(b)/dummy-librt.so \
	  ${LDFLAGS$(b)}

$(BUILD_DIR)/lib$(b)/dummy-librt.so:
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -m$(b) -shared -fPIC -Wl,-soname,bsd-librt.so.1 -o $(.TARGET)

$(BUILD_DIR)/lib$(b)/fakecxxrt.so:
	mkdir -p $(BUILD_DIR)/lib$(b)
	gcc${GCC_VER} -m$(b) -shared -fPIC \
	  -Wl,--version-script=src/fakecxxrt.map \
	  -Wl,-soname,libcxxrt.so.1 \
	  -Wl,-rpath=/usr/local/lib${b:S/64//}/gcc${GCC_VER} \
	  -L/usr/local/lib${b:S/64//}/gcc${GCC_VER} \
	  -lstdc++ \
	  -o $(.TARGET)

.endfor

check-prototypes:
	./utils/prototype-check.rb | /compat/linux/bin/gcc -x c -std=c99 --sysroot=/compat/linux -o /dev/null -

clean:
.for f in $(LIBS) lib32 lib64
.  if exists($f)
	rm $f
.  endif
.endfor
.for b in 32 64
.  for f in $(BUILD_DIR)/wrappers$(b).c $(BUILD_DIR)/wrappers$(b).h $(BUILD_DIR)/versions$(b).h $(BUILD_DIR)/lib$(b)/dummy-librt.so
.    if exists($f)
	rm $f
.    endif
.    if exists($f.tmp)
	rm $f.tmp
.    endif
.  endfor
.endfor
