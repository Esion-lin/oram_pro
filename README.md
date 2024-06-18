# DT + ORAM + RISC
author:TYAN

## 1) Compiling 

First make sure the following options in `CMakeLists.txt' is turned on.

| **Options** ||
| :--- | :---|
| ENABLE_FFS | FSS and ORAM|
| ENABLE_GC | dependence|
| TEST_FOREST_DT | decision tree|
| TEST_RISC | private function evaluation|

Then run

 ```
mkdir build
cd build
cmake ..
 ```
When `cmake` reports error, modify the cmakefile that reported the error:
- `aes_128.txt.cpp` -> `aes_128.txt`
- delete the last two lines

Later,

```
cmake ..
make -j12
```
If `make` reports error, delete all `LOG` that reported the error.

## 2) Running the benchmarks

Move config and instruction files to the build folder.

| **Simple TEST** ||
| :--- | :---|
| DT benchmarks | ``` ./test_dt -p player0 -c false/true -a 3 -n 5 -d all ```|
| ORAM benchmarks | ```./ORAM_TEST -p player0```,```./ORAM_TEST -p aid```|
| PFE | ```./test_risc -p player0```,```./ORAM_TEST -p aid```|

There is `-h` command for more parameter settings.


### PFE

There are three instruction files:

| **instruction** ||
| :--- | :---|
| quick sort | ``` quick_sort.ins```|
| set intersection | ```set_inte.ins```|
| binary sort | ```test.ins```|

Adjust input size by changing `ARR_LEN` in file `protocol/risc.h`.

## TODO
* preliminaries
- +o_read+
- +o_write+
- +overflow+
- +share conversion+
* operator
- +cmpe/cmpa/cmpae []+
- +add/sub []+
- mov
- memery
- jump


