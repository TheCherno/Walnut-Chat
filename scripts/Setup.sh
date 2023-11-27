#!/bin/bash

pushd "$(dirname "$0")"/..
Walnut/vendor/bin/premake/Linux/premake5 --cc=clang --file=Build-Headless.lua gmake2
popd
