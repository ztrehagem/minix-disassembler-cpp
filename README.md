# Disassembler and Interpreter of binaries for Minix2

## Build
```
$ make
```

## Execute
```
$ ./disassembler bin/1.s.out
$ ./disassembler bin/2.s.out
$ ./disassembler bin/3.s.out
$ ./disassembler bin/4.s.out

$ ./disassembler bin/1.c.out
$ ./disassembler bin/2.c.out
 :
$ ./disassembler bin/7.s.out
```

```
$ ./interpreter bin/1.s.out
$ ./interpreter bin/2.s.out
$ ./interpreter bin/3.s.out
$ ./interpreter bin/4.s.out

$ ./interpreter bin/1.c.out
$ ./interpreter bin/2.c.out
 :
$ ./interpreter bin/5.c.out arg1 arg2 ...
 :
$ ./interpreter bin/7.s.out
```

## Clean
```
$ make clean
```

## Screenshots
- See the `screenshorts` directory