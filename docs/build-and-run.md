## Compiling Electroneum from Source

### Dependencies

The following table summarizes the tools and libraries required to build. A few of the libraries are also included in this repository (marked as "Vendored"). By default, the build uses the library installed on the system, and ignores the vendored sources. However, if no library is found installed on the system, then the vendored source will be built and used. The vendored sources are also used for tatically-linked builds because distribution packages often include only shared library binaries (`.so`) but not static library archives (`.a`).

| Dep          | Min. version  | Vendored | Debian/Ubuntu pkg    | Arch pkg       | Fedora              | Optional | Purpose                  |
|--------------|---------------| -------- |----------------------|----------------|---------------------| ----- |--------------------------|
| GCC          | 4.7.3         | NO       | `build-essential`    | `base-devel`   | `gcc`               | NO    | C/C++ compiler           |
| CMake        | 3.5           | NO       | `cmake`              | `cmake`        | `cmake`             | NO    | Build system             |
| pkg-config   | any           | NO       | `pkg-config`         | `base-devel`   | `pkgconf`           | NO    | Compiler options         |
| Boost        | **1.85 required** (see note below) | NO | Build from source    | `boost`        | `boost-devel`       | NO    | C++ libraries            |
| GMP          | ?             | NO       | `libgmp-dev`         | `gmp`          | `gmp`               | NO    | Arithmetic Opoerations   |
| OpenSSL      | basically any | NO       | `libssl-dev`         | `openssl`      | `openssl-devel`     | NO    | sha256 sum               |
| libzmq       | 3.0.0         | NO       | `libzmq3-dev`        | `zeromq`       | `cppzmq-devel`      | NO    | ZeroMQ library           |
| libsecp256k1 | ?             | NO       | `libsecp256k1-dev`   | `libsecp256k1` | `libsecp256k1`      | NO    | ETN Smartchain Migration |
| OpenPGM      | ?             | NO       | `libpgm-dev`         | `libpgm`       | `openpgm-devel`     | NO    | For ZeroMQ               |
| libnorm[2]   | ?             | NO       | `libnorm-dev`        |                | `                   | YES   | For ZeroMQ               |
| libunbound   | 1.4.16        | YES      | `libunbound-dev`     | `unbound`      | `unbound-devel`     | NO    | DNS resolver             |
| libsodium    | ?             | NO       | `libsodium-dev`      | `libsodium`    | `libsodium-devel`   | NO    | cryptography             |
| libunwind    | any           | NO       | `libunwind8-dev`     | `libunwind`    | `libunwind-devel`   | YES   | Stack traces             |
| liblzma      | any           | NO       | `liblzma-dev`        | `xz`           | `xz-devel`          | YES   | For libunwind            |
| libreadline  | 6.3.0         | NO       | `libreadline6-dev`   | `readline`     | `readline-devel`    | YES   | Input editing            |
| ldns         | 1.6.17        | NO       | `libldns-dev`        | `ldns`         | `ldns-devel`        | YES   | SSL toolkit              |
| expat        | 1.1           | NO       | `libexpat1-dev`      | `expat`        | `expat-devel`       | YES   | XML parsing              |
| GTest        | 1.5           | YES      | `libgtest-dev`[1]    | `gtest`        | `gtest-devel`       | YES   | Test suite               |
| Doxygen      | any           | NO       | `doxygen`            | `doxygen`      | `doxygen`           | YES   | Documentation            |
| Graphviz     | any           | NO       | `graphviz`           | `graphviz`     | `graphviz`          | YES   | Documentation            |
| lrelease     | ?             | NO       | `qttools5-dev-tools` | `qt5-tools`    | `qt5-linguist`      | YES   | Translations             |
| libhidapi    | ?             | NO       | `libhidapi-dev`      | `hidapi`       | `hidapi-devel`      | YES   | Hardware wallet          |
| libusb       | ?             | NO       | `libusb-dev`         | `libusb`       | `libusb-devel`      | YES   | Hardware wallet          |
| libprotobuf  | ?             | NO       | `libprotobuf-dev`    | `protobuf`     | `protobuf-devel`    | YES   | Hardware wallet          |
| protoc       | ?             | NO       | `protobuf-compiler`  | `protobuf`     | `protobuf-compiler` | YES   | Hardware wallet          |


[1] On Debian/Ubuntu `libgtest-dev` only includes sources and headers. You must
build the library binary manually. This can be done with the following command ```sudo apt-get install libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libg* /usr/lib/ ```
[2] libnorm-dev is needed if your zmq library was built with libnorm, and not needed otherwise

### Boost 1.85 (required on all platforms)

Electroneum must be built against **Boost 1.85** on every platform. Newer Boost
releases (1.86+, including Homebrew's current `boost` formula at 1.90) remove
APIs the project depends on and will fail to compile. Older versions from
distro repos are typically not 1.85 either, so install it explicitly as
described below.

**macOS (Homebrew):** install the tapped formula and force-link it over any
existing `boost` keg:
```
brew install boost@1.85
brew link --force --overwrite boost@1.85
```
If you already have a newer `boost` linked, unlink it first:
`brew uninstall --ignore-dependencies boost` (or `brew unlink boost`).

**Debian/Ubuntu:** remove any distro Boost and build 1.85 from source:
```
sudo apt remove --purge 'libboost*-dev' 'libboost*1.*' libboost-all-dev
sudo apt autoremove --purge
sudo apt install -y build-essential g++ python3-dev libicu-dev libbz2-dev zlib1g-dev liblzma-dev
cd /tmp
wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.bz2
tar xjf boost_1_85_0.tar.bz2 && cd boost_1_85_0
./bootstrap.sh --prefix=/usr/local
sudo ./b2 -j"$(nproc)" variant=release link=static,shared threading=multi runtime-link=shared install
sudo ldconfig
```
Verify with `grep BOOST_LIB_VERSION /usr/local/include/boost/version.hpp` — expect `1_85`.

**Other distros:** install Boost 1.85 via the distro package manager if
available, otherwise build from source as shown above. Arch users can pin
an older `boost` AUR package; Fedora users can sometimes find a
`boost1.85` compat package.

Install the remaining dependencies at once on Debian/Ubuntu:

``` sudo apt update && sudo apt install build-essential cmake pkg-config libssl-dev libzmq3-dev libsecp256k1-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libpgm-dev libusb-dev libgmp-dev```

Install the remaining dependencies at once on macOS with the provided Brewfile:
``` brew update && brew bundle --file=contrib/brew/Brewfile ```
(The Brewfile pulls `boost@1.85` — if you skipped the Boost step above, running
the Brewfile is sufficient, but you will still need `brew link --force --overwrite boost@1.85`
if a newer boost is already linked.)

FreeBSD one liner for required to build dependencies
```pkg install git gmake cmake pkgconf boost-libs cppzmq libsodium libsecp256k1```
(Ensure the `boost-libs` package resolves to 1.85 — if not, build from source.)

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

#### On Windows (64-bit, MSYS2 + MinGW-w64)

Windows binaries are built inside [MSYS2](https://www.msys2.org) using the
MinGW-w64 toolchain. The toolchain runs inside MSYS2 but produces native
Windows `.exe` binaries that run outside of it.

> **Boost 1.85 is required.** The MSYS2 `mingw-w64-x86_64-boost` package now
> ships Boost 1.90+, which has removed APIs this codebase depends on. **Do
> not `pacman -S mingw-w64-x86_64-boost`.** Build Boost 1.85 from source as
> described below. The build will fail at link time against any other
> version.
>
> **GCC 14 is required.** MSYS2's current toolchain ships GCC 15, which
> breaks the project in two ways: libstdc++ 15 removed transitive
> `<cstdint>` includes (project headers use `uint64_t` without including
> it), and GCC 15 defaults to a C++ standard where `std::unary_function`
> / `std::binary_function` are removed. Step 2b below installs GCC 14
> side-by-side. Do not skip it unless `gcc --version` already prints
> 14.x.

These instructions work on a headless Windows box (e.g. an EC2 instance you
only access via RDP/CLI) as well as on a desktop. All commands are run from
the **MinGW64** shell (`msys2_shell.cmd -mingw64`) unless stated otherwise.

##### 1. Install MSYS2

From PowerShell or cmd.exe:

```powershell
# Download + silent install
Invoke-WebRequest -Uri https://github.com/msys2/msys2-installer/releases/latest/download/msys2-x86_64-latest.exe -OutFile msys2.exe
Start-Process -Wait -FilePath .\msys2.exe -ArgumentList 'install','--root','C:\msys64','--confirm-command'
```

Add `C:\msys64\usr\bin` and `C:\msys64\mingw64\bin` to `PATH` for
convenience (optional).

From here on, all commands run inside the **MinGW64 shell**. How you
open it depends on whether your Windows session has a desktop:

**Desktop Windows (local machine or RDP):** run from PowerShell or cmd.exe:

```powershell
C:\msys64\msys2_shell.cmd -mingw64
```

A new terminal window opens — run every command in the following
sections inside that window. Leave it open for the whole build.

**Headless Windows (AWS EC2 via Systems Manager, SSH-only, any CLI-only
session):** `msys2_shell.cmd` spawns a GUI window that has nowhere to
render, so use bash directly instead. From PowerShell:

```powershell
$env:MSYSTEM = "MINGW64"
$env:CHERE_INVOKING = "1"
C:\msys64\usr\bin\bash.exe --login -i
```

That drops you into a persistent interactive MinGW64 bash prompt inside
the same PowerShell/SSM session — no extra window. Run every command in
the following sections at that prompt. Exit with `exit` or Ctrl+D.
Environment variables you set (e.g. `BOOST_ROOT`) persist for the
whole session.

(For scripted one-shots on either setup, you can run a single command
and exit with `C:\msys64\msys2_shell.cmd -mingw64 -defterm -here -no-start -c "<command>"`.
Each invocation is a fresh shell, so env vars do not persist between
calls — use the interactive forms above for anything multi-step.)

##### 2. Update MSYS2 and install toolchain / dependencies

Inside the MinGW64 shell: 
(you may have to run the packages install command twice if you see any bandwidth related errors)

```bash
pacman -Syu --noconfirm        # may ask you to close and reopen the shell, then re-run
pacman -Syu --noconfirm

pacman -S --noconfirm \
  git make autoconf automake libtool pkg-config unzip \
  mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-zeromq \
  mingw-w64-x86_64-libsodium \
  mingw-w64-x86_64-hidapi \
  mingw-w64-x86_64-unbound \
  mingw-w64-x86_64-openssl \
  mingw-w64-x86_64-icu
```

`mingw-w64-x86_64-icu` is required because Boost.Locale links against
ICU. Without it, the final link of `electroneum-wallet-cli.exe` fails
with `cannot find -licuio / -licuin / -licuuc / -licudt / -licutu`.

**Explicitly do not install `mingw-w64-x86_64-boost`.** If a previous attempt
installed it, remove it first: `pacman -R --noconfirm mingw-w64-x86_64-boost`.

> **Do not install `mingw-w64-x86_64-cmake` either.** The MSYS2 repo now
> ships CMake 4.x, which has *removed* the classic `FindBoost` module this
> project relies on (see policy CMP0167). You will hit `Could NOT find
> Boost (missing: Boost_INCLUDE_DIR ...)` even with a correct Boost 1.85
> install. Use an official CMake 3.x release instead:
>
> ```bash
> cd /c
> curl -LO https://github.com/Kitware/CMake/releases/download/v3.29.9/cmake-3.29.9-windows-x86_64.zip
> unzip cmake-3.29.9-windows-x86_64.zip
>
> # put 3.x ahead of anything else on PATH for the current shell
> export PATH=/c/cmake-3.29.9-windows-x86_64/bin:$PATH
> cmake --version    # MUST print 3.29.9 — if it says 4.x, PATH is wrong
> which cmake        # MUST print /c/cmake-3.29.9-windows-x86_64/bin/cmake
>
> # persist it so every new MinGW64 shell picks it up automatically
> echo 'export PATH=/c/cmake-3.29.9-windows-x86_64/bin:$PATH' >> ~/.bashrc
> ```
>
> If `cmake --version` still shows 4.x after re-opening a shell, the
> MSYS2 cmake is ahead of the 3.29 install on PATH. Remove it outright:
>
> ```bash
> pacman -R --noconfirm mingw-w64-x86_64-cmake
> ```

##### 2b. Install GCC 14 side-by-side (required)

Check what `pacman` installed for you:

```bash
gcc --version
```

If it prints **15.x**, this project will fail to compile with errors like
`'uint64_t' does not name a type` and deprecation warnings for
`std::unary_function` / `std::binary_function`. The project's headers
use `uint64_t` without `#include <cstdint>`, relying on transitive
includes that libstdc++ 15 no longer ships; GCC 15 also defaults to a
C++ standard where the deprecated function-object base classes are
removed.

If `gcc --version` printed **14.x**, skip this section. On a fresh
MSYS2 install in 2026 this will not happen — the `toolchain`
metapackage pulls in GCC 15 — so expect to follow the steps below.

You can't simply `pacman -U` an older `mingw-w64-x86_64-gcc` package —
the newer `gcc-libs` provides a virtual `cc-libs` dependency that a
dozen other MSYS2 packages (cmake, curl, python, ninja, zeromq, ...)
now require, so pacman refuses the downgrade. Instead, install GCC 14
into a **standalone directory** and put it first on `PATH` for the
build:

```bash
# Download GCC 14 + matching binutils (versions current as of 2026-04)
cd /tmp
curl -LO https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-binutils-2.43.1-1-any.pkg.tar.zst
curl -LO https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-gcc-libs-14.2.0-3-any.pkg.tar.zst
curl -LO https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-gcc-14.2.0-3-any.pkg.tar.zst

# Verify all three downloaded fully (each should be MB, not bytes)
ls -lh *.pkg.tar.zst

# Extract into /c/gcc-14 (NOT over the top of /mingw64)
mkdir -p /c/gcc-14
cd /c/gcc-14
tar --zstd -xf /tmp/mingw-w64-x86_64-gcc-14.2.0-3-any.pkg.tar.zst      --strip-components=1 -C . mingw64
tar --zstd -xf /tmp/mingw-w64-x86_64-gcc-libs-14.2.0-3-any.pkg.tar.zst --strip-components=1 -C . mingw64
tar --zstd -xf /tmp/mingw-w64-x86_64-binutils-2.43.1-1-any.pkg.tar.zst --strip-components=1 -C . mingw64

# Verify
ls /c/gcc-14/bin/gcc.exe            # should exist
/c/gcc-14/bin/gcc.exe --version     # should print 14.2.0
```

GCC 14 in this standalone tree is missing the mingw-w64 CRT headers
(`stdint.h`, `stdlib.h`, etc.) and import libs. Copy them from the
MSYS2 install (this project's `gcc-14` tree is where `#include_next`
needs to find them — setting `C_INCLUDE_PATH` alone breaks
`#include_next` ordering):

```bash
cp -rn /mingw64/include/* /c/gcc-14/include/ 2>/dev/null
cp -rn /mingw64/lib/*     /c/gcc-14/lib/     2>/dev/null
```

Put GCC 14 first on PATH, set `CC`/`CXX`, and persist:

```bash
export PATH=/c/gcc-14/bin:$PATH
export CC=/c/gcc-14/bin/gcc.exe
export CXX=/c/gcc-14/bin/g++.exe

echo 'export PATH=/c/gcc-14/bin:$PATH' >> ~/.bashrc
echo 'export CC=/c/gcc-14/bin/gcc.exe' >> ~/.bashrc
echo 'export CXX=/c/gcc-14/bin/g++.exe' >> ~/.bashrc

# Sanity checks
gcc --version                       # MUST print 14.2.0 (not 15.x)
which gcc                           # MUST print /c/gcc-14/bin/gcc
echo '#include <stdlib.h>
int main(void) { return 0; }' > /tmp/t.c
gcc -c /tmp/t.c -o /tmp/t.o && echo "compile OK"
rm -f /tmp/t.c /tmp/t.o
```

If the package URLs above return small HTML 404 files instead of
multi-MB archives, MSYS2 has rotated out those specific versions.
List what's currently available and pick the latest GCC 14 build:

```bash
curl -sL https://repo.msys2.org/mingw/mingw64/ | grep -oE 'mingw-w64-x86_64-gcc-14[^"]*\.pkg\.tar\.zst'      | sort -u | tail -5
curl -sL https://repo.msys2.org/mingw/mingw64/ | grep -oE 'mingw-w64-x86_64-gcc-libs-14[^"]*\.pkg\.tar\.zst' | sort -u | tail -5
curl -sL https://repo.msys2.org/mingw/mingw64/ | grep -oE 'mingw-w64-x86_64-binutils-2\.4[0-9][^"]*\.pkg\.tar\.zst' | sort -u | tail -5
```

##### 3. Build Boost 1.85 from source

Still inside the MinGW64 shell, and with GCC 14 active
(`gcc --version` must print 14.x before you start — Boost picks up
whichever gcc is first on `PATH`, and if it's built with GCC 15 the
Electroneum compile will fail even if the rest of the setup is correct):

```bash
cd /c
curl -LO https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.bz2
tar xjf boost_1_85_0.tar.bz2
cd boost_1_85_0

./bootstrap.sh --with-toolset=gcc --prefix=/c/boost_1_85_0

./b2 -j"$(nproc)" \
  toolset=gcc \
  address-model=64 \
  link=static \
  runtime-link=static \
  threading=multi \
  variant=release \
  --layout=system \
  --with-system --with-filesystem --with-thread --with-date_time \
  --with-chrono --with-regex --with-serialization \
  --with-program_options --with-locale \
  --prefix=/c/boost_1_85_0 \
  install
```

If you already built Boost with GCC 15 before realising you need GCC 14,
wipe and rebuild — the `.a` files are ABI-incompatible between major GCC
versions and you'll get undefined-reference link errors later otherwise:

```bash
cd /c/boost_1_85_0
rm -rf bin.v2/ stage/ include/ lib/
# then re-run the bootstrap + b2 commands above
```

The four settings that matter:
- `toolset=gcc` inside MinGW64 resolves to MinGW's gcc — matches what the
  main build uses. Do **not** run `bootstrap.bat gcc` from cmd.exe; that
  picks up the wrong `gcc`.
- `link=static runtime-link=static` produces the `.a` files the static
  build expects.
- `--layout=system` gives plain names (`libboost_system.a`) instead of
  versioned ones (`libboost_system-mgw13-mt-s-x64-1_85.a`), which is what
  CMake's `FindBoost` looks for by default with MinGW.
- `--with-*` restricts to the components used by this project (much
  faster than `--build-type=complete`).

Verify:
```bash
grep BOOST_LIB_VERSION /c/boost_1_85_0/include/boost/version.hpp    # expect 1_85
ls /c/boost_1_85_0/lib/libboost_system.a                            # must exist
```

##### 4. Build secp256k1

```bash
cd ~
git clone https://github.com/bitcoin-core/secp256k1
cd secp256k1
./autogen.sh && ./configure && make && make install
```

##### 5. Clone and build Electroneum

Clone the repo:

```bash
cd ~
git clone --recursive https://github.com/electroneum/electroneum.git
cd electroneum

# Optional: check out a specific tag, e.g.
# git checkout v5.0.0.0 && git submodule update --init --recursive
```

**Do not use `make release-static-win64` on Windows.** That target's
cmake invocation is incompatible with the Boost 1.85 setup on modern
CMake: the project's `cmake_minimum_required` predates policy CMP0074,
which means CMake deliberately ignores the `BOOST_ROOT` env var and fails
to locate Boost with `Could NOT find Boost (missing: Boost_INCLUDE_DIR
system filesystem thread ...)`. Invoke cmake directly instead — this
lets us set `CMAKE_POLICY_DEFAULT_CMP0074=NEW` and pass Boost paths as
`-D` flags, which CMake always respects.

**You must also patch `cmake/64-bit-toolchain.cmake` once.** The
toolchain file pins `CMAKE_FIND_ROOT_PATH` to `C:/msys64/mingw64` and
sets library/include searches to `ONLY` within it. That makes any Boost
install outside `C:/msys64` invisible to `find_library`, and
command-line `-D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH` flags get
overwritten when the toolchain file is evaluated. The patches below
change `ONLY` to `BOTH` and add `C:/boost_1_85_0` to the find-root
path list.

First, patch the toolchain file from the repo root:

```bash
cd ~/electroneum

# relax find-root restrictions so libraries outside C:/msys64 can be located
sed -i 's/CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY/CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH/' cmake/64-bit-toolchain.cmake
sed -i 's/CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY/CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH/' cmake/64-bit-toolchain.cmake

# add Boost's install root to the find-path list
cat >> cmake/64-bit-toolchain.cmake <<'EOF'

# Let find_library/find_path see our out-of-tree Boost install
list (APPEND CMAKE_FIND_ROOT_PATH "C:/boost_1_85_0")
EOF

# verify
grep -n 'MODE_LIBRARY\|MODE_INCLUDE\|CMAKE_FIND_ROOT_PATH' cmake/64-bit-toolchain.cmake
```

Expected output of the `grep`:
```
43:set (CMAKE_FIND_ROOT_PATH "${MSYS2_FOLDER}/mingw64")
46:set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # Find programs on host
47:set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH) # Find libs in target
48:set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH) # Find includes in target
54:list (APPEND CMAKE_FIND_ROOT_PATH "C:/boost_1_85_0")
```

Now configure and build. **Use absolute Windows-native paths** for the
toolchain file and source directory — cmake on Windows sometimes fails
to resolve MSYS-style relative paths like `../../cmake/...` even when
your shell can `ls` them fine. First figure out what your `~/electroneum`
maps to on the Windows filesystem:

```bash
cd ~/electroneum
cygpath -w "$(pwd)"    # e.g. prints C:\msys64\home\ssm-user\electroneum
```

Use that path (with forward slashes) as both `CMAKE_TOOLCHAIN_FILE` and
the source argument.

Before running cmake, verify the shell is clean:

```bash
gcc --version                          # MUST print 14.2.0 — if 15.x, redo step 2b
which gcc                              # MUST print /c/gcc-14/bin/gcc
ls /c/boost_1_85_0/lib/libboost_system.a    # must exist
cmake --version                        # MUST print 3.29.9 (not 4.x)
```

If `gcc` shows 15.x, re-export in this shell:

```bash
export PATH=/c/gcc-14/bin:$PATH
export CC=/c/gcc-14/bin/gcc.exe
export CXX=/c/gcc-14/bin/g++.exe
```

Then wipe any stale configure artefacts. If you did an earlier configure
that succeeded with GCC 15, its CMake cache has GCC 15 baked in and will
ignore the new `PATH` / `CC` / `CXX`:

```bash
cd ~/electroneum
rm -rf build/ CMakeCache.txt CMakeFiles/ cmake_install.cmake

# The previous configure writes a cmake-generated Makefile to the repo root.
# The project also ships its own Makefile. Check which one is present now:
head -3 Makefile 2>/dev/null
```

If `head -3 Makefile` prints `# CMAKE generated file: DO NOT EDIT!`,
it's the stale one — delete it so cmake regenerates fresh:

```bash
rm Makefile
```

If it prints normal variable assignments / target rules without a
"CMAKE generated" banner, it's the project's own — leave it.

Now configure, hard-pinning GCC 14 so the CMake cache can't silently
fall back to `/mingw64/bin/gcc` (which is still GCC 15):

```bash
cd ~/electroneum

# scrub any stale Boost env vars
unset Boost_ROOT BOOST_ROOT BOOST_INCLUDEDIR BOOST_LIBRARYDIR Boost_NO_SYSTEM_PATHS

# replace SOURCE_DIR below with your cygpath output (use / not \)
SOURCE_DIR=C:/msys64/home/ssm-user/electroneum

cmake -G "MSYS Makefiles" \
  -D CMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -D CMAKE_POLICY_DEFAULT_CMP0074=NEW \
  -D CMAKE_POLICY_DEFAULT_CMP0144=NEW \
  -D CMAKE_C_COMPILER=C:/gcc-14/bin/gcc.exe \
  -D CMAKE_CXX_COMPILER=C:/gcc-14/bin/g++.exe \
  -D STATIC=ON \
  -D ARCH="x86-64" \
  -D BUILD_64=ON \
  -D CMAKE_BUILD_TYPE=Release \
  -D BUILD_TAG="win-x64" \
  -D CMAKE_TOOLCHAIN_FILE=$SOURCE_DIR/cmake/64-bit-toolchain.cmake \
  -D MSYS2_FOLDER=C:/msys64 \
  -D BOOST_ROOT=C:/boost_1_85_0 \
  -D BOOST_INCLUDEDIR=C:/boost_1_85_0/include \
  -D BOOST_LIBRARYDIR=C:/boost_1_85_0/lib \
  -D Boost_INCLUDE_DIR=C:/boost_1_85_0/include \
  -D Boost_NO_SYSTEM_PATHS=ON \
  -D Boost_NO_BOOST_CMAKE=ON \
  -D Boost_USE_STATIC_LIBS=ON \
  -D Boost_USE_STATIC_RUNTIME=ON \
  $SOURCE_DIR
```

Watch the first few lines of configure output — they must say:
```
-- The C compiler identification is GNU 14.2.0
-- The CXX compiler identification is GNU 14.2.0
```

If they say 15.x, the `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` flags
didn't take effect — stop, double-check `/c/gcc-14/bin/gcc.exe` exists,
and retry.

Once configure finishes, cmake will print:
```
-- Build files have been written to: C:/msys64/home/ssm-user/electroneum
```

Note the build files land in the **repo root**, not in a `build/` subdir —
this is an in-source build (a quirk of running cmake from the repo root
with the source dir as `$SOURCE_DIR`). The generated `Makefile` sits next
to the project's own `Makefile` and takes precedence. Compile from the
repo root:

```bash
cd ~/electroneum
make -j"$(nproc)"
```

The resulting binaries are in `~/electroneum/bin/`. Compilation typically
takes 20–60 minutes on a modest EC2 instance.

Key things to notice:
- The toolchain-file patches are a **one-time edit** to the repo. Re-apply
  them if you ever `git pull` and the file changes.
- `SOURCE_DIR` is a **Windows-native absolute path** with forward slashes.
  cmake on Windows under MSYS2 resolves some relative paths inconsistently
  with the shell; absolute Windows paths eliminate the ambiguity entirely.
- `CMAKE_POLICY_DEFAULT_CMP0074=NEW` and `CMAKE_POLICY_DEFAULT_CMP0144=NEW`
  are what make `Boost_ROOT` (mixed case) and `BOOST_ROOT` (upper case)
  actually work on a project with an old `cmake_minimum_required`.
  Without them you get "For compatibility, find_package is ignoring the
  variable" and the search fails.
- `Boost_INCLUDE_DIR=C:/boost_1_85_0/include` pins the include dir
  directly — FindBoost occasionally can't locate it through `find_path`
  even when the headers are demonstrably in place, which cascades into a
  bogus `Boost_INCLUDE_DIR-NOTFOUND/lib` entry in the library search
  path and causes the whole detection to fail.
- `Boost_NO_BOOST_CMAKE=ON` disables Boost's own `BoostConfig.cmake`
  hunting. b2 installs drop a `lib/cmake/` directory alongside the libs;
  leaving Config mode enabled can half-activate it and confuse the old
  FindBoost module.
- Paths like `C:/boost_1_85_0` (not `/c/boost_1_85_0`). MSYS-style
  paths get mangled by CMake on Windows.
- `Boost_USE_STATIC_LIBS=ON` + `Boost_USE_STATIC_RUNTIME=ON` match the
  `link=static runtime-link=static` b2 build.
- Add `-D Boost_DEBUG=ON` to the cmake line if you ever need to see
  exactly which paths and filenames it searches.

##### Troubleshooting

- `'uint64_t' does not name a type` in `perf_timer.h` / `crypto-ops.h`
  during compile, or `std::unary_function` / `std::binary_function`
  deprecation errors: you're compiling with GCC 15. Go through step 2b
  to install GCC 14, rebuild Boost with it (step 3's "nuke and rebuild"
  block), wipe the CMake cache (`rm Makefile CMakeCache.txt -rf
  CMakeFiles/ cmake_install.cmake`), and reconfigure with the
  `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` flags set to `/c/gcc-14/bin/`.
- `Could NOT find Boost (missing: ... system filesystem ...)`: almost
  always means (a) Boost wasn't built with `link=static`, (b) layout isn't
  `system`, (c) the CMake cache from a previous attempt is pinning wrong
  paths, or (d) the toolchain-file patch from step 5 wasn't applied so
  `CMAKE_FIND_ROOT_PATH_MODE_LIBRARY` is still `ONLY`. Delete the cache
  and retry. For verbose search output, add `-D Boost_DEBUG=ON` to the
  cmake line.
- `Could NOT find Boost (missing: ...)` *with* `Policy CMP0167 is not set`
  warning: you're on CMake 4.x. That version removed the `FindBoost`
  module entirely. Install CMake 3.29.9 per step 2's note and put it
  first on PATH.
- `For compatibility, find_package is ignoring the variable` warning:
  `CMAKE_POLICY_DEFAULT_CMP0074=NEW` and/or
  `CMAKE_POLICY_DEFAULT_CMP0144=NEW` are missing from the cmake command.
- `undefined reference` errors to `boost::*` during link: Boost and
  Electroneum were built with different GCC majors. Check
  `gcc --version` and confirm both Boost and the Electroneum configure
  used the same compiler. If not, nuke Boost (`rm -rf
  /c/boost_1_85_0/{bin.v2,stage,include,lib}`) and rebuild it with the
  currently-active gcc.
- `cannot find -licuio / -licuin / -licuuc / -licudt / -licutu` at the
  final link of `electroneum-wallet-cli.exe` (usually at 98%+): ICU
  isn't installed or isn't visible to the linker.
  Boost.Locale requires ICU, and the project's cmake `find_package`
  pulls in `locale`. Fix:

  ```bash
  pacman -S --noconfirm mingw-w64-x86_64-icu
  cp -rn /mingw64/lib/libicu*     /c/gcc-14/lib/     2>/dev/null
  cp -rn /mingw64/include/unicode /c/gcc-14/include/ 2>/dev/null
  cd ~/electroneum && make -j"$(nproc)"   # no reconfigure needed
  ```
- Builds on 32-bit Windows are no longer supported by the project Boost
  build instructions above. If you need a 32-bit build, adjust
  `address-model=32` and use the `mingw32` MSYS2 shell consistently.

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