# What is `idax`?

`idax` is a set of C++ extensions for the `IDASDK`. These extensions are a work in progress and are not meant to be used in production code yet. As of now, only my personal IDA [plugins](https://github.com/0xeb/ida-strikeout) use `idax`.

# Installation

To use this library in existing IDA plugin projects, just copy (or clone) the `idax` folder into `<idasdk>/include/`.

Normally, you would import `IDASDK` headers like this:
```c++
#include <kernwin.hpp>
```

Now, to use `idax`, simply do:

```c++
#include <idax/xkernwin.hpp>
```

Note that `idax` requires the C++17 standard. If you are using CMake/[`ida-cmake`](https://github.com/0xeb/ida-cmake), you can set the C++ standard like this:

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```


## Symbolic links

In the case of multiple IDA SDKs on the system, it is best to clone this project into its own folder then create symbolic links.

For instance, on MS Windows:

```batch
D:\Projects\ida\idasdk76\include>mklink /j %cd%\idax D:\Projects\opensource\idax
```

# Extensions summary

## xpro.hpp

- Low level / support helpers

## xkernwin.hpp

- Action manager: simplifies action creation and management


## xhexrays.hpp

- Various helpers for Hexrays. [ida-strikeout](../ida-strikeout) makes use of this header a lot