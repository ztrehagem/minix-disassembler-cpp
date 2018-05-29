default: disassembler

clean:
	-rm disassembler

disassembler: src/main.cpp src/disassembler.cpp src/inst.cpp
	g++ -std=c++11 -o $@ $^

src/main.cpp: src/disassembler.hpp
src/disassembler.hpp: src/a.out.hpp src/inst.hpp
src/disassembler.cpp: src/disassembler.hpp src/inst.hpp src/consts.hpp
src/inst.hpp: src/consts.hpp
src/inst.cpp: src/inst.hpp src/disassembler.hpp
src/consts.hpp:
