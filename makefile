CMAKE_COMMAND = /usr/bin/cmake
CMAKE_SOURCE_DIR = /home/zorn/Trabalhos/Engenharia de Computação/Matérias/Computação Gráfica II/Exercícios/OpenGL
CMAKE_BINARY_DIR = $(CMAKE_SOURCE_DIR)/build

$(VERBOSE).SILENT:

all: pong_hau_ki.cpp
	$(CMAKE_COMMAND) -E cmake_progress_start "$(CMAKE_BINARY_DIR)/CMakeFiles" "$(CMAKE_BINARY_DIR)/CMakeFiles/progress.marks"
	$(MAKE) -C "$(CMAKE_BINARY_DIR)" -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start "$(CMAKE_BINARY_DIR)/CMakeFiles" 0
	./trabalho_2.1