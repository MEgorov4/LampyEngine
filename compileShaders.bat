echo "COMPILE_SHADERS"
glslc.exe Source/Shaders/shader.vert -o build/vert.spv
glslc.exe Source/Shaders/shader.frag -o build/frag.spv