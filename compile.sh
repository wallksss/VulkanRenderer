glslc ./shaders/shader.vert -o ./shaders/vert.spv
glslc ./shaders/shader.frag -o ./shaders/frag.spv
glslc ./shaders/shader.vert -o - -S
glslc ./shaders/shader.frag -o - -S
