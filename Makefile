default: disassembler interpreter

clean:
	-rm disassembler
	-rm interpreter
	-find dist -type f -name '*.o' -delete

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

disassembler: dist/main-disassembler.o dist/disassembler.o dist/machine.o dist/inst.o dist/util.o
	g++ -std=c++11 -o $@ $^

interpreter: dist/main-interpreter.o dist/interpreter.o dist/machine.o dist/inst.o dist/util.o
	g++ -std=c++11 -o $@ $^

dist/main-disassembler.o: src/main-disassembler.cpp src/disassembler.hpp
	g++ -std=c++11 -o $@ -c $<
dist/main-interpreter.o: src/main-interpreter.cpp src/interpreter.hpp
	g++ -std=c++11 -o $@ -c $<
dist/disassembler.hpp: src/a.out.hpp
dist/disassembler.o: src/disassembler.cpp src/disassembler.hpp src/inst.hpp src/consts.hpp src/util.hpp
	g++ -std=c++11 -o $@ -c $<
dist/interpreter.o: src/interpreter.cpp src/interpreter.hpp src/inst.hpp src/util.hpp src/consts.hpp src/message.hpp
	g++ -std=c++11 -o $@ -c $<
dist/interpreter.hpp: src/a.out.hpp src/reg.hpp src/flags.hpp
dist/machine.o: src/machine.cpp src/machine.hpp
	g++ -std=c++11 -o $@ -c $<
dist/machine.hpp: src/a.out.hpp src/reg.hpp src/flags.hpp
src/inst.hpp: src/consts.hpp
dist/inst.o: src/inst.cpp src/inst.hpp src/util.hpp
	g++ -std=c++11 -o $@ -c $<
dist/util.o: src/util.cpp src/util.hpp
	g++ -std=c++11 -o $@ -c $<
src/reg.hpp: src/util.hpp