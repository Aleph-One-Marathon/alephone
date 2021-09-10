# Mac

1. Clone vcpkg (https://github.com/microsoft/vcpkg)
2. cd vcpkg
3. Bootstrap vcpkg: ./booststrap-vcpkg.sh
4. ./vcpkg integrate install
5. Use either install-arm64-osx.sh or install-x64-osx.sh if you want to build for your machine architecture; run both if you intend to build universal binaries
6. Build Aleph One in Xcode

# Windows

1. Clone vcpkg (https://github.com/microsoft/vcpkg)
2. cd vcpkg
3. Bootstrap vcpkg: bootstrap-vcpkg.bat
4. vcpkg integrate install
5. In Visual Studio, click Build
