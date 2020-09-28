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

# ALTERNATIVELY: Perform a clean-build of the project (Debug build)
make CFLAGS='-g -Og' clean all

# Run the demo, if you'd like
./surdemo

# Install SurRender?
# TODO: Explain this step more!
make install
```

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