####################################################################################################
# 
# Minicurl
# Copyright 2019 Jean Diogo (aka Jango) <jeandiogo@gmail.com>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
####################################################################################################
#
# makefile (requires GCC 8.0.0 or later)
#
####################################################################################################

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
