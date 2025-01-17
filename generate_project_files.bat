@echo off

echo "Generate project files"


conan install . --output-folder=build --build missing --profile=conanProfileDebug --profile:b=conanProfileDebug

cd build

cmake .. --fresh -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
