# wtssynth & nrsynth test
PROGRAM_WTSSYNTH_TEST = wtssynth_test
OBJS_WTSSYNTH_TEST	= freq_table.o wtssynth_def.o wtssynth_test.o wtssynth.o nrsynth.o xor128.o audiobuf.o misc.o sdelay.o midievent.o

# mid2wav 
PROGRAM = mid2wav
OBJS	= mid2wav.o soundmodule.o midifile.o midievent.o freq_table.o wtssynth_def.o wtssynth.o nrsynth.o xor128.o audiobuf.o misc.o sdelay.o

# mid2wav.dll should be compiled by MinGW gcc
PROGRAM_DLL = mid2wav.dll
PROGRAM_DLL_OBJS	= mid2wavdll.o soundmodule.o midifile.o midievent.o freq_table.o wtssynth_def.o wtssynth.o nrsynth.o xor128.o audiobuf.o misc.o sdelay.o

# mid2wav.dll test
PROGRAM_TEST_DLL = mid2wavdll_test
PROGRAM_TEST_DLL_OBJS	= mid2wavdll_test.o 

CC	= gcc
RUBY	= ruby
#CFLAGS	= -g -Wall -O2 -DWINDOWS
CFLAGS	= -g -Wall -DWINDOWS

.SUFFIXIES:	.c .o

all:	$(PROGRAM) $(PROGRAM_DLL) $(PROGRAM_TEST_DLL) $(PROGRAM_WTSSYNTH_TEST) 

$(PROGRAM)	:$(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $^

$(PROGRAM_DLL)	:$(PROGRAM_DLL_OBJS)
	$(CC) $(CFLAGS) -shared -o $(PROGRAM_DLL) $^ -Wl,--out-implib,libmid2wav_dll.a

$(PROGRAM_TEST_DLL)	:$(PROGRAM_TEST_DLL_OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM_TEST_DLL) $^

$(PROGRAM_WTSSYNTH_TEST)	:$(OBJS_WTSSYNTH_TEST)
	$(CC) $(CFLAGS) -o $(PROGRAM_WTSSYNTH_TEST) $^

freq_table.c:	freq_table.rb
	$(RUBY) $<

wtssynth_def.c:	wtssynth_def.rb
	$(RUBY) $<

freq_table.h:	freq_table.rb
	$(RUBY) $<
.c.o:
	$(CC) $(CFLAGS) -c $<


.PHONY:	clean
clean:
	$(RM) $(PROGRAM) $(PROGRAM_DLL) $(PROGRAM_WTSSYNTH_TEST) $(PROGRAM_TEST_DLL) $(PROGRAM_DLL_OBJS) $(OBJS_WTSSYNTH_TEST) $(PROGRAM_TEST_DLL_OBJS) $(OBJS) freq_table.c freq_table.h wtssynth_def.c libmid2wav_dll.a

