# Vulkan Pool

A simple pool game simulator built with C++ and the Vulkan API.

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
