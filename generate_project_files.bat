@echo off

echo "Generate project files"

:: Устанавливаем зависимости через Conan и сохраняем их в папке Engine/ThirdParty
conan install . --output-folder=Engine/ThirdParty --build=missing --profile=conanProfileDebug --profile:b=conanProfileDebug

:: Переходим в папку build
if not exist build (
    mkdir build
)
cd build

:: Вызываем CMake с правильным путём к conan_toolchain.cmake
cmake .. -DCMAKE_TOOLCHAIN_FILE="Engine/ThirdParty/conan_toolchain.cmake" --fresh
