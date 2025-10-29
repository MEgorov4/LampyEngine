@echo off

cd ..
echo "Generate project files"

conan install . --output-folder=Engine/ThirdParty --build=missing --profile=Automation/conanProfileDebug --profile:b=Automation/conanProfileDebug

if not exist build (
    mkdir build
)
cd build

cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE="Engine/ThirdParty/conan_toolchain.cmake" --fresh

cd ..
cd Automation
