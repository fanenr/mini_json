# mini_json is a header-only json library
1. Dependents
```
1. CMake 3.15 or later
2. vcpkg
3. make
4. valgrind (for memory check)
```
1. Build
``` shell
mkdir build && cd build
cmake ..
```
1. Run test
``` shell
# cd build

# run test
make run_t

# run benchmark
make run_b

# run memory check
make run_m
```