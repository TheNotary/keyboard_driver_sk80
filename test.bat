rmdir /s /q builds
cmake --preset release-config
cmake --build --preset release-build --target clean
cmake --build --preset release-build
ctest --preset test-release
