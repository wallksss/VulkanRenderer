# Makefile for Vulkan Project
# Compatible with Linux (g++) and Windows (MSV_C cl.exe)

# =============================================================================
#                            USER CONFIGURATION
# =============================================================================
# !!! EDIT THIS PATH TO MATCH YOUR SYSTEM INSTALLATION !!!
VULKAN_SDK_PATH = C:/VulkanSDK/1.4.313.2
# !!! END OF USER CONFIGURATION !!!

# =============================================================================
#                            SHARED CONFIGURATION
# =============================================================================
TARGET = VulkanTest
SRCDIR = src
INCDIR = libs
INCDIR_GLOBAL = C:/libs
OBJDIR = obj
SHADERDIR = shaders
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
# Regra padrão para compilar todos os shaders .vert e .frag para .spv com o mesmo nome base
SHADERS_SRC_AUTO = $(wildcard $(SHADERDIR)/*.vert) $(wildcard $(SHADERDIR)/*.frag)
SHADERS_SPV_AUTO = $(patsubst %.vert,%.spv,$(SHADERS_SRC_AUTO))
SHADERS_SPV_AUTO := $(patsubst %.frag,%.spv,$(SHADERS_SPV_AUTO))

# =============================================================================
#                       OS-SPECIFIC CONFIGURATION
# =============================================================================
OS_NAME = Windows_NT
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
    INCLUDES = -I$(INCDIR_GLOBAL)/glm -I$(INCDIR_GLOBAL) -I$(INCDIR)
    LDFLAGS = -L$(INCDIR_GLOBAL)/GLFW/lib # Ajuste se necessário
    LIBS = -lglfw3 -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
    OBJ_EXT = .o
    TARGET_EXEC = $(TARGET)
    RM = rm -rf
    MKDIR_P = mkdir -p
    RUN_PREFIX = ./

# --- Configurações para Windows (cl.exe) ---
else
    CXX = cl
    GLSL_COMPILER = "$(VULKAN_SDK_PATH)/Bin/glslc.exe"
    # Adicionada a flag /MD para garantir consistência da C-Runtime Library
    CXXFLAGS = /nologo /MD /std:c++17 /O2 /EHsc /D_CRT_SECURE_NO_WARNINGS
    
    # Caminhos de include para todas as bibliotecas: Vulkan, GLM (global), GLFW (global) e libs locais
    WIN_INCLUDES = /I"$(VULKAN_SDK_PATH)/Include" /I"$(INCDIR_GLOBAL)/glm" /I"$(INCDIR_GLOBAL)" /I"$(INCDIR)"

    # Bibliotecas a serem linkadas
    LIBS = vulkan-1.lib glfw3.lib user32.lib gdi32.lib shell32.lib
    
    # Caminhos para as bibliotecas .lib
    # !!! ATENÇÃO: Confirme o caminho para o seu glfw3.lib !!!
    # Este exemplo assume C:/libs/GLFW/lib-vc2022
    WIN_LINK_OPTS = /LIBPATH:"$(VULKAN_SDK_PATH)/Lib" /LIBPATH:"$(INCDIR_GLOBAL)"

    OBJ_EXT = .obj
    TARGET_EXEC = $(TARGET).exe
    RM = rmdir /s /q
    MKDIR_P = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
    RUN_PREFIX =

endif

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%$(OBJ_EXT),$(SOURCES))

# =============================================================================
#                                 BUILD RULES
# =============================================================================
# Lista de alvos que não são arquivos
.PHONY: all clean run shaders compile-named-shaders shader-asm

# Alvo principal: compila os shaders automaticamente e depois o programa
all: shaders $(TARGET_EXEC)

# Regra de linkagem para criar o executável final
$(TARGET_EXEC): $(OBJECTS)
	@echo LD   $@
ifeq ($(OS_NAME),Linux)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
else
	$(CXX) $(CXXFLAGS) /Fe$@ $^ $(LIBS) /link $(WIN_LINK_OPTS)
endif

# Regra para compilar cada arquivo .cpp para um .obj/.o
$(OBJDIR)/%$(OBJ_EXT): $(SRCDIR)/%.cpp
	@$(call MKDIR_P,$(OBJDIR))
	@echo CXX  $<
ifeq ($(OS_NAME),Linux)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(WIN_INCLUDES) /c $< /Fo$@
endif

# --- Regras para Shaders ---

# Regra para compilar automaticamente todos os shaders para a build principal
shaders: $(SHADERS_SPV_AUTO)

%.spv: %.vert
	@echo GLSL $<
	$(GLSL_COMPILER) $< -o $@

%.spv: %.frag
	@echo GLSL $<
	$(GLSL_COMPILER) $< -o $@

# =============================================================================
#                          NOVAS TAREFAS ADICIONADAS
# =============================================================================

# Compila shader.vert para vert.spv e shader.frag para frag.spv
compile-named-shaders:
	@echo "Compiling shaders with specific output names..."
	$(GLSL_COMPILER) $(SHADERDIR)/shader.vert -o $(SHADERDIR)/vert.spv
	$(GLSL_COMPILER) $(SHADERDIR)/shader.frag -o $(SHADERDIR)/frag.spv
	@echo "Created $(SHADERDIR)/vert.spv and $(SHADERDIR)/frag.spv"

# Mostra o assembly (SPIR-V legível) dos shaders no terminal
shader-asm:
	@echo ""
	@echo "--- Vertex Shader Assembly (shader.vert) ---"
	$(GLSL_COMPILER) $(SHADERDIR)/shader.vert -o - -S
	@echo ""
	@echo "--- Fragment Shader Assembly (shader.frag) ---"
	$(GLSL_COMPILER) $(SHADERDIR)/shader.frag -o - -S
	@echo ""

# =============================================================================
#                                 UTILIDADES
# =============================================================================
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