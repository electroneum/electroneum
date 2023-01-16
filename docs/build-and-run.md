## Compiling Electroneum from Source

### Dependencies

The following table summarizes the tools and libraries required to build. A few of the libraries are also included in this repository (marked as "Vendored"). By default, the build uses the library installed on the system, and ignores the vendored sources. However, if no library is found installed on the system, then the vendored source will be built and used. The vendored sources are also used for tatically-linked builds because distribution packages often include only shared library binaries (`.so`) but not static library archives (`.a`).

| Dep          | Min. version  | Vendored | Debian/Ubuntu pkg | Arch pkg     | Fedora         | Optional | Purpose                  |
| ------------ |---------------| -------- | -------------- | ------------ | -------------- | ----- |--------------------------|
| GCC          | 4.7.3         | NO       | `build-essential` | `base-devel` | `gcc`          | NO    | C/C++ compiler           |
| CMake        | 3.5           | NO       | `cmake`        | `cmake`      | `cmake`        | NO    | Build system             |
| pkg-config   | any           | NO       | `pkg-config`   | `base-devel` | `pkgconf`      | NO    | Compiler options         |
| Boost        | 1.58          | NO       | `libboost-all-dev` | `boost`      | `boost-devel`  | NO    | C++ libraries            |
| OpenSSL      | basically any | NO       | `libssl-dev`   | `openssl`    | `openssl-devel` | NO    | sha256 sum               |
| libzmq       | 3.0.0         | NO       | `libzmq3-dev`  | `zeromq`     | `cppzmq-devel` | NO    | ZeroMQ library           |
| libsecp256k1 | ?             | NO       | `libsecp256k1-dev` | `libsecp256k1` | `libsecp256k1` | NO    | ETN Smartchain Migration |
| OpenPGM      | ?             | NO       | `libpgm-dev`   | `libpgm`     | `openpgm-devel` | NO    | For ZeroMQ               |
| libnorm[2]   | ?             | NO       | `libnorm-dev`  |              |               ` | YES   | For ZeroMQ               |
| libunbound   | 1.4.16        | YES      | `libunbound-dev` | `unbound`    | `unbound-devel` | NO    | DNS resolver             |
| libsodium    | ?             | NO       | `libsodium-dev` | `libsodium`  | `libsodium-devel` | NO    | cryptography             |
| libunwind    | any           | NO       | `libunwind8-dev` | `libunwind`  | `libunwind-devel` | YES   | Stack traces             |
| liblzma      | any           | NO       | `liblzma-dev`  | `xz`         | `xz-devel`     | YES   | For libunwind            |
| libreadline  | 6.3.0         | NO       | `libreadline6-dev` | `readline`   | `readline-devel` | YES   | Input editing            |
| ldns         | 1.6.17        | NO       | `libldns-dev`  | `ldns`       | `ldns-devel`   | YES   | SSL toolkit              |
| expat        | 1.1           | NO       | `libexpat1-dev` | `expat`      | `expat-devel`  | YES   | XML parsing              |
| GTest        | 1.5           | YES      | `libgtest-dev`[1] | `gtest`      | `gtest-devel`  | YES   | Test suite               |
| Doxygen      | any           | NO       | `doxygen`      | `doxygen`    | `doxygen`      | YES   | Documentation            |
| Graphviz     | any           | NO       | `graphviz`     | `graphviz`   | `graphviz`     | YES   | Documentation            |
| lrelease     | ?             | NO       | `qttools5-dev-tools` | `qt5-tools`  | `qt5-linguist`   | YES   | Translations             |
| libhidapi    | ?             | NO       | `libhidapi-dev`  | `hidapi`     | `hidapi-devel`   | YES   | Hardware wallet          |
| libusb       | ?             | NO       | `libusb-dev`     | `libusb`     | `libusb-devel`   | YES   | Hardware wallet          |
| libprotobuf  | ?             | NO       | `libprotobuf-dev` | `protobuf`   | `protobuf-devel` | YES   | Hardware wallet          |
| protoc       | ?             | NO       | `protobuf-compiler` | `protobuf`   | `protobuf-compiler` | YES   | Hardware wallet          |


[1] On Debian/Ubuntu `libgtest-dev` only includes sources and headers. You must
build the library binary manually. This can be done with the following command ```sudo apt-get install libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libg* /usr/lib/ ```
[2] libnorm-dev is needed if your zmq library was built with libnorm, and not needed otherwise

Install all dependencies at once on Debian/Ubuntu:

``` sudo apt update && sudo apt install build-essential cmake pkg-config libboost-all-dev libssl-dev libzmq3-dev libsecp256k1-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libpgm-dev libusb-dev```

Install all dependencies at once on macOS with the provided Brewfile:
``` brew update && brew bundle --file=contrib/brew/Brewfile ```

FreeBSD one liner for required to build dependencies
```pkg install git gmake cmake pkgconf boost-libs cppzmq libsodium libsecp256k1```

### Cloning the repository

Clone recursively to pull-in needed submodule(s):

`$ git clone --recursive https://github.com/electroneum/electroneum`

If you already have a repo cloned, initialize and update:

`$ cd electroneum && git submodule init && git submodule update`

On OSX:
Homebrew installs OpenSSL but doesn’t link it to /usr/local/include, where the compiler looks into during #include<…> Thus, you must manually link it instead:

```ln -s /usr/local/opt/openssl/include/openssl /usr/local/include```
```ln -s /usr/local/Cellar/openssl/[version]/include/openssl /usr/bin/openssl```
```ln -s /usr/local/opt/openssl/lib/libssl.1.0.0.dylib /usr/local/lib/```

### Build instructions

Electroneum uses the CMake build system and a top-level [Makefile](/Makefile) that
invokes cmake commands as needed.

#### On Linux and macOS

* Install the dependencies
* Change to the root of the source code directory, change to the most recent release branch, and build:

    ```bash
    cd electroneum
    make
    ```

    *Optional*: If your machine has several cores and enough memory, enable
    parallel build by running `make -j<number of threads>` instead of `make`. For
    this to be worthwhile, the machine should have one core and about 2GB of RAM
    available per thread.

    *Note*: If cmake can not find zmq.hpp file on macOS, installing `zmq.hpp` from
    https://github.com/zeromq/cppzmq to `/usr/local/include` should fix that error.

    *Note*: The instructions above will compile the most stable release of the
    Monero software. If you would like to use and test the most recent software,
    use ```git checkout master```. The master branch may contain updates that are
    both unstable and incompatible with release software, though testing is always
    encouraged.

* The resulting executables can be found in `build/release/bin`

* Add `PATH="$PATH:$HOME/electroneum/build/release/bin"` to `.profile`

* Run electroneum with `electroneumd --detach`

* **Optional**: build and run the test suite to verify the binaries:

    ```bash
    make release-test
    ```

    *NOTE*: `core_tests` test may take a few hours to complete.

* **Optional**: to build binaries suitable for debugging:

    ```bash
    make debug
    ```

* **Optional**: to build statically-linked binaries:

    ```bash
    make release-static
    ```

Dependencies need to be built with -fPIC. Static libraries usually aren't, so you may have to build them yourself with -fPIC. Refer to their documentation for how to build them.

* **Optional**: build documentation in `doc/html` (omit `HAVE_DOT=YES` if `graphviz` is not installed):

    ```bash
    HAVE_DOT=YES doxygen Doxyfile
    ```

#### On Windows:

Binaries for Windows are built on Windows using the MinGW toolchain within
[MSYS2 environment](https://www.msys2.org). The MSYS2 environment emulates a
POSIX system. The toolchain runs within the environment and *cross-compiles*
binaries that can run outside of the environment as a regular Windows
application.

**Preparing the build environment**

* Download and install the [MSYS2 installer](https://www.msys2.org), either the 64-bit or the 32-bit package, depending on your system.
* Open the MSYS shell via the `MSYS2 Shell` shortcut
* Update packages using pacman:  

    ```bash
    pacman -Syu
    ```

* Exit the MSYS shell using Alt+F4  
* Edit the properties for the `MSYS2 Shell` shortcut changing "msys2_shell.bat" to "msys2_shell.cmd -mingw64" for 64-bit builds or "msys2_shell.cmd -mingw32" for 32-bit builds
* Restart MSYS shell via modified shortcut and update packages again using pacman:  

    ```bash
    pacman -Syu
    ```

* Install dependencies:

    To build for 64-bit Windows:

    ```bash
    pacman -S mingw-w64-x86_64-toolchain make mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi
    ```

    To build for 32-bit Windows:

    ```bash
    pacman -S mingw-w64-i686-toolchain make mingw-w64-i686-cmake mingw-w64-i686-boost mingw-w64-i686-openssl mingw-w64-i686-zeromq mingw-w64-i686-libsodium mingw-w64-i686-hidapi
    ```

* Open the MingW shell via `MinGW-w64-Win64 Shell` shortcut on 64-bit Windows
  or `MinGW-w64-Win64 Shell` shortcut on 32-bit Windows. Note that if you are
  running 64-bit Windows, you will have both 64-bit and 32-bit MinGW shells.

**Cloning**

* To git clone, run:

    ```bash
    git clone --recursive https://github.com/electroneum/electroneum.git
    ```

**Building**

* Change to the cloned directory, run:

    ```bash
    cd electroneum
    ```

* If you would like a specific [version/tag](https://github.com/electroneum/electroneum/tags), do a git checkout for that version. eg. 'v3.1.0.0'. If you don't care about the version and just want binaries from master, skip this step:
	
    ```bash
    git checkout v3.1.0.0
    ```

* If you are on a 64-bit system, run:

    ```bash
    make release-static-win64
    ```

* If you are on a 32-bit system, run:

    ```bash
    make release-static-win32
    ```

* The resulting executables can be found in `build/release/bin`

### Building Portable Statically Linked Binaries

By default, in either dynamically or statically linked builds, binaries target the specific host processor on which the build happens and are not portable to other processors. Portable binaries can be built using the following targets:

* ```make release-static-linux-x86_64``` builds binaries on Linux on x86_64 portable across POSIX systems on x86_64 processors
* ```make release-static-linux-i686``` builds binaries on Linux on x86_64 or i686 portable across POSIX systems on i686 processors
* ```make release-static-linux-armv8``` builds binaries on Linux portable across POSIX systems on armv8 processors
* ```make release-static-linux-armv7``` builds binaries on Linux portable across POSIX systems on armv7 processors
* ```make release-static-linux-armv6``` builds binaries on Linux portable across POSIX systems on armv6 processors
* ```make release-static-win64``` builds binaries on 64-bit Windows portable across 64-bit Windows systems
* ```make release-static-win32``` builds binaries on 64-bit or 32-bit Windows portable across 32-bit Windows systems

## Running electroneumd

The build places the binary in `bin/` sub-directory within the build directory
from which cmake was invoked (repository root by default). To run in
foreground:

    ./bin/electroneumd

To list all available options, run `./bin/electroneumd --help`.  Options can be
specified either on the command line or in a configuration file passed by the
`--config-file` argument.  To specify an option in the configuration file, add
a line with the syntax `argumentname=value`, where `argumentname` is the name
of the argument without the leading dashes, for example `log-level=1`.

To run in background:

    ./bin/electroneumd --log-file electroneumd.log --detach

To run as a systemd service, copy
[electroneumd.service](/utils/systemd/electroneumd.service) to `/etc/systemd/system/` and
[electroneumd.conf](/utils/conf/electroneumd.conf) to `/etc/`. The [example
service](/utils/systemd/electroneumd.service) assumes that the user `electroneum` exists
and its home is the data directory specified in the [example
config](/utils/conf/electroneumd.conf).

If you're on Mac, you may need to add the `--max-concurrency 1` option to
electroneum-wallet-cli, and possibly electroneumd, if you get crashes refreshing.