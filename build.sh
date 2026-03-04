#!/usr/bin/env bash
# build.sh — Build helper for the allocator project.
# Usage:
#   ./build.sh [target]
#
# Targets:
#   compile   — Build all test executables (default)
#   test      — Build and run all tests
#   coverage  — Run tests with code coverage report
#   lint      — Run cpplint on source and test files
#   docs      — Generate Doxygen documentation
#   all       — Full workflow: compile → test → coverage → lint → docs
#   clean     — Remove the build directory entirely
#   help      — Show this message

set -euo pipefail

BUILD_DIR="build"
TARGET="${1:-compile}"

cmake_configure() {
    local build_type="${1:-Debug}"
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$build_type"
}

case "$TARGET" in
    compile)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target compile
        ;;
    test)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target compile
        cd "$BUILD_DIR" && ctest --output-on-failure
        ;;
    coverage)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target coverage
        ;;
    lint)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target lint
        ;;
    docs)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target documentation
        ;;
    all)
        cmake_configure Debug
        cmake --build "$BUILD_DIR" --target default
        ;;
    clean)
        rm -rf "$BUILD_DIR"
        echo "Cleaned $BUILD_DIR/"
        ;;
    help|--help|-h)
        sed -n '2,/^$/{ s/^# //; s/^#//; p }' "$0"
        ;;
    *)
        echo "Unknown target: $TARGET" >&2
        echo "Run '$0 help' for usage." >&2
        exit 1
        ;;
esac
