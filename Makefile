default: disassembler

clean:
	-rm disassembler

disassembler: src/main.cpp src/disassembler.cpp
	g++ -std=c++11 -o $@ $^

src/main.cpp: src/disassembler.hpp
src/disassembler.hpp: src/a.out.hpp
src/disassembler.cpp: src/disassembler.hpp
