# cache-simulator
Two level cache simulator

# Exclusive Policy

***The following is some clarification of Exclusive policy. The program is based on these rules.***
- Read
  - (RH, NA) Read Hit in L1: directly return the data and no need to access L2.
  - (RM, RH) Read Miss in L1 and Read Hit in L2: move the data in L2 to L1. If there's data in L1
    needed to be evicted, a write access in L2 by the evicted data defined location is triggered.
  - (RM, RM) Read Miss in L1 and Read Miss in L2: retrieve the data from memory and place it only in  L1.
- Write
  - (WH, NA) Write Hit in L1: Nothing.
  - (WM, WH) Write Miss in L1 and Write Hit in L2: L1 forward the write to L2.
  - (WM, WM) Write Miss in L1 and Write Miss in L2: L1 forward the write to L2 and L2 forward the write to memory.

# How to use

A `makefile' covering compiling, building and execution has been provided. Use following command line to execute: (make sure you are locating in the source file cachesimulator.cpp folder)
```bash
make
```

It has the same effect as the following command line:
```bash
g++ cachesimulator.cpp -o cachesimulator.out
./cachesimulator.out cacheconfig.txt trace.txt
```

After the execution, the result file named `output.txt` will be generated in the current directory.

- cacheconfig.txt is the file defines the parameters of cache, like cache size, associativity and block size.
- trace.txt provides read/write addresses.
