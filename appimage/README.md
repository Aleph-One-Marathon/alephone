Build instructions
------------------

First install the build dependencies:
```
sudo apt install --no-install-recommends \
  build-essential autoconf autoconf-archive nasm wget git fuse3 \
  libboost-all-dev libsdl2-dev libsdl2-image-dev libsdl2-net-dev
  libsdl2-ttf-dev libzzip-dev zlib1g-dev libpng-dev libvpx-dev libvorbis-dev \
  libcurl4-gnutls-dev libminiupnpc-dev libopenal-dev libsndfile1-dev \
  libglu1-mesa-dev
```

Fetch all required files:
`./fetch-files.sh`

Build AlephOne and create AppImages:
`./build.sh`

AppImages will be saved in the directory `out`.

