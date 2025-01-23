sudo apt-get update
sudo apt-get install -y build-essential make cmake ninja-build pkg-config \
    libxcb-util-dev libxcb-util0-dev 
conan install . --output-folder=Engine/ThirdParty --build=missing --profile=conanProfileDebugLinux --profile:b=conanProfileDebugLinux

if [ ! -d "build" ]; then
    mkdir build
fi

cd build || exit
cmake .. -DCMAKE_TOOLCHAIN_FILE="Engine/ThirdParty/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug
