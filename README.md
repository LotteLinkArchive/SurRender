# SurRender ![Licensed under GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-orange) ![Written in C](https://img.shields.io/badge/language-C-lightgrey)

<img src="https://git.lotte.link/naphtha/SurRender/raw/branch/master/images/logo.png" align="right"
     title="SurRender Logo" width="240">

SurRender is an attempt to create an abstract rendering engine for rendering 2D and 3D scenes in C.

As SurRender itself does not do any window creation and intends to be as open-ended as possible, it could potentially be used for the following ideas:
* A browser engine
* A 2D/3D game engine
* Scientific purposes?
* A tool for creating user interfaces
* A desktop window manager

## Project Completion

Many features are currently unfinished. We keep track of all of these unfinished features in the [issues tab](https://git.lotte.link/naphtha/SurRender/issues) if you're interested in helping us finish them.

Right now, SurRender is not very useful. 2D functions are partially completed, but 2D scenes and 3D scenes are completely missing. 2D ought to be fairly easy to implement, but 3D will be immensely difficult, and will require lots of help. We plan to allow for the use of multiple 3D rendering techniques, including both software rendering and OpenGL.

## Environment

If you'd like to contribute (or just use SurRender at all), you will need to set up the build environment. To do this...

```sh
# Clone the project AND THE SUBMODULES
git clone --recurse-submodules -j8 https://git.lotte.link/naphtha/SurRender.git
cd SurRender*

# Set up the build environment
# Re-run this step if you suddenly start having issues with autotools after a pull
git submodule update --init --recursive
aclocal
autoconf
libtoolize
automake --add-missing

# Update the project and its submodules (This is a VERY GOOD IDEA)
git pull --recurse-submodules

# IF, AND ONLY IF, YOU GET A WARNING ABOUT DIVERGENT BRANCHES, RUN THIS
git config pull.rebase false

# Run the configure script in order to get an appropriate Makefile
./configure

# Perform a clean-build of the project (Fastest output build)
# For best results, use the latest version of GCC available.
make CFLAGS='-Ofast -march=native -mtune=native' clean all

# ALTERNATIVELY: Perform a clean-build of the project (Small, compatible and fast build)
make CFLAGS='-Os -march=core2 -mtune=generic' clean all

# ALTERNATIVELY: Perform a clean-build of the project (Extremely fast, intended for Zen2+, Uses Clang)
make CFLAGS='-Ofast -march=znver2 -mtune=znver2 -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -msse4.2 -mavx -mavx2' CC='clang' clean all

# ALTERNATIVELY: Perform a clean-build of the project (Debug build)
make CFLAGS='-g -Og' clean all

# Run the demo, if you'd like
./surdemo

# Install SurRender?
# TODO: Explain this step more!
make install
```

**Note**: It is recommended to use [Clang](https://clang.llvm.org/) rather  than [GCC](https://gcc.gnu.org/), primarly because it seems like Clang is far better at optimizing vector-related code (of which SurRender uses quite heavily) than GCC is. You will see bigger benefits depending on how many vector instructions your processor supports. Although SurRender has not been tested on an ARM processor, ARM NEON looks like it could offer some extremely large performance benefits for SurRender.

**Note**: In order to use Clang, simply install it and feed `CC='clang'` into `./configure` or `make` as an argument, as demonstrated above.

**Note**: If you're wondering why Clang is better for vector operations, take a look at an assembly output example [like this](https://godbolt.org/z/4f6eP6). It just seems like Clang is better at handling non-intrinsic (cross-platform) vector operations in many cases. GCC seems to produce massive amounts of wasted cycles seemingly by accident. GCC especially seems to struggle when performing an arithmetic operation between a vector and a scalar value (Even a literal scalar value). Clang usually handles this with ease, however. They both struggle horribly with examples [like this one](https://godbolt.org/z/We65fn), to the point where neither of them produces particularly efficient code. Things get a lot better if you use vector sizes that are natively supported. If you don't, both compilers still do a pretty good job, but can have accidental breakdowns in certain edge cases.

**Note:** See [this Godbolt demonstration](https://godbolt.org/z/ehbYo3) to see the difference between GCC's implementation of the `SR_MergeCanvasIntoCanvas` function and Clang's implementation of it. You will find the results quite eye-opening! Clang's generated code is faster in almost every section except for the final call to `memcpy`, which probably could've been inlined.

### Using SurRender as a Git submodule

You can add SurRender to your Git repository as a submodule like so...
```
git submodule add https://git.lotte.link/naphtha/SurRender <destination_folder>
```
*Note: When you add the SurRender submodule, keep in mind that SurRender has its own submodules, which you will need to recursively update/pull/initialize too!*

This will effectively clone SurRender into `<destination_folder>` without breaking anything. Once you've done this, you will need to rewrite the build script of your application to depend on SurRender/automatically initialize and compile it.

Alternatively, you can install `libsurrender.so` to your system via `make install` and link your program against it. Then, you only need to use the header files in the submodule for your application.

The following guides may come in handy:
* https://devconnected.com/how-to-add-and-update-git-submodules/
* https://www.gnu.org/software/libtool/manual/html_node/Using-Automake.html
* https://autotools.io/index.html
* https://opensource.com/article/19/7/introduction-gnu-autotools
* https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install
* https://www.gnu.org/software/automake/manual/html_node/Libtool-Libraries.html#Libtool-Libraries
* https://www.gnu.org/software/automake/manual/html_node/A-Library.html
* https://autotools.io/whosafraid.html

## Current Dependencies

In order to use SurRender, you'll need a few dependencies.

### Test code

* SDL2 (You will need to install the \*-dev/\*-devel packages for it, as you will need both the library itself and its headers.)

### SurRender itself

* `stb_image.h` from [nothings/stb](https://github.com/nothings/stb) **(bundled with SurRender as a Git submodule)**
* [naphtha/HolyH](https://git.lotte.link/naphtha/HolyH) **(bundled with SurRender as a Git submodule)**

## Contributor Advice

When contributing to the project, keep these ideas in mind:
* Always think of ways you could improve the performance of your code.
* Try to make it as readable as possible, but not in such a way that it becomes ugly.
* Your code should be easy to use, but not in such a way that performance is crippled.
* Remember that this project is intended to be used by actual humans, for actual projects. Make sure your contributions are generally secure, easy to understand, fast and have a generally positive effect on the project.

Here's a [generally useful document](https://www.kernel.org/doc/html/v4.10/process/coding-style.html)  outlining how your code should be formatted, although we don't follow all of the points outlined there.

If your code doesn't quite follow these guidelines but you'd still like to make a pull request, that's fine! We'll take a look at what you've written and help you fix parts of it for you if needed.

We also don't **only** need help programming - we also need people to test for bugs, assess what could be made easier to use, and so on.

Also, when you submit any code changes, please make sure you are conforming to the types defined by [naphtha/HolyH](https://git.lotte.link/naphtha/HolyH), e.g use CHR instead of char, U8 instead of uint8_t, etc.

## Bugs, feature requests and general issues

Please post any bugs and feature requests under the [issues tab](https://git.lotte.link/naphtha/SurRender/issues). We'll help you when we can, and investigate as many bugs as possible. We're also always looking for feature suggestions and ideas on how things could be changed.

## Documentation

The project currently lacks official documentation, but we may add some information and guidance to the Gitea Wiki section. For now, read the comments in the header files. They are VERY verbose, and there are a lot of them. It should be fairly easy to understand the majority of the library's interface(s).

**Note:** If you're looking for a place to start, I would reccomend [canvas.h](https://git.lotte.link/naphtha/SurRender/src/branch/master/src/canvas.h). You should also take a look at some of the demo code [here](https://git.lotte.link/naphtha/SurRender/src/branch/master/demo.c).

## Performance

At the time of writing, SurRender performed at the following speeds on the following hardware...

| Processor | Memory | Demo | Framerate | Percentage of 60 Hz V. Sync | Compiler |
| - | - | - | - | - | - |
| [AMD Ryzen 5 3600 6-Core Processor](http://www.cpu-world.com/CPUs/Zen/AMD-Ryzen%205%203600.html) @ 3.6 GHz | *2x* [CMK32GX4M2D3000C16](https://www.corsair.com/us/en/Categories/Products/Memory/VENGEANCE%C2%AE-LPX-32GB-%282-x-16GB%29-DDR4-DRAM-3000MHz-C16-Memory-Kit---Black/p/CMK32GX4M2D3000C16) @ 3000 MT/s | [Atlas](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 6225 fps | 10375% | Clang |
| *Ditto* | *Ditto* | [Puck](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 410 fps | 683% | Clang |
| *Ditto* | *Ditto* | [Doki](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 3340 fps | 5566% | Clang |