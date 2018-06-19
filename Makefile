default: disassembler interpreter

clean:
	-rm disassembler

check-disassemble: disassembler
	./disassembler bin/1.c.out > out/1.c.txt
	-diff out/1.c.txt ans/1.c.ans.txt
	./disassembler bin/2.c.out > out/2.c.txt
	-diff out/2.c.txt ans/2.c.ans.txt
	./disassembler bin/3.c.out > out/3.c.txt
	-diff out/3.c.txt ans/3.c.ans.txt
	./disassembler bin/4.c.out > out/4.c.txt
	-diff out/4.c.txt ans/4.c.ans.txt
	./disassembler bin/5.c.out > out/5.c.txt
	-diff out/5.c.txt ans/5.c.ans.txt
	./disassembler bin/6.c.out > out/6.c.txt
	-diff out/6.c.txt ans/6.c.ans.txt
	./disassembler bin/7.c.out > out/7.c.txt
	-diff out/7.c.txt ans/7.c.ans.txt

check-interpret: interpreter
	./interpreter bin/1.s.out
	./interpreter bin/2.s.out
	./interpreter bin/3.s.out
	./interpreter bin/4.s.out

disassembler: src/main-disassembler.cpp src/disassembler.cpp src/inst.cpp src/util.cpp
	g++ -std=c++11 -o $@ $^

interpreter: src/main-interpreter.cpp src/interpreter.cpp src/inst.cpp src/util.cpp
	g++ -std=c++11 -o $@ $^

src/main-disassembler.cpp: src/disassembler.hpp
src/main-interpreter.cpp: src/interpreter.hpp
src/disassembler.hpp: src/a.out.hpp src/inst.hpp
src/disassembler.cpp: src/disassembler.hpp src/inst.hpp src/consts.hpp src/util.hpp
src/inst.hpp: src/consts.hpp
src/inst.cpp: src/inst.hpp src/util.hpp
src/interpreter.cpp: src/interpreter.hpp src/inst.hpp src/util.hpp src/consts.hpp src/message.hpp
src/interpreter.hpp: src/inst.hpp src/a.out.hpp src/reg.hpp
src/util.cpp: src/util.hpp
