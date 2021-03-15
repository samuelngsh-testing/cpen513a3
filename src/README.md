> **Disclaimer:** this assignment code and documentations reuse assignment 2's code when appropriate. That said, since this is a multi-threaded implementation whereas assignment 2 was not, much effort was still put into making multi-threaded workers to GUI communication compatible.

# Running the Routing Program

## macOS

A bundled binary is included with the assignment submission with the name `partitioner.app`. It should be supported by macOS 14 and above (tested on macOS 14 and 15).

If compiling from source is desired, CMake and Qt5 are required. The simplest way to acquire `qt` seems to be from the [Homebrew package manager](https://brew.sh). Once you have brew installed, type:

```
brew install cmake qt
```

With the prerequisites installed, the rest of the steps are identical to the Ubuntu compilation steps starting from the `git clone` line which you can find below.

## Ubuntu

### Compile from Source

Note: these instructions should also work on a WSL installation of Ubuntu 18.04 LTS or 20.04 LTS on Windows 10, assuming that you have a viable X11 server such as Xming.

Install prerequisites:
```
sudo apt install make gcc g++ qtchooser qt5-default libqt5svg5-dev qttools5-dev qttools5-dev-tools libqt5charts5-dev
```

Acquire the source code by:

```
git clone https://github.com/samuelngsh-testing/cpen513a3
```

From the project root, the `src` directory contains the source code (where this README is also located). To compile the project, run the following from the project root:

```
mkdir src/build && cd src/build
cmake .. && make
```

By default, unit tests are performed during the compilation with the results available via standard output. You should now be in the `src/build` directory relative to the project root. From there, invoke the GUI binary by:

```
./partitioner
```

Command line options are also available, which you can view by:

```
./partitioner --help
```

# Running in Headless Mode from the Terminal

Run `./partitioner --help` for exact invocation syntaxes.
