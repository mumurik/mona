SRCTOP=../..
include $(SRCTOP)/Makefile.inc

CRT_OBJECT   = monapi_crt.o
LIB_STATIC   = libmonapi.a
TARGET       = $(CRT_OBJECT) $(LIB_STATIC)

ifneq ($(BUILD_TARGET),ELF)
LIB_IMPORT   = libmonapi-imp.a
LIB_DYNAMIC  = MONAPI.DLL
LIB_DYNAMIC5 = MONAPI.DL5
TARGET      += $(LIB_IMPORT) $(LIB_IMPORT) $(LIB_DYNAMIC) $(LIB_DYNAMIC5)
endif

SOURCES = monapi_impl.cpp \
          string.cpp \
          syscall.cpp \
          CString.cpp \
          MemoryManager.cpp \
          MemoryMap.cpp \
          Message.cpp \
          MonaApplication.cpp \
          Mutex.cpp \
          Observable.cpp \
          Random.cpp \
          Screen.cpp \
          System.cpp \
          VirtualScreen.cpp \
          mt19937ar.cpp \
          Keys.cpp \
          messages.cpp \
          cmemoryinfo.cpp \
          io.cpp \
          Stream.cpp \
          Thread.cpp \
          stub_for_linker.cpp\
          power.cpp\
          uitoa.cpp\
          get_digits.cpp\
          strstr_n.cpp\
          audio/Beep.cpp \
          audio/mml/Config.cpp \
          audio/mml/Element.cpp \
          audio/mml/Parser.cpp \
          terminal/CommandCreator.cpp \
          terminal/CommandParser.cpp \
          terminal/Util.cpp \
          net.cpp \
          BufferReceiver.cpp \
          text/Line.cpp
CSOURCES = FreeBSD/qdivrem.c FreeBSD/udivdi3.c


NASMSRCS = _alloca.asm

OBJECTS = $(SOURCES:.cpp=.o) $(NASMSRCS:.asm=.o) $(CSOURCES:.c=.o)
#INCLUDE = -I$(INCDIR) -I. -I..
INCLUDE = -I. -I.. -I$(INCDIR) -I$(INCDIR)/stlport/
all : $(TARGET)

$(CRT_OBJECT): monapi_crt.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $(INCLUDE) $<

$(LIB_STATIC): $(OBJECTS)
	rm -f $@
	$(AR) $@ $(OBJECTS)
	$(RANLIB) $@

ifneq ($(BUILD_TARGET),ELF)
$(LIB_IMPORT): $(LIB_DYNAMIC)
$(LIB_DYNAMIC): $(OBJECTS)
	$(LD) -e _dllmain --export-all-symbols --out-implib $(LIB_IMPORT) -o $@ $(OBJECTS) -Map monapi.map
	echo $(STRIP)
	$(STRIP) $@

$(LIB_DYNAMIC5): $(LIB_DYNAMIC)
#	bzip2 -c $(LIB_DYNAMIC) > $@
	bim2bin in:$(LIB_DYNAMIC) out:$@ -osacmp -tek5 BS:0 eprm:z0
endif

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) -c -o $(<:.cpp=.o) $(CXXFLAGS) $(INCLUDE) $<

.SUFFIXES: .c .o
.c.o:
	$(CC) -c -o $(<:.c=.o) $(CXXFLAGS) $(INCLUDE) $<



.SUFFIXES: .asm .o
.asm.o:
	$(NASM) $(NFLAGS) -o $@ $<

depend dependencies:
	$(CXX) -MM $(INCLUDE) $(CXXFLAGS) monapi_crt.cpp $(SOURCES) > dependencies

clean:
	rm -f $(TARGET) $(OBJECTS) dependencies

install: $(TARGET)
	mkdir -p $(LIBDIR)
	$(INSTALL) -p -m 0644 $(LIB_STATIC) $(LIBDIR)/
ifneq ($(BUILD_TARGET),ELF)
	$(INSTALL) -p -m 0644 $(LIB_IMPORT) $(LIBDIR)/
	mkdir -p $(BINDIR)/LIBS
	$(INSTALL) -p $(LIB_DYNAMIC5) $(BINDIR)/LIBS/
	$(INSTALL) -p -m 0644 $(CRT_OBJECT) $(LIB_STATIC) $(LIBDIR)/
endif
	$(INSTALL) -p $(CRT_OBJECT) $(LIBDIR)

uninstall:
	rm -f $(LIBDIR)/$(CRT_OBJECT) $(LIBDIR)/$(LIB_STATIC)
ifneq ($(BUILD_TARGET),ELF)
	rm -f $(LIBDIR)/$(LIB_IMPORT)
	rm -f $(BINDIR)/LIBS/$(LIB_DYNAMIC5)
endif

-include dependencies
