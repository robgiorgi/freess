#!/bin/make -f

# Common settings
TGT     = freess
DEFIN   = -DSHARING_STAT -DDGEN -DDXP
DIR     = $(TGT)
OBJS    = $(REL)/main.o $(REL)/superscalar.o ./lib/libsui.a
CFLAGS1 = -D_Linux_ $(DEFIN) -I./lib -I./include
CFLAGS2 = -lm
ARFLAGS = r
AR      = ar

# Native build settings (gcc)
CC_NATIVE = gcc

# WebAssembly build settings (emcc)
CC_WASM   = emcc
WASM_FLAGS1 = -O3 $(CFLAGS1) -s USE_SDL=2
# -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s EXIT_RUNTIME=1
WASM_FLAGS2 = -s MODULARIZE=1 -s EXPORT_NAME="freessModule" -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
              -s EXPORTED_RUNTIME_METHODS='["FS", "callMain"]' -s EXIT_RUNTIME=0 \
              --embed-file program-ex1

# Targets

.PHONY: all release debug clean veryclean install test t0 t1 t2 t3 t4 t5 t6 new wasm

all: clean release

release:
	-@mkdir -p Release Debug
	@echo ""
	@echo "------------------- Building $(TGT) Release Version"
	@CCOO="-O3 "; REL=Release; export REL CCOO; $(MAKE) tool CC="$(CC_NATIVE)"

debug:
	-@mkdir -p Release Debug
	@echo ""
	@echo "------------------- Building $(TGT) Debug Version"
	@CCOO="-g "; CCSS=d; REL=Debug; export REL CCOO CCSS; $(MAKE) tool CC="$(CC_NATIVE)"

tool: $(OBJS)
	@echo ""
	@echo "------------------- Building TOOL BINARY"
	$(CC) $(OBJS) $(CFLAGS1) -o $(TGT)$(CCSS) $(CFLAGS2)

wasm: clean
	-@mkdir -p Release Debug
	@echo ""
	@echo "------------------- Building LIBRARY for WebAssembly"
	@cd ./lib && $(MAKE) wasm
	@echo ""
	@echo "------------------- Building $(TGT) WebAssembly Version"
	@REL=Release; CCSS=.js; export REL CCSS; $(MAKE) tool CC="$(CC_WASM)" \
		CFLAGS1="$(WASM_FLAGS1)" CFLAGS2="$(WASM_FLAGS2)"
#	@echo ""
#	@echo "------------------- Linking to WebAssembly module for Browser"
#	$(CC_WASM) $(OBJS) $(WASM_FLAGS) -o $(TGT).js

./lib/libsui.a: ./lib/sui.c ./lib/sui.h
	@echo ""
	@echo "------------------- Building LIBRARY"
	cd ./lib && $(MAKE) || exit 1

$(REL)/main.o: main.c main.h ./lib/sui.h ./lib/libsui.a
	$(CC) $(CFLAGS1) -o $(REL)/main.o -c main.c

$(REL)/superscalar.o: superscalar.c superscalar.h
	$(CC) $(CFLAGS1) -o $(REL)/superscalar.o -c superscalar.c

clean:
	@echo ""
	@echo "------------------- Cleaning build files"
	-rm -rf Release Debug *.ln $(TGT) $(TGT)d $(TGT).exe $(TGT).cpe $(TGT).def \
	           $(TGT).c $(TGT).h $(TGT).tem
	-cd ./lib && $(MAKE) clean || exit 0
	-mkdir -p Release Debug

veryclean: clean
	-rm -f $(TGT) $(TGT)d $(TGT).exe $(TGT)ini $(TGT).cpe $(TGT).def \
	           freess.js freess.wasm freess.data

install:
	cd .. && $(MAKE) install && cd $(TGT)

test: all
	@{ a=`./run-ex1.sh -int no|tail -2`;g=`echo "$$a"|tail -1`; \
           c=`echo "$$a"|head -1|sed 's/.*CK=\([0-9][0-9]*\).*/\1/g'`; \
	   if [ "$$g" = "Goodbye." -a "$$c" = "20" ]; then echo "* TEST1: PASSED"; \
	   else echo "* TEST1: FAILED"; fi; }
	@{ a=`./run-ex2.sh -int no|tail -2`;g=`echo "$$a"|tail -1`; \
           c=`echo "$$a"|head -1|sed 's/.*CK=\([0-9][0-9]*\).*/\1/g'`; \
	   if [ "$$g" = "Goodbye." -a "$$c" = "11" ]; then echo "* TEST2: PASSED"; \
	   else echo "* TEST2: FAILED"; fi; }
	@{ a=`./run-ex3.sh -int no|tail -2`;g=`echo "$$a"|tail -1`; \
           c=`echo "$$a"|head -1|sed 's/.*CK=\([0-9][0-9]*\).*/\1/g'`; \
	   if [ "$$g" = "Goodbye." -a "$$c" = "14" ]; then echo "* TEST3: PASSED"; \
	   else echo "* TEST3: FAILED"; fi; }
	@for i in test*; do \
	   if [ -d $$i ]; then cd $$i; $(MAKE) test; cd ..; fi \
	done

t0:
	cd test0 && ($(MAKE) && $(MAKE) cmp)
t1:
	cd test1 && ($(MAKE) && $(MAKE) cmp)
t2:
	cd test2 && ($(MAKE) && $(MAKE) cmp)
t3:
	cd test3 && ($(MAKE) && $(MAKE) cmp)
t4:
	cd test4 && ($(MAKE) && $(MAKE) cmp)
t5:
	cd test5 && ($(MAKE) && $(MAKE) cmp)
t6:
	cd test6 && ($(MAKE) && $(MAKE) cmp)

new:
	rm -f $(TGT)
	$(MAKE)
