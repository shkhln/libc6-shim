
.PHONY: all clean check-prototypes

BUILD_DIR = build
SOURCES   = ${:!find src -name \*.c | sort!}

LIBS      = $(BUILD_DIR)/lib64/nvshim.so \
            $(BUILD_DIR)/lib64/nvshim.debug.so \
            $(BUILD_DIR)/lib32/nvshim.so \
            $(BUILD_DIR)/lib32/nvshim.debug.so

CFLAGS    = -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-incompatible-pointer-types-discards-qualifiers \
 -shared -fPIC -Wl,-soname,librt.so.1 -Wl,--version-script=src/shim.map -I/usr/local/include

GCC_VER ?= 9

.if exists(/usr/local/lib/gcc${GCC_VER})
LIBS += $(BUILD_DIR)/lib64/fakecxxrt.so
LIBS += $(BUILD_DIR)/lib32/fakecxxrt.so
.endif

all: $(LIBS)

.for b in 32 64

$(BUILD_DIR)/versions$(b).h:
	./utils/symver.rb glibc-2.17-symbols.$(b) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/wrappers$(b).h: utils/prototypes.rb $(SOURCES)
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_h.rb -m$(b) $(SOURCES) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/wrappers$(b).c: utils/prototypes.rb
	mkdir -p $(BUILD_DIR)
	./utils/wrappers_c.rb glibc-2.17-symbols.$(b) > $(.TARGET).tmp && mv $(.TARGET).tmp $(.TARGET)

$(BUILD_DIR)/lib$(b)/nvshim.so:       $(SOURCES) $(BUILD_DIR)/wrappers$(b).c $(BUILD_DIR)/wrappers$(b).h $(BUILD_DIR)/versions$(b).h $(BUILD_DIR)/lib$(b)/dummy-librt.so
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -O2     -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  $(BUILD_DIR)/lib$(b)/dummy-librt.so \
	  -lm -pthread

$(BUILD_DIR)/lib$(b)/nvshim.debug.so: $(BUILD_DIR)/lib$(b)/nvshim.so $(BUILD_DIR)/lib$(b)/dummy-librt.so
	mkdir -p $(BUILD_DIR)/lib$(b)
	$(CC) -DDEBUG -m$(b) $(CFLAGS) -o $(.TARGET) $(SOURCES) \
	  -include $(BUILD_DIR)/versions$(b).h \
	  -include $(BUILD_DIR)/wrappers$(b).h \
	  $(BUILD_DIR)/wrappers$(b).c \
	  $(BUILD_DIR)/lib$(b)/dummy-librt.so \
	  -lm -pthread

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
.for f in $(LIBS)
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
