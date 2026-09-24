#!/usr/bin/env bash
# Thin wrapper for the single supported core-solver configuration.
set -euo pipefail
build_type=Production
build_dir=build
cmake_args=()
for arg in "$@"; do
  case "$arg" in
    debug) build_type=Debug ;;
    production) build_type=Production ;;
    testing) build_type=Testing ;;
    --name=*) build_dir="${arg#*=}" ;;
    --prefix=*) cmake_args+=("-DCMAKE_INSTALL_PREFIX=${arg#*=}") ;;
    --unit-testing) cmake_args+=(-DENABLE_UNIT_TESTING=ON) ;;
    --assertions) cmake_args+=(-DENABLE_ASSERTIONS=ON) ;;
    --auto-download) cmake_args+=(-DENABLE_AUTO_DOWNLOAD=ON) ;;
    --ninja) cmake_args+=(-G Ninja) ;;
    -D*) cmake_args+=("$arg") ;;
    --help|-h)
      echo 'Usage: ./configure.sh [debug|production|testing] [--ninja] [--name=DIR]'
      echo '       [--unit-testing] [--assertions] [--auto-download] [--prefix=DIR] [-DKEY=VALUE]'
      exit 0 ;;
    *) echo "Unsupported core build argument: $arg" >&2; exit 1 ;;
  esac
done
exec cmake -S "$(dirname "$0")" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" "${cmake_args[@]}"
