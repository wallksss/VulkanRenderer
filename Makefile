# Nome do executável
TARGET = VulkanTest

# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -g
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Diretórios
SRCDIR = ./src
INCDIR = ./libs
OBJDIR = obj
SHADERDIR = shaders

# Lista de arquivos fonte C++
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Gera a lista de arquivos objeto (.o) na pasta obj/
# Ex: main.cpp -> obj/main.o
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Lista de arquivos de shader GLSL
SHADERS_SRC = $(wildcard $(SHADERDIR)/*.vert) $(wildcard $(SHADERDIR)/*.frag)
# Gera a lista de arquivos .spv (shaders compilados)
SHADERS_SPV = $(patsubst %.vert,%.spv,$(SHADERS_SRC))
SHADERS_SPV := $(patsubst %.frag,%.spv,$(SHADERS_SPV))


# Regra principal (default)
all: $(TARGET)

# Regra para linkar o executável final
$(TARGET): $(OBJECTS)
	@echo "LD   $@"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Regra para compilar cada arquivo .cpp em um arquivo .o
# Isso cria a pasta obj/ se ela não existir
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@echo "CXX  $<"
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Regra para compilar os shaders
# Os shaders .spv dependem de seus arquivos fonte .vert ou .frag
%.spv: %.vert
	@echo "GLSL $<"
	glslc $< -o $@

%.spv: %.frag
	@echo "GLSL $<"
	glslc $< -o $@

# Phony targets não representam arquivos
.PHONY: all clean run shaders

# Dependência dos shaders para a regra 'all' (opcional, mas bom)
# Isso garante que os shaders sejam compilados antes do programa
all: shaders

shaders: $(SHADERS_SPV)

# Regra para executar o programa
run: all
	./$(TARGET)

# Regra para limpar os arquivos gerados
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR) $(TARGET) $(SHADERDIR)/*.spv

# Impede que o make delete arquivos intermediários
.SECONDARY: $(OBJECTS)
