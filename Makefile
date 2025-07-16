# Makefile for Vulkan Project

# =============================================================================
#                            ENVIRONMENT SETUP
# =============================================================================
# This Makefile is designed to be cross-platform (Linux/Windows).
# For it to work, you need to set the following environment variables:
#
# VULKAN_SDK: Path to the Vulkan SDK.
#   - Linux:   export VULKAN_SDK=/path/to/your/vulkansdk
#   - Windows: set VULKAN_SDK=C:/VulkanSDK/x.x.x.x
#
# GLFW_PATH: Path to the GLFW library.
#   - Linux:   export GLFW_PATH=/path/to/your/glfw
#   - Windows: set GLFW_PATH=C:/path/to/your/glfw
#
# GLM_PATH: Path to the GLM library.
#   - Linux:   export GLM_PATH=/path/to/your/glm
#   - Windows: set GLM_PATH=C:/path/to/your/glm
#
# If these variables are not set, the Makefile will try to use default system paths.
# =============================================================================

# =============================================================================
#                            PROJECT CONFIGURATION
# =============================================================================
TARGET = VulkanTest
SRCDIR = src
INCDIR = libs
OBJDIR = obj
SHADERDIR = shaders

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
SHADERS_SRC = $(wildcard $(SHADERDIR)/*.vert) $(wildcard $(SHADERDIR)/*.frag)
SHADERS_SPV = $(patsubst %.vert,%.spv,$(SHADERS_SRC)) 
SHADERS_SPV := $(patsubst %.frag,%.spv,$(SHADERS_SPV))

# =============================================================================
#                       OS-SPECIFIC CONFIGURATION
# =============================================================================

# Default to Linux, then check for Windows
OS_NAME = Linux
ifeq ($(OS), Windows_NT)
    OS_NAME = Windows
endif

# --- Linux (g++) ---
ifeq ($(OS_NAME),Linux)
    CXX = g++
    GLSL_COMPILER = $(VULKAN_SDK)/bin/glslc
    CXXFLAGS = -std=c++17 -O2 -g
    INCLUDES = -I$(INCDIR)
    LDFLAGS = -L$(VULKAN_SDK)/lib -L$(GLFW_PATH)/lib
    LIBS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
    RM = rm -rf
    TARGET_EXEC = $(TARGET)

# --- Windows (MSVC) ---
else
    CXX = cl
    GLSL_COMPILER = "$(VULKAN_SDK)/Bin/glslc.exe"
    CXXFLAGS = /nologo /MD /std:c++17 /O2 /EHsc /D_CRT_SECURE_NO_WARNINGS
    INCLUDES = /I"$(VULKAN_SDK)/Include" /I"$(GLFW_PATH)/include" /I"$(GLM_PATH)" /I"$(INCDIR)"
    LDFLAGS = /LIBPATH:"$(VULKAN_SDK)/Lib" /LIBPATH:"$(GLFW_PATH)/lib-vc2022"
    LIBS = vulkan-1.lib glfw3.lib user32.lib gdi32.lib shell32.lib
    RM = rmdir /s /q
    TARGET_EXEC = $(TARGET).exe
endif

# =============================================================================
#                                 BUILD RULES
# =============================================================================
.PHONY: all clean run shaders

all: shaders $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJECTS)
	@echo "[LD]   $@"
ifeq ($(OS_NAME),Linux)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
else
	$(CXX) $(CXXFLAGS) /Fe$@ $^ $(LIBS) /link $(LDFLAGS)
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp shaders shaders
	@mkdir -p $(OBJDIR)
	@echo "[CXX]  $<"
ifeq ($(OS_NAME),Linux)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(INCLUDES) /c $< /Fo$@
endif

shaders: $(SHADERDIR)/vert.spv $(SHADERDIR)/frag.spv

$(SHADERDIR)/vert.spv: $(SHADERDIR)/shader.vert
	@echo "[GLSL] $< -> $@"
	$(GLSL_COMPILER) $< -o $@

$(SHADERDIR)/frag.spv: $(SHADERDIR)/shader.frag
	@echo "[GLSL] $< -> $@"
	$(GLSL_COMPILER) $< -o $@

# =============================================================================
#                               UTILITY RULES
# =============================================================================
run: all
	@echo "[RUN]  $(TARGET_EXEC)"
ifeq ($(OS_NAME),Linux)
	./$(TARGET_EXEC)
else
	$(TARGET_EXEC)
endif

clean:
	@echo "[CLEAN] Removing build artifacts..."
ifeq ($(OS_NAME),Linux)
	$(RM) $(OBJDIR) $(TARGET_EXEC) $(wildcard $(SHADERDIR)/*.spv)
else
	if exist $(subst /,\\,$(OBJDIR)) $(RM) $(subst /,\\,$(OBJDIR))
	if exist $(TARGET_EXEC) del /q $(TARGET_EXEC) $(TARGET).ilk $(TARGET).pdb
	if exist $(subst /,\\,$(SHADERDIR))\\*.spv del /q $(subst /,\\,$(SHADERDIR))\\*.spv
endif

.SECONDARY: $(OBJECTS)