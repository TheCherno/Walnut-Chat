#!/bin/bash

pushd ..
Walnut/vendor/bin/premake/Linux/premake5 --cc=clang --file=Build-Headless.lua gmake2
popd
