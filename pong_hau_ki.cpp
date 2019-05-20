// Inclui o header padrão.
#include <bits/stdc++.h>

// Inclui a GLEW.
#include <GL/glew.h>

// Inclui a GLFW.
#include <GLFW/glfw3.h>

// Inclui a GLM.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Inclui a AntTweakBar.
#include <AntTweakBar.h>

// Inclui headers comuns dos tutoriais OpenGL.
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

using namespace glm;
using namespace std;

// Tamanho da janela.
float width = 1366.0;
float height = 720.0;

// Definição da cor de fundo do jogo.
vec3 backgroundColor = vec3(0.1, 0.1, 0.1);

GLFWwindow* window;

int main() {
	// Inicializa a GLFW.
	if (!glfwInit()) {
		fprintf(stderr, "Falha ao inicializar a GLFW!\n");
		getchar();
		
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Abre a janela e cria seu contexto OpenGL.
	window = glfwCreateWindow(width, height, "Pong Hau K'i", NULL, NULL);
	
	if (window == NULL) {
		fprintf(stderr, "Falha ao abrir a janela GLFW! Se você tem uma GPU Intel, elas não são compatíveis com a versão 3.3. Tente a versão 2.1 dos tutoriais.\n");
		getchar();
		glfwTerminate();
		
		return -1;
	}
	
	glfwMakeContextCurrent(window);

	// Necessário para o perfil do núcleo.
	glewExperimental = true;
	
	// Inicializa a GLEW.
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Falha ao inicializar a GLEW!\n");
		getchar();
		glfwTerminate();

		return -1;
	}

	// Garante que seja possível capturar o ESC sendo pressionado.
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Esconde o mouse.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, width / 2, height / 2);

	// Cor do fundo.
	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 0);

	// Ativa o teste de profundidade.
	glEnable(GL_DEPTH_TEST);
	// Aceita fragmento se estiver mais perto da câmera do que o anterior.
	glDepthFunc(GL_LESS); 

	// Abate triângulos baseado nas coordenadas da janela.
	glEnable(GL_CULL_FACE);
	// Ativa a multiamostragem, para melhorar a qualidade das bordas.
	glEnable(GL_MULTISAMPLE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Cria e compila o programa GLSL dos shaders.
	GLuint ProgramID = LoadShaders("shader.vertexshader", "shader.fragmentshader");

	// Manuseia os uniforms relacionados ao "MVP".
	GLuint MatrixID = glGetUniformLocation(ProgramID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(ProgramID, "View");
	GLuint ModelMatrixID = glGetUniformLocation(ProgramID, "Model");
	
	// Manuseia o uniform "Texture".
	GLuint TextureID = glGetUniformLocation(ProgramID, "Texture");

	// Manuseia os uniforms de luzes.
	GLuint LeftLightID = glGetUniformLocation(ProgramID, "LeftLight");
	GLuint RightLightID = glGetUniformLocation(ProgramID, "RightLight");

	// Manusei os uniforms das luzes.
	GLuint LightColorID = glGetUniformLocation(ProgramID, "LightColor");
	GLuint LightPowerID = glGetUniformLocation(ProgramID, "LightPower");
	GLuint AmbientColorID = glGetUniformLocation(ProgramID, "AmbientColor");

	// Lista os arquivos dos objetos.
	vector<vector<string>> objects = {
		{"objects/board.obj", "textures/wood.dds"},
		{"objects/env.obj", "textures/env.dds"},
		{"objects/name.obj", "textures/black.dds"},
		{"objects/piece1.obj", "textures/animal.dds"},
		{"objects/piece2.obj", "textures/person.dds"},
		{"objects/piece3.obj", "textures/person.dds"},
		{"objects/piece4.obj", "textures/animal.dds"},
		{"objects/table.obj", "textures/table.dds"},
		{"objects/text.obj", "textures/black.dds"},
	};

	vector<string> textures;

	// Obtém as texturas do vetor de objetos e os adiciona no vetor de texturas.
	for (vector<string> &i : objects) {
		textures.push_back(i.at(1));
	}

	// Ordena as texturas em ordem crescente.
	sort(textures.begin(), textures.end());
	// Remove as duplicatas.
	textures.erase(unique(textures.begin(), textures.end()), textures.end());

	map<string, GLuint> Textures;

	// Este mapa de texturas contém as strings obtidas anteriormente e
	// o GLuint da imagem correspondente a tal string, carregada pela função loadDDS.
	for (string &i : textures) {
		Textures.insert(make_pair(i, loadDDS(&*i.begin())));
	}

	// Cria os vetores de vértices, UVs e normals para cada objeto separadamente.
	vector<vector<vec3>> vertices(objects.size());
	vector<vector<vec2>> uvs(objects.size());
	vector<vector<vec3>> normals(objects.size());
	
	// Cria os buffers de vertex e UV para cada objeto.
	vector<GLuint> vertexbuffers(objects.size());
	vector<GLuint> uvbuffers(objects.size());
	vector<GLuint> normalbuffers(objects.size());

	// Carrega todos os objetos nos seus devidos vetores e buffers.
	for (int i = 0; i < objects.size(); i++) {
		// Carrega os arquivos .obj.
		loadOBJ(&objects[i][0][0], vertices[i], uvs[i], normals[i]);
		
		glGenBuffers(1, &vertexbuffers[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(vec3), &vertices[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffers[i]);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, uvs[i].size() * sizeof(vec2), &uvs[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &normalbuffers[i]);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, normals[i].size() * sizeof(vec3), &normals[i][0], GL_STATIC_DRAW);
	}
	
	// Matrizes do programa.
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
	mat4 MVP;

	// Posições das luzes.
	vec3 LeftLightPos = vec3(-28, 20, 21);
	vec3 RightLightPos = vec3(-30, 20, -17);
	
	// Propriedades das luzes.
	float LightColor[3] = {1, 1, 1};
	float LightPower = 800;
	vec3 AmbientColor = vec3(0.6, 0.6, 0.6);

	// Para calcular a velocidade.
	float deltaTime, currentTime, lastTime = glfwGetTime();
	float timer = 0;
 
	do {
		// Mede a velocidade de processamento.
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Limpa a tela.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Usa os shaders.
		glUseProgram(ProgramID);
		
		// Matrizes principais configuradas para a tela inicial.
		ProjectionMatrix = perspective(radians(35.0f), 16.0f / 9.0f, 0.01f, 100.0f);
		ViewMatrix = lookAt(vec3(-7, 1.1, 0), vec3(0, -2.85, 0), vec3(0, 1, 0));
		ModelMatrix = mat4(1.0);

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Para cada objeto, atribui uma textura e mapeamento UV e desenha o objeto.
		for (int i = 0; i < objects.size(); i++) {
			if (objects[i][0] == "objects/text.obj" && (int) (timer * 2) % 2 != 0) {
				continue;
			}

			// 1º atributo do buffer: vértices.
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// 2º atributo do buffer: UVs.
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffers[i]);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// 3º atributo do buffer: normals.
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffers[i]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// Envia as transformações para o shader vinculado no momento.
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			
			// Envia a posição das luzes para o shader.
			glUniform3f(LeftLightID, LeftLightPos.x, LeftLightPos.y, LeftLightPos.z);
			glUniform3f(RightLightID, RightLightPos.x, RightLightPos.y, RightLightPos.z);

			// Envia as propriedades das luzes para o shader.
			glUniform3f(LightColorID, LightColor[0], LightColor[1], LightColor[2]);
			glUniform1f(LightPowerID, LightPower);
			glUniform3f(AmbientColorID, AmbientColor.x, AmbientColor.y, AmbientColor.z);

			// Vincula a textura na Texture Unit 0.
			glActiveTexture(GL_TEXTURE0);
			
			// Assume a textura.
			glBindTexture(GL_TEXTURE_2D, Textures.find(objects[i][1])->second);
			
			// Define a amostra "Texture" para usar a Texture Unit 0.
			glUniform1i(TextureID, 0);

			// Desenha os triângulos do objeto.
			glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
		}

		// Desabilita os arrays de atributos.
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		
		// Troca os buffers.
		glfwSwapBuffers(window);
		glfwPollEvents();

		timer += deltaTime;
	} while (
		(glfwGetKey(window, GLFW_KEY_ENTER) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_ENTER) != GLFW_PRESS) // ENTER
		&& (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) // ESC
	);

	// Alterna a exibição das opções.
	int showSettings = 0;
	// Alterna a exibição do ambiente de fundo.
	bool showEnvironment = true;
	// Flag para não aceitar nenhuma ação externa durante a tela inicial.
	bool startScreen = true;
	// Ativa a animação da câmera.
	bool cameraRotate = false;
	// A rotação inicial da câmera quando em rotação.
	float rotation = 0.0;
	// Tempo de uma volta da câmera no modo de rotação, em segundos.
	float rotationTime = 4;
	// A distância em X = 17 e Z = 7 até o ponto de origem no mundo.
	float cameraDistance = hypotf(17, 7);
	// Tempo que a câmera leva para sair da tela inicial, em segundos.
	float startToFinalCamTime = 1;
	// Distância focal da câmera.
	float focalLength = 35;
	// Posição da câmera na tela de início.
	vec3 startCamPosition = vec3(-7, 1.1, 0);
	// Look vector da câmera na tela de início.
	vec3 startCamLookVector = vec3(0, -2.85, 0);
	// Serve para suavizar a movimentação da câmera ao pressionar ENTER na tela inicial.
	vec3 camPositionSofter = vec3(0, 0, 0);
	// Posições possíveis da câmera.
	vec3 viewPositions[4][3] = {
		{startCamPosition, startCamLookVector, vec3(0, 1, 0)}, // 0
		{vec3(-cameraDistance, 3, 0), vec3(0, 0, 0), vec3(0, 1, 0)}, // 1
		{vec3(cameraDistance, 3, 0), vec3(0, 0, 0), vec3(0, 1, 0)}, // 3
		{vec3(0, cameraDistance + 10, 0), vec3(0, 0, 0), vec3(-1, 1, 0)}, // 7
	};

	// Salva o tempo de quando a animação foi alternada pela última vez.
	float rotateStartTime;
	// Flag que permite ou não pressionar uma tecla.
	// Usada para as teclas E e R.
	bool allowButton = true;

	// Inicializa a GUI.
	TwInit(TW_OPENGL_CORE, NULL);
	
	// Informa para a GUI o tamanho da janela GLFW.
	TwWindowSize(width, height);
	
	// Cria uma nova GUI.
	TwBar *Settings = TwNewBar("Configuracoes");

	// Define alguns parâmetros da GUI.
	TwDefine("Configuracoes color='0 0 0' fontresizable=false iconified=true movable=false position='16 16' refresh=0.05 resizable=false size='249 259' valueswidth=84");
	TwDefine("TW_HELP visible=false");

	// Define tipos de dados enumerados para serem usados na GUI como dropdowns.
	typedef enum {WIDE, FRONT, BACK, TOP} Viewmode;
	typedef enum {PERSPECTIVE, ORTHOGONAL} Projectionmode;

	// Esta variável indica o modo de visão da câmera.
	Viewmode viewMode = WIDE;

	// Esta variável indica o modo de projeção da câmera.
	Projectionmode projectionMode = PERSPECTIVE;

	// Atribui labels aos valores dos dropdowns.
	TwEnumVal viewModes[] = {
		{WIDE, "Ampla"},
		{FRONT, "Frontal"},
		{BACK, "Traseira"},
		{TOP, "Superior"}
	};

	TwEnumVal projectionModes[] = {
		{PERSPECTIVE, "Perspectiva"},
		{ORTHOGONAL, "Ortogonal"}
	};
	
	// Cria tipos de dados específicos para a AntTweakBar;
	TwType viewModeTwType = TwDefineEnum("ViewmodeType", viewModes, 4);
	TwType projectionModeTwType = TwDefineEnum("ProjectionmodeType", projectionModes, 2);

	// Monta a GUI com as informações abaixo.
	// Opções gráficas.
	TwAddVarRW(Settings, "Ambiente de fundo", TW_TYPE_BOOLCPP, &showEnvironment, "group=Graficos");
	// Opções de câmera.
	TwAddVarRW(Settings, "Distancia focal", TW_TYPE_FLOAT, &focalLength, "group=Camera min=1 max=100 step=0.01");
	TwAddVarRW(Settings, "Visao", viewModeTwType, &viewMode, "group=Camera");
	TwAddVarRW(Settings, "Projecao", projectionModeTwType, &projectionMode, "group=Camera");
	TwAddVarRW(Settings, "Animacao", TW_TYPE_BOOLCPP, &cameraRotate, "group=Camera");
	// Opções de luz.
	TwAddVarRW(Settings, "Cor", TW_TYPE_COLOR3F, &LightColor, "group=Iluminacao opened=true");
	TwAddVarRW(Settings, "Brilho", TW_TYPE_FLOAT, &LightPower, "group=Iluminacao min=0 max=10000 step=10");
 
	// Define os callbacks de eventos da GLFW.
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun) TwEventMouseButtonGLFW);
	glfwSetCursorPosCallback(window, (GLFWcursorposfun) TwEventMousePosGLFW);
 
	// Para calcular a velocidade.
	lastTime = glfwGetTime();
	timer = 0;
	
	do {
		// Mede a velocidade de processamento.
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Limpa a tela.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Usa os shaders.
		glUseProgram(ProgramID);

		// Escolhe os modos de projeção.
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !startScreen) { // P
			projectionMode = PERSPECTIVE;
		} else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !startScreen) { // O
			projectionMode = ORTHOGONAL;
		}
		
		// Escolhe as câmeras ou inicia a animação dela.
		if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS && !startScreen) { // 0
			viewMode = WIDE;
			cameraRotate = false;
		} else if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS && !startScreen) { // 1
			viewMode = FRONT;
			cameraRotate = false;
		} else if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS && !startScreen) { // 3
			viewMode = BACK;
			cameraRotate = false;
		} else if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS && !startScreen) { // 7
			viewMode = TOP;
			cameraRotate = false;
		} else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !startScreen) { // E
			// Verifica se é possível alternar a exibição do ambiente de fundo.
			if (allowButton) {
				// Inverte o status da variável.
				showEnvironment = !showEnvironment;
				
				// Ao pressionar a tecla, bloqueia esta ação até soltá-la.
				allowButton = false;
			}

			// Se a tecla foi solta, desbloqueia esta ação.
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
				allowButton = true;
			}

			if (showEnvironment) {
				LightPower = 800;
				AmbientColor = vec3(0.6, 0.6, 0.6);
			} else {
				LightPower = 1000;
				AmbientColor = backgroundColor;
			}
		} else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !startScreen) { // R
			// Verifica se é possível alternar a animação.
			if (allowButton) {
				// Inverte o status da animação.
				cameraRotate = !cameraRotate;
				
				// Ao pressionar a tecla, bloqueia esta ação até soltá-la.
				allowButton = false;

				// Se a animação foi ativada, reseta a rotação.
				if (cameraRotate) {
					rotation = 0;
				}
			}

			// Se a tecla foi solta, desbloqueia esta ação.
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
				allowButton = true;
			}
		} else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !startScreen) { // S
			// Verifica se é possível alternar a exibição das configurações.
			if (allowButton) {
				// Obtém o estado atual da janela de configurações.
				TwGetParam(Settings, NULL, "iconified", TW_PARAM_INT32, 1, &showSettings);

				// Inverte o status da variável.
				showSettings = !showSettings;
				
				// Ao pressionar a tecla, bloqueia esta ação até soltá-la.
				allowButton = false;

				// Define o parâmetro "iconified" da janela de configurações para o estado atual da variável.
				TwSetParam(Settings, NULL, "iconified", TW_PARAM_INT32, 1, &showSettings);
			}

			// Se a tecla foi solta, desbloqueia esta ação.
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
				allowButton = true;
			}
		}
		
		// Define a projeção de acordo com a escolha anterior.
		ProjectionMatrix = projectionMode == PERSPECTIVE ?
			perspective(radians(focalLength), 16.0f / 9.0f, 0.01f, 100.0f) :
			ortho(-10.0f, 10.0f, (float) -90 / 16, (float) 90 / 16, 0.1f, 100.0f);
		
		// Define a posição da câmera, até mesmo quando a animação dela for escolhida.
		if (cameraRotate) {
			ViewMatrix = lookAt(
				vec3(
					sinf(rotation) * (cameraDistance / 1.5) + 3.0,
					6 - (2 * sinf(rotation)),
					cosf(rotation) * (cameraDistance / 1.5) - 3.0
				),
				vec3(0, 0, 0),
				vec3(0, 1, 0)
			);

			rotation -= (2 * M_PI / rotationTime) * deltaTime;
		} else {
			ViewMatrix = lookAt(
				viewPositions[viewMode][0],
				viewPositions[viewMode][1],
				viewPositions[viewMode][2]
			);
		}

		// Matriz do modelo. Uma matriz identidade.
		ModelMatrix = mat4(1.0);

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		if (startScreen) {
			vec3 finalCamPosition = vec3(-17, 4, -7);
			vec3 finalCamLookVector = vec3(0, 0, 0);

			float speed = deltaTime / startToFinalCamTime;
			float stepRatio = abs(sinf((M_PI / startToFinalCamTime) * timer));

			viewPositions[0][0] = vec3(
				viewPositions[0][0].x + ((speed * (finalCamPosition.x - startCamPosition.x)) * stepRatio),
				viewPositions[0][0].y + ((speed * (finalCamPosition.y - startCamPosition.y)) * stepRatio),
				viewPositions[0][0].z + ((speed * (finalCamPosition.z - startCamPosition.z)) * stepRatio)
			);

			viewPositions[0][1] = vec3(
				viewPositions[0][1].x + ((speed * (finalCamLookVector.x - startCamLookVector.x)) * stepRatio),
				viewPositions[0][1].y + ((speed * (finalCamLookVector.y - startCamLookVector.y)) * stepRatio),
				viewPositions[0][1].z + ((speed * (finalCamLookVector.z - startCamLookVector.z)) * stepRatio)
			);

			timer += deltaTime;

			if (timer >= startToFinalCamTime) {
				// Agora já não está mais na tela inicial.
				// Portanto são aceitas ações externas (botões sendo pressionados).
				startScreen = false;
			}
		}

		// Para cada objeto, atribui uma textura e mapeamento UV e desenha o objeto.
		for (int i = 0; i < objects.size(); i++) {
			if (objects[i][0] == "objects/text.obj") {
				continue;
			}

			if (objects[i][0] == "objects/env.obj" && !showEnvironment) {
				continue;
			}

			// 1º atributo do buffer: vértices.
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// 2º atributo do buffer: UVs.
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffers[i]);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// 3º atributo do buffer: normals.
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffers[i]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

			// Envia a transformação para o shader vinculado no momento.
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			
			// Envia a posição das luzes para o shader.
			glUniform3f(LeftLightID, LeftLightPos.x, LeftLightPos.y, LeftLightPos.z);
			glUniform3f(RightLightID, RightLightPos.x, RightLightPos.y, RightLightPos.z);

			// Envia as propriedades das luzes para o shader.
			glUniform3f(LightColorID, LightColor[0], LightColor[1], LightColor[2]);
			glUniform1f(LightPowerID, LightPower);
			glUniform3f(AmbientColorID, AmbientColor.x, AmbientColor.y, AmbientColor.z);

			// Vincula a textura na Texture Unit 0.
			glActiveTexture(GL_TEXTURE0);
			
			// Assume a textura.
			glBindTexture(GL_TEXTURE_2D, Textures.find(objects[i][1])->second);
			
			// Define a amostra "Texture" para usar a Texture Unit 0.
			glUniform1i(TextureID, 0);

			// Desenha os triângulos do objeto.
			glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
		}

		// Desabilita os arrays de atributos.
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Desenha o GUI.
		if (!startScreen) TwDraw();
		
		// Troca os buffers.
		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0); // ESC

	// Limpa todos os vetores e os buffers.
	for (GLuint &i : vertexbuffers) glDeleteBuffers(1, &i);
	for (GLuint &i : uvbuffers) glDeleteBuffers(1, &i);
	for (GLuint &i : normalbuffers) glDeleteBuffers(1, &i);

	for (string &i : textures) glDeleteTextures(1, &Textures.find(i)->second);

	glDeleteProgram(ProgramID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Fecha a janela do OpenGL e finaliza a GUI e a GLFW.
	TwTerminate();
	glfwTerminate();

	return 0;
}