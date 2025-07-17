# Graphics Processing Project: 3D Pool with Vulkan

This project is a 3D pool game simulator written in C++ and using the Vulkan SDK.

## Fulfillment of Project Specifications

This section details how each requirement from the assignment was met.

✅ **1. Visualization of at least one 3D object per group member**
> Each group member was responsible for the creation, positioning, and individual scaling of one or more objects in the scene, contributing to the virtual environment:
> - **Pool Table:** Created and positioned by [Student Name 1].
> - **Cue Stick:** Created and positioned by [Student Name 2].
> - **Cue Ball:** Created and positioned by [Student Name 3].
> - **Colored Balls:** Created and positioned by [Student Name 4].

✅ **2. Use of a custom shader**
> A **custom fragment shader** was implemented to simulate a **specular highlight** effect on the pool balls. This shader calculates light reflection to give the objects a more polished and realistic appearance, rather than using a standard material system.

✅ **3. Definition of at least two cameras**
> The project features two distinct cameras to offer different perspectives of the scene:
> - **Orbital Camera:** A free camera that can be rotated around the table, with pitch, yaw, and zoom controls. This is the primary camera for aiming.
> - **Top-down Camera:** A static camera positioned above the table, providing a strategic overview of the game, ideal for analyzing the position of the balls.
>
> The player can switch between cameras using a dedicated key (see the controls section).

✅ **4. Simple movement of at least one object**
> Two types of movement are implemented:
> - **Rotational Movement:** The cue stick rotates around the cue ball, allowing the player to aim.
> - **Translational Movement:** After the shot, the cue ball moves across the table, following the direction aimed by the cue stick.

✅ **5. Application of a texture to at least one object**
> A **wood texture** was applied to the side structure of the pool table. We used UV coordinate mapping to ensure the wood image wrapped around the 3D model correctly and cohesively.

## Main Implemented Features

*   **Vulkan Rendering:** The entire scene is rendered using the Vulkan API, allowing for low-level control over the graphics pipeline and GPU resource management.
*   **Complete 3D Scene:** The environment is composed of multiple objects that form a cohesive pool game scene, including the table, cue stick, and all 16 balls.
*   **Dual Camera System:** The implementation of two cameras (orbital and top-down) enhances interactivity and gameplay, allowing for both precise aiming and strategic viewing.
*   **Interaction and Simplified Physics:** The player can aim with the cue stick and "shoot" the cue ball, which moves across the scene, simulating the basic mechanics of the game.

## Requirements

To build and run this project, you will need the following dependencies installed on your system:

*   **Vulkan SDK**: Provides the Vulkan headers and validation layers.
*   **GLFW**: Used for window and input management.
*   **GLM (OpenGL Mathematics)**: A header-only library for vector and matrix operations.
*   **A C++17 compatible compiler**: Such as g++ on Linux or MSVC on Windows.
*   **Make**: The build system used for this project.

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
