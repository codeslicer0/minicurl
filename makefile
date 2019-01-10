# makefile

OUT = test.out
SRC = $(wildcard *.cpp)
WRN = -Wall -Wextra -pedantic -Werror -pedantic-errors -Wfatal-errors -Wno-unused-result -Wshadow -Wconversion -Wsign-conversion -Wnull-dereference -Wuseless-cast -Wcast-align
OPT = -pipe -fwhole-program -fPIE -Ofast -march=native -std=c++2a -pthread -fopenmp -lstdc++fs -fno-rtti #-fno-exceptions
LIB = -L/usr/lib/x86_64-linux-gnu -lcurl

.PHONY: $(OUT)

$(OUT):
	@rm -f *~ *.o $@
	@clear
	@clear
	@g++ $(SRC) -o $@ $(WRN) $(OPT) $(LIB)
	@chmod a=rx $@
	@rm -f *~ *.o
	@clear
	@./$@
#
