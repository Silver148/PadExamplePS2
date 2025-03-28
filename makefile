EE_BIN = padexample.elf
EE_OBJS = padexample.o 
EE_LIBS = -lkernel -ldebug -lpad


all: $(EE_BIN)
		$(EE_STRIP) --strip-all $(EE_BIN)

clean:
		rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal