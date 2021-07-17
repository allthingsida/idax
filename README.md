# What is `idax`?

`idax` is a set of C++ extensions for the `IDASDK`. These extensions are a work in progress and are not meant to be used in production code yet. As of now, only my personal IDA [plugins](https://github.com/0xeb/ida-strikeout) use `idax`.

# Install

To use this library in existing IDA plugin projects, just copy the `idax` folder into `<idasdk>/include/`. Normally, you would import `IDASDK` headers like this:
```c++
#include <kernwin.hpp>
```

Now, to use `idax`, simply do:

```c++
#include <idax/xkernwin.hpp>
```

# Extensions summary

## xpro.hpp

- Low level / support helpers

## xkernwin.hpp

- Action manager: simplifies action creation and management


## xhexrays.hpp

- Various helpers for Hexrays. `ida-strikeout` makes use of this header a lot