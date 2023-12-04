#! /bin/bash

cd "$(dirname "$0")"
mkdir -p merged_dylibs

# These are the original output names from vcpkg angle port, which are not what we want.
VCPKG_EGL_NAME="liblibEGL_angle.dylib"
VCPKG_GLES_NAME="liblibGLESv2_angle.dylib"

ARM_EGL_PATH="installed-arm64-osx/arm64-osx/lib/${VCPKG_EGL_NAME}"
X86_EGL_PATH="installed-x64-osx/x64-osx/lib/${VCPKG_EGL_NAME}"

ARM_GLES_PATH="installed-arm64-osx/arm64-osx/lib/${VCPKG_GLES_NAME}"
X86_GLES_PATH="installed-x64-osx/x64-osx/lib/${VCPKG_GLES_NAME}"

OUTPUT_EGL_NAME=libEGL.dylib
OUTPUT_GLES_NAME=libGLESv2.dylib

if [[ -f "${ARM_EGL_PATH}" && -f "${X86_EGL_PATH}" ]]; then
	echo "Will create universal binary for EGL and GLESv2 dyanmic libraries"
	
	/usr/bin/lipo "${ARM_EGL_PATH}" "${X86_EGL_PATH}" -create -output "merged_dylibs/${OUTPUT_EGL_NAME}"
	/usr/bin/lipo "${ARM_GLES_PATH}" "${X86_GLES_PATH}" -create -output "merged_dylibs/${OUTPUT_GLES_NAME}"
elif [ -f "${ARM_EGL_PATH}" ]; then
	echo "EGL and GLESv2 dyanmic libraries will be ARM64 only."
	/bin/cp "${ARM_EGL_PATH}" "merged_dylibs/${OUTPUT_EGL_NAME}"
	/bin/cp "${ARM_GLES_PATH}" "merged_dylibs/${OUTPUT_GLES_NAME}"
elif [ -f "${X86_EGL_PATH}" ]; then
	echo "EGL and GLESv2 dyanmic libraries will be x86-64 only."
	/bin/cp "${X86_EGL_PATH}" "merged_dylibs/${OUTPUT_EGL_NAME}"
	/bin/cp "${X86_GLES_PATH}" "merged_dylibs/${OUTPUT_GLES_NAME}"
else
	echo "Unable to find EGL and GLESv2 dyanmic libraries for any arch. This might be a big problem for your build."
fi

cd "merged_dylibs"

if [ -e "${OUTPUT_EGL_NAME}" ]; then
    install_name_tool -id "@rpath/${OUTPUT_EGL_NAME}" "${OUTPUT_EGL_NAME}"
    install_name_tool -change "@rpath/${VCPKG_GLES_NAME}" "@rpath/${OUTPUT_GLES_NAME}" "${OUTPUT_EGL_NAME}"

    install_name_tool -id "@rpath/${OUTPUT_GLES_NAME}" ${OUTPUT_GLES_NAME}
fi
