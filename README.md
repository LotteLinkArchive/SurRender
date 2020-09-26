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

If you'd like to contribute, you will need to set up the build environment. To do this...

```
# Clone the project AND THE SUBMODULES
git clone --recurse-submodules -j8 https://git.lotte.link/naphtha/SurRender.git
cd SurRender*

# Update and build the project
git pull --recurse-submodules
make clean; make -j8

# Run the demo
./demo/a.out
```

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