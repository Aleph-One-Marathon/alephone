`cat ~/.vcpkg/vcpkg.path.txt`/vcpkg --overlay-triplets=custom-triplets --triplet=arm64-osx --x-install-root=installed-arm64-osx --feature-flags="versions" install

"`dirname \"$0\"`/merge_dylibs.sh"
