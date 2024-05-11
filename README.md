# Aleph One

Aleph One is the open source continuation of Bungie™’s _Marathon® 2_ and _Marathon Infinity_ game engines. Aleph One plays _Marathon_, _Marathon 2_, _Marathon Infinity_, and third-party content on a variety of platforms.

Aleph One is available under the terms of the [GNU General Public License (GPL 3)](http://www.gnu.org/licenses/gpl-3.0.html)

[![](https://dcbadge.vercel.app/api/server/NvF3pdV)](https://discord.gg/NvF3pdV)

# Download

To download ready-to-run versions of all three _Marathon_ games for macOS,
Windows, and Linux Flatpak, visit
[alephone.lhowon.org](https://alephone.lhowon.org)

You can also download The _Maraton_ Games on Steam
[https://store.steampowered.com/search/?publisher=Aleph%20One%20Developers]([url](https://store.steampowered.com/search/?publisher=Aleph%20One%20Developers))

# Build from source

## CI status

[![Build Status](https://github.com/Aleph-One-Marathon/alephone/actions/workflows/ci-build.yml/badge.svg)](https://github.com/Aleph-One-Marathon/alephone/actions/workflows/ci-build.yml?query=branch%3Amaster+)

## Scenario data

If you only want an Aleph One executable, you can simply download and untar a release source tarball. However, to build all-in-one Mac apps, flatpaks, or Windows zip files, you will need to populate the data/Scenarios directory. The easiest way to do that is to clone the repository and submodules:

    git clone --recurse-submodules https://github.com/Aleph-One-Marathon/alephone.git

Alternatively, you can download the [data files](https://alephone.lhowon.org/scenarios.html) and unzip them in the data/Scenarios/ directory.

## macOS

These instructions assume familiarity with the Xcode tools and the macOS command line.

macOS dependencies are managed by [vcpkg](https://github.com/microsoft/vcpkg).

Some users have had issues building Aleph One when there are spaces in the path to vcpkg and alephone, so it is recommended to put them in paths without spaces.

Download, bootstrap, and install vcpkg:

    git clone https://github.com/microsoft/vcpkg
    ./vcpkg/bootstrap-vcpkg.sh
    ./vcpkg/vcpkg integrate install

`cd` into Aleph One's vcpkg subdirectory and use the `install-arm-osx.sh` and `install-x64-osx.sh` scripts to install macOS dependencies for arm64 and x64.

You should now be able to open `PBProjects/AlephOne.xcodeproj` in Xcode and build Aleph One.

## Windows

Windows builds are built using [Visual Studio](https://visualstudio.microsoft.com/vs/)

Windows dependencies are managed by [vcpkg](https://github.com/microsoft/vcpkg).

Note this important recommendation in the vcpkg getting-started guide: _If installing globally, we recommend a short install path like: C:\src\vcpkg or C:\dev\vcpkg, since otherwise you may run into path issues for some port build systems._ Spaces in the path and non-ASCII characters can also cause problems. These notes apply to the Aleph One source location as well.

Download, bootstrap, and install vcpkg:

    git clone https://github.com/microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
    .\vcpkg integrate install

You should now be able to build Aleph One using the `VisualStudio\AlephOne.sln` project file

## Linux/FreeBSD/other

Linux/FreeBSD/other builds are built using autoconf. If you downloaded a source tarball, the configure system is already set up for you. If you cloned from git, you first need to set up the configure system. Install `autoconf` and `autoconf-archive` from your distro package manager, then:

    autoreconf -i

### Dependencies

Aleph One requires a C++17 compiler and the following libraries:

+ `Boost`
+ `SDL2`
+ `SDL2_image`
+ `SDL2_net`
+ `SDL2_ttf`
+ `zlib`
+ `libsndfile`
+ `openal-soft`

These libraries are recommended for full features and third-party scenario compatibility:

+ `curl` _for stats upload to lhowon.org_
+ `ffmpeg` _for music playback and film video export_
+ `miniupnpc` _for opening router ports_
+ `zziplib` _for using zipped plugins_

#### Fedora

First, enable the [RPM Fusion Repository](http://rpmfusion.org/Configuration).

Then, install the following packages.

    sudo dnf install boost-devel curl-devel ffmpeg-devel gcc-c++ \
      libpng-devel SDL2-devel SDL2_ttf-devel SDL2_image-devel SDL2_net-devel \
      zziplib-devel miniupnpc-devel openal-soft-devel libsndfile-devel

If you don't compile with FFmpeg support, you won't be able to use WebM export in Aleph One.

#### Ubuntu

Run this command to install the necessary prerequisites for building Aleph One:

    sudo apt install build-essential libboost-all-dev libsdl2-dev \
      libsdl2-image-dev libsdl2-net-dev libsdl2-ttf-dev \
      libzzip-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
      libpng-dev libcurl4-gnutls-dev libminiupnpc-dev libopenal-dev libsndfile1-dev

### Compile

First, run the configure script:

    ./configure

After running the configure script, start the compile process by running make:

    make

Once the compile is finished, you can install the executable by running:

    sudo make install

By default, the Aleph One executable is installed into `/usr/local/bin/alephone`.

### Run

You can download game data from the [Aleph One Scenarios](https://alephone.lhowon.org/scenarios.html) page. After unzipping one of the games, pass the directory as an argument to Aleph One:

    /usr/local/bin/alephone ~/Games/Marathon
