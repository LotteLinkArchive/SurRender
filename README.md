# SurRender ![Licensed under LGPL-3.0](https://img.shields.io/badge/license-LGPL--3.0-orange) ![Written in C](https://img.shields.io/badge/language-C-lightgrey)

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

## Compiling & Environment

**Warning:** Before compiling, ensure you have everything listed in the "Dependencies" section of this document.

If you'd like to contribute (or just use SurRender at all), you will need to set up the build environment. To do this...

```sh
# Clone the project AND THE SUBMODULES
git clone --recurse-submodules -j8 https://git.lotte.link/naphtha/SurRender.git
cd SurRender*

# Set up the build environment
git submodule update --init --recursive

# Update the project and its submodules (This is a VERY GOOD IDEA - Do this before building every time)
git pull --recurse-submodules

# Create a dev/debug build of the project - Requires Clang
./devbuild.sh

# Run the demo, if you'd like
./bin/tools/surdemo
```

## Current Dependencies

In order to use SurRender, you'll need a few dependencies.

### Test code

* SDL2 (You will need to install the \*-dev/\*-devel packages for it, as you will need both the library itself and its headers.)

### SurRender itself

* `stb_image.h` from [nothings/stb](https://github.com/nothings/stb) **(bundled with SurRender as a Git submodule)**
* [simde-everywhere/simde](https://github.com/simd-everywhere/simde) **(bundled with SurRender as a Git submodule)**
* [naphtha/HolyH](https://git.lotte.link/naphtha/HolyH) **(bundled with SurRender as a Git submodule)**

### Building

* clang
* git
* meson
* ninja

**Warning**: This has only been tested on Clang 10.0.0+ and GCC 10.2.1+. Be careful what compiler version you use - many old versions lack the necessary vector extensions (and are generally very buggy). For example, if you're using a GNU/Linux distro that isn't particularly good (e.g Ubuntu), you are very likely going to be stuck with stone-age versions of Clang and GCC. At the time of writing, Ubuntu 18.04 provides only Clang version 3.8.1-24 in its respositories, which is incredibly old, and will not compile SurRender. Please use a GNU/Linux distro that isn't terrible, namely one that provides actual up-to-date packages. Try Artix, OpenSUSE, Fedora, Gentoo and anything else that isn't completely terrible.

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
| [AMD Ryzen 5 3600 6-Core Processor](http://www.cpu-world.com/CPUs/Zen/AMD-Ryzen%205%203600.html) @ 3.6 GHz | *2x* [CMK32GX4M2D3000C16](https://www.corsair.com/us/en/Categories/Products/Memory/VENGEANCE%C2%AE-LPX-32GB-%282-x-16GB%29-DDR4-DRAM-3000MHz-C16-Memory-Kit---Black/p/CMK32GX4M2D3000C16) @ 3000 MT/s | [Atlas](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 6225 fps | 10375% | Clang 10.0.0 |
| *Ditto* | *Ditto* | [Puck](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 410 fps | 683% | *Ditto* |
| *Ditto* | *Ditto* | [Doki](https://git.lotte.link/naphtha/SurRender/src/commit/630ae6c72bb77aff29d450f003776f4f1449b697/demos.h) | 3340 fps | 5566% | *Ditto* |