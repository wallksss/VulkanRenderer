# Makefile for Vulkan Project
# Compatible with Linux (g++) and Windows (MSV_C cl.exe)

# =============================================================================
#                            USER CONFIGURATION
# =============================================================================
# !!! EDIT THIS PATH TO MATCH YOUR SYSTEM INSTALLATION !!!
# For Windows
VULKAN_SDK_PATH = C:/VulkanSDK/1.4.313.2
# For Linux (these are common defaults)
LINUX_INCDIR_GLOBAL = /usr/include
LINUX_LIBDIR_GLOBAL = /usr/lib
# !!! END OF USER CONFIGURATION !!!

# =============================================================================
#                            SHARED CONFIGURATION
# =============================================================================
TARGET = VulkanTest
SRCDIR = src
INCDIR = libs
OBJDIR = obj
SHADERDIR = shaders
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
SHADERS_SRC_AUTO = $(wildcard $(SHADERDIR)/*.vert) $(wildcard $(SHADERDIR)/*.frag)
SHADERS_SPV_AUTO = $(patsubst %.vert,%.spv,$(SHADERS_SRC_AUTO))
SHADERS_SPV_AUTO := $(patsubst %.frag,%.spv,$(SHADERS_SPV_AUTO))

# =============================================================================
#                       OS-SPECIFIC CONFIGURATION
# =============================================================================
# Default to Windows, then check for Linux
OS_NAME = Windows
ifeq ($(OS), Windows_NT)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(findstring Linux,$(UNAME_S)),Linux)
        OS_NAME = Linux
    endif
endif

# --- Configurações para Linux (g++) ---
ifeq ($(OS_NAME),Linux)
    CXX = g++
    GLSL_COMPILER = glslc
    CXXFLAGS = -std=c++17 -O2 -g
    INCLUDES = -I$(LINUX_INCDIR_GLOBAL)/glm -I$(LINUX_INCDIR_GLOBAL) -I$(INCDIR)
    LDFLAGS = -L$(LINUX_LIBDIR_GLOBAL)
    LIBS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
    OBJ_EXT = .o
    TARGET_EXEC = $(TARGET)
    RM = rm -rf
    RUN_PREFIX = ./

# --- Configurações para Windows (cl.exe) ---
else
    CXX = cl
    GLSL_COMPILER = "$(VULKAN_SDK_PATH)/Bin/glslc.exe"
    CXXFLAGS = /nologo /MD /std:c++17 /O2 /EHsc /D_CRT_SECURE_NO_WARNINGS
    WIN_INCLUDES = /I"$(VULKAN_SDK_PATH)/Include" /I"C:/libs/glm" /I"C:/libs" /I"$(INCDIR)"
    LIBS = vulkan-1.lib glfw3.lib user32.lib gdi32.lib shell32.lib
    WIN_LINK_OPTS = /LIBPATH:"$(VULKAN_SDK_PATH)/Lib" /LIBPATH:"C:/libs/glfw/lib-vc2022"
    OBJ_EXT = .obj
    TARGET_EXEC = $(TARGET).exe
    RM = rmdir /s /q
    RUN_PREFIX =
endif

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%$(OBJ_EXT),$(SOURCES))

# =============================================================================
#                                 BUILD RULES
# =============================================================================
.PHONY: all clean run shaders compile-named-shaders shader-asm

all: shaders $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJECTS)
	@echo LD   $@
ifeq ($(OS_NAME),Linux)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
else
	$(CXX) $(CXXFLAGS) /Fe$@ $^ $(LIBS) /link $(WIN_LINK_OPTS)
endif

# Regra para compilar cada arquivo .cpp para um .obj/.o
$(OBJDIR)/%$(OBJ_EXT): $(SRCDIR)/%.cpp
ifeq ($(OS_NAME),Linux)
	@mkdir -p $(@D)
	@echo CXX  $<
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
else
	@if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
	@echo CXX  $<
	$(CXX) $(CXXFLAGS) $(WIN_INCLUDES) /c $< /Fo$@
endif

# --- Regras para Shaders ---
shaders: $(SHADERDIR)/vert.spv $(SHADERDIR)/frag.spv

$(SHADERDIR)/vert.spv: $(SHADERDIR)/shader.vert
	@echo GLSL $<
	$(GLSL_COMPILER) $< -o $@

$(SHADERDIR)/frag.spv: $(SHADERDIR)/shader.frag
	@echo GLSL $<
	$(GLSL_COMPILER) $< -o $@

# =============================================================================
#                          UTILITY TARGETS
# =============================================================================
compile-named-shaders:
	@echo "Compiling shaders with specific output names..."
	$(GLSL_COMPILER) $(SHADERDIR)/shader.vert -o $(SHADERDIR)/vert.spv
	$(GLSL_COMPILER) $(SHADERDIR)/shader.frag -o $(SHADERDIR)/frag.spv
	@echo "Created $(SHADERDIR)/vert.spv and $(SHADERDIR)/frag.spv"

shader-asm:
	@echo ""
	@echo "--- Vertex Shader Assembly (shader.vert) ---"
	$(GLSL_COMPILER) $(SHADERDIR)/shader.vert -o - -S
	@echo ""
	@echo "--- Fragment Shader Assembly (shader.frag) ---"
	$(GLSL_COMPILER) $(SHADERDIR)/shader.frag -o - -S
	@echo ""

run: all
	@echo Running $(TARGET_EXEC)...
	$(RUN_PREFIX)$(TARGET_EXEC)

clean:
	@echo "Cleaning up..."
ifeq ($(OS_NAME),Linux)
	$(RM) $(OBJDIR) $(TARGET_EXEC) $(wildcard $(SHADERDIR)/*.spv)
else
	if exist $(subst /,\,$(OBJDIR)) $(RM) $(subst /,\,$(OBJDIR))
	if exist $(TARGET_EXEC) del /q $(TARGET_EXEC) $(TARGET).ilk $(TARGET).pdb
	if exist $(subst /,\,$(SHADERDIR))\*.spv del /q $(subst /,\,$(SHADERDIR))\*.spv
endif

.SECONDARY: $(OBJECTS)
