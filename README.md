# Graphics Processing Project (UFSCar): 3D Pool with Vulkan

This project is a 3D pool game simulator written in C++ and using the Vulkan SDK.

## Authors

*   [Anderson Sassaki](https://github.com/wallksss): Resposible for setting up window creation, Vulkan, shaders and all the basics.
*   [Giovanna Vieira](https://github.com/gihtheghost): Loaded the objects and their textures on scene.
*   [Lorenzo Grippo](https://github.com/loren-gc): Resposible for setting up the lighting and make everything look pretty.
*   [Ti Ribeiro](https://github.com/tiribsil): User input, physics emulation and object movement.

## Fulfillment of Project Specifications

This section details how each requirement from the assignment was met.

✅ **1. Visualization of at least one 3D object per group member**
> There is a pool table, 16 balls, a cue stick and a lamp.

✅ **2. Use of a custom shader**
> A custom fragment shader was implemented to simulate lighting in the scene.

✅ **3. Definition of at least two cameras**
> The project features a controllable camera, so there are virtually infinite camera positions.

✅ **4. Simple movement of at least one object**
> There is a lot of movement. And physics.

✅ **5. Application of a texture to at least one object**
> Each object has its own material.

## Main Implemented Features

*   **Vulkan Rendering:** The entire scene is rendered using the Vulkan API, allowing for low-level control over the graphics pipeline and GPU resource management.
*   **Complete 3D Scene:** The environment is composed of multiple objects that form a cohesive pool game scene, including the table, cue stick, and all 16 balls.
*   **Interaction and Simplified Physics:** The player can aim with the cue stick and shoot the cue ball, which moves across the scene, simulating the basic mechanics of the game.

## Main UNimplemented Features

*   **Winning:** It's impossible.
*   **Scoring Points:** It's why **winning** is **not possible**.

## Requirements

To build and run this project, you will need the following dependencies installed on your system:

*   **Vulkan SDK**: Provides the Vulkan headers and validation layers.
*   **GLFW**: Used for window and input management.
*   **GLM (OpenGL Mathematics)**: A header-only library for vector and matrix operations.
*   **A C++17 compatible compiler**: Such as g++ on Linux or MSVC on Windows.
*   **Make**: The build system used for this project.

You can run the following command to install all dependencies in Debian based Linux distributions:
```bash
apt install glslc libvulkan-dev libglfw3-dev libglm-dev libxi-dev libxxf86vm-dev vulkan-validationlayers
```

## Building the Project

This project uses a `Makefile` that is compatible with both Linux and Windows. Before building, you must set the following environment variables to point to the locations of your installed dependencies:

*   `VULKAN_SDK`: Path to your Vulkan SDK installation.
*   `GLFW_PATH`: Path to your GLFW library installation.
*   `GLM_PATH`: Path to your GLM library installation.

**Example for Linux:**
```bash
export VULKAN_SDK=/path/to/your/vulkansdk
export GLFW_PATH=/path/to/your/glfw
export GLM_PATH=/path/to/your/glm
```

**Example for Windows:**
```powershell
set VULKAN_SDK=C:/VulkanSDK/x.x.x.x
set GLFW_PATH=C:/path/to/your/glfw
set GLM_PATH=C:/path/to/your/glm
```

Once the environment variables are set, you can build the project by running the following command in the root directory:

```bash
make
```

## Running the Application

To run the compiled application, use:

```bash
make run
```

## Controls

### Camera Controls

*   **W / S**: Pitch the camera up and down.
*   **A / D**: Yaw the camera left and right.
*   **Q / E**: Zoom the camera in and out.

### Game Controls

*   **Left / Right Arrow Keys**: Rotate the cue stick around the cue ball.
*   **Spacebar**: Shoot the cue ball.
