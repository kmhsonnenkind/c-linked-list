# C Generic Thread-Safe Linked List Implementation

[![GitHub](https://img.shields.io/github/license/kmhsonnenkind/c-linked-list)](https://github.com/kmhsonnenkind/c-linked-list/blob/main/LICENSE)
[![Build](https://github.com/kmhsonnenkind/c-linked-list/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/kmhsonnenkind/c-linked-list/actions/workflows/build.yml)
[![Codecov](https://codecov.io/gh/kmhsonnenkind/c-linked-list/branch/main/graph/badge.svg?token=5LNZWPDGI2)](https://codecov.io/gh/kmhsonnenkind/c-linked-list)

## About

This project offers a generic thread-safe linked list implementation in `C99`.


## Examples

```c
#include "linkedlist.h"

// Initialize lists for use with integers
LinkedList list;
linkedlist_initialize(&list, sizeof(int), NULL, NULL);

// Add some items
for (int i=0; i < 10; i++)
{
    linkedlist_add(&list, &i);
}

// Query item
int value = 0;
linkedlist_get(&list, 5, &value);

// Update value
value = 42;
linkedlist_update(&list, 5, &value);

// Remove item
linkedlist_remove(&list, 5);

// Cleanup
linkedlist_destroy(&list);
```


## Build and Installation

The build uses [cmake](https://cmake.org/) and can be easily integrated into existing projects.

### Dependencies

As mentioned the build uses `cmake` so you will need the `cmake` binaries as well as a matching `C` and `C++` compiler.

The code uses `C99` features so make sure your compiler supports it (all the big ones do).
For the unittests you will also need a compiler supporting `C++11`.
The code was tested using [gcc-9](https://gcc.gnu.org/), [clang-10](https://clang.llvm.org/) and [msvc-19](https://visualstudio.microsoft.com/de/vs/features/cplusplus/).

If you want to build the full API documentation you will additionally need [doxygen]().

For additional test features and static code analysis you might also want to install [lcov](http://ltp.sourceforge.net/coverage/lcov.php), [cppcheck](http://cppcheck.sourceforge.net/) and [iwyu](https://include-what-you-use.org/).

On `Debian` based systems you can install the required packages using:

```sh
sudo apt-get update

# Required dependencies
sudo apt-get install build-essential \
                     cmake \
                     gcc \
                     g++

# Optional dependencies
sudo apt-get install doxygen \
                     graphviz \
                     lcov \
                     cppcheck \
                     iwyu
```

All tools should also provide installers for your targeted operating system so just follow the instructions on the tools' websites.

#### Supported Operating Systems

The code tries to be as platform-independent as possible but needs to rely on native mutex objects for thread-safety.
To do so during configuration your system will be checked and the corresponding implementation chosen accordingly.

Currently the following implementations are available:

* [pthread_mutex_t](https://man7.org/linux/man-pages/man7/pthreads.7.html) for systems supporting POSIX mutexes (Linux, BSD, MacOS).
* [win32 CreateMutex()](https://docs.microsoft.com/en-us/windows/win32/sync/using-mutex-objects) for Windows system.

If your operating system supports neither an error message will be displayed and you will not be able build the library.

### Build

The build can be triggered like any other `cmake` project.

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

It offers several configuration parameters described in the following sections. For your convenience a [cmake variants](https://vector-of-bool.github.io/docs/vscode-cmake-tools/variants.html) file is provided that lets you choose the desired target configuration directly in [Visual Studio Code](https://code.visualstudio.com/).

### Installation

To simply install the code, you can use cmake's `install` target.

```
# In build directory, with code built as described above
sudo cmake --build . --target install
```

This will install:
* The `linkedlist` library to your system's library directory (somewhere like */usr/local/lib*).
* The required headers to your system's include directory (somewhere like */usr/local/include*).
* The cmake files for stuff like `find_package()` to cmake's include directory (somewhere like */usr/local/lib/linkedlist*)

For other ways to integrate the code in your project see the corresponding [section](#use-in-own-projects) below.

#### Special Case: Windows

Windows does not have a standard location for user libraries and will therefore not know where to put the code. You can override the install location manually using the `CMAKE_INSTALL_PREFIX` parameter.

```sh
# In build directory, with code built as described above
cmake -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake" ..
cmake --build . --target install
```

This will put all files under the given directory (*%APPDATA%/cmake/include*, *%APPDATA%/cmake/lib*) and you will need to make sure your compiler / toolchain picks up the data from there.

### Tests and Checks

The code is unittested using [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) and [catch2](https://github.com/catchorg/Catch2) (provided in [tests/catch2](tests/catch2/catch.hpp)). As with any `ctest` project, the option to build the unittests can be enabled using the `BUILD_TESTING` cmake parameter (enabled by default).

```sh
# In build directory, with cmake configured as described above
cmake -DBUILD_TESTING=ON ..
cmake --build .
ctest .
```

#### Memory Checks

If you have a memory checker tools supported by `ctest` installed, you can run checks on the provided unittests using the `ctest memcheck` target. The code was tested using [valgrind](https://valgrind.org/) and should not produce any warnings or errors.

```
ctest -T memcheck .
```

#### Coverage

If you want to get detailed information about the code coverage, you can turn on the cmake option `CODE_COVERAGE` (this requires `BUILD_TESTING` to be enabled as well). If enabled a special library (`linkedlist-coverage`) will be built from the same source code as the main library. This version has the appropriate compiler flags set to enable coverage measurements. The unittests will then use this version and provide coverage information after execution.
To avoid issues during later use, the main library will be built as usual as well.

**Note:** Coverage measurements are only available with `gcc` or `clang`. If using another compiler the `linkedlist-coverage` library will simply be skipped.

To analyze the coverage data you can use [lcov](http://ltp.sourceforge.net/coverage/lcov.php) after running the unittests.

```sh
# In build directory, with unittests configured as described above
cmake -DCODE_COVERAGE=ON ..
cmake --build .
ctest .

# Get code coverage
lcov --capture --directory . --output-file code.coverage
lcov --remove code.coverage '/usr/*' --output-file code.coverage
lcov --remove code.coverage '**/tests/*' --output-file code.coverage
lcov --remove code.coverage '**/catch.hpp' --output-file code.coverage
lcov --list code.coverage
```

This will print an overview of the code coverage to console. See the `lcov` documentation for information on further report types.

The design decision was taken to enforce code coverage of 100%. Code that cannot be covered or tested without immense workarounds (such as failing memory allocations) is explicitly marked as uncoverable using `LCOV_EXCL_*` markers.

#### Code Analysis

During development several static code analyzers were used to ensure high code quality and avoid potential issues:

* [cppcheck](http://cppcheck.sourceforge.net/) is used to perform static code analysis and detect potential issues.
* [iwyu](https://include-what-you-use.org/) is used to check that only required header files are imported.
* [lwyu](https://cmake.org/cmake/help/latest/prop_tgt/LINK_WHAT_YOU_USE.html) is used to ensure only required libraries are linked.

The tools can be integrated in `cmake` and if enabled will be executed during every build. Note that `iwyu` is expected to be used with `clang` so make sure to set the compiler accordingly.

```sh
# In build directory, with cmake configured as described above
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
cmake -DCMAKE_C_CPPCHECK=cppcheck ..
cmake -DCMAKE_C_INCLUDE_WHAT_YOU_USE=iwyu ..
cmake -DCMAKE_LINK_WHAT_YOU_USE=ON ..
cmake --build .
```

All checks should pass and not produce any warnings or errors.

### Documentation

For further information on the API you can build additional documentation yourself. You can enable the automatic build of the documentation using the `BUILD_DOCUMENTATION` cmake parameter. This will require you to have [doxygen](https://www.doxygen.nl/) installed.

```sh
# In build directory, with cmake configured as described above
cmake -DBUILD_DOCUMENTATION=ON ..
cmake --build .
```

This will put the built documentation in your cmake build directory under *docs*.


## Use in Own Projects

The code is designed to be easily integrable into existing projects. It does not have any external dependencies and should be straight-forward to use.

### Use in CMake

To use the code in `cmake` you can either copy it as a submodule or use the installed version described [above](#installation). Once available a custom cmake target `kmhsonnenkind::linkedlist` can be consumed and linked like any other.

To use the code as a submodule, simply clone it somewhere to your source tree (e.g. in an *external* directory) and call `add_subdirectory()`.

```
cmake_minimum_required(VERSION 3.12)
project(example)

add_subdirectory(external/linkedlist)
```

Otherwise you can use the installed version using `find_package()`.

```
cmake_minimum_required(VERSION 3.12)
project(example)

find_project(linkedlist REQUIRED)
```

Special care has to be taken in Windows to patch the `CMAKE_PREFIX_PATH` to match the directory mentioned [above](#special-case-windows).

```sh
cmake -DCMAKE_PREFIX_PATH="%APPDATA%/cmake/lib" .
```

Either way you can then link the `linkedlist` library using the `target_link_libraries()` command.

```
cmake_minimum_required(VERSION 3.12)
project(example)

find_project(linkedlist REQUIRED)

add_executable(example src/example.c)
target_link_libraries(example PRIVATE linkedlist)
```

### Use Outside of CMake

If you cannot use cmake for any reason, you can still use the [installed version](#installation) or the [built code](#build).

To do so make sure that:
* Your compiler can find the required header files (should be the case after installation).
* Your linker can find the required library (should also be the case after installation).
* Your code correctly links the library.

As an example you can do this with `gcc` using:

```sh
gcc -Wall -o example example.c -llinkedlist
```

If you want to use the library without installation, make sure to set your compiler's include and linker paths accordingly.
The include path should contain this project's *include* directory and the linker path the cmake build directory.

```
gcc -Wall -o example example.c -Iexternal/linkedlist/include -Lextenal/linkedlist/build -llinkedlist
```
