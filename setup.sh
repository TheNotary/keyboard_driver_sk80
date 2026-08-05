#!/bin/bash
set -e

# ARM/aarch64 requires this for vcpkg to use system-installed tools
case "$(uname -m)" in
    aarch64|arm*|s390x|ppc64le|riscv*) export VCPKG_FORCE_SYSTEM_BINARIES=1 ;;
esac

# Bootstrap vcpkg if not present
if [ ! -d "vcpkg" ] || [ ! -f "vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    git submodule update --init
    ./vcpkg/bootstrap-vcpkg.sh
fi

export VCPKG_ROOT="$(pwd)/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"

echo "Installing dependencies..."
vcpkg install

echo "Configuring (Debug)..."
cmake --preset linux-debug

echo "Building..."
cmake --build --preset linux-debug-build

echo "Running tests..."
ctest --preset test-linux-debug

echo "Done! Build artifacts are in build/linux-debug/"
