// Inclui o header padrão.
#include <bits/stdc++.h>

// Inclui a GLEW.
#include <GL/glew.h>

// Inclui a GLFW.
#include <GLFW/glfw3.h>

// Inclui a GLM.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
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

/**
 * Obtém as coordenadas da tela e transforma para o espaço do mundo.
 * 
 * double mouseX: posição do cursor em X, da esquerda para a direita.
 * double mouseY: posição do cursor em Y, de baixo para cima.
 * mat4 ViewMatrix: posição da câmera e orientação.
 * mat4 ProjectionMatrix: parâmetros da câmera, a projeção.
 * vec3& outOrigin: saída. Origem do raio. Inicia no near plane, mas caso queira que o raio inicie na posição da câmera, ignore.
 * vec3& outDirection: saída. Direção, no espaço do mundo, do raio parte do cursor.
 */
void screenPositionToWorldRay(double mouseX, double mouseY, mat4 ViewMatrix, mat4 ProjectionMatrix, vec3 &outOrigin, vec3 &outDirection) {
	// Posições de início e fim do raio, com as coordenadas do dispositivo normalizado.
	vec4 lRayStart_NDC = vec4(
		(mouseX / width - 0.5f) * 2.0f, // [0, 1366] -> [-1,1].
		-(mouseY / height - 0.5f) * 2.0f, // [0, 768] -> [-1,1].
		-1.0f, // O near plane é mapeado para Z = -1 nas coordenadas do dispositivo normalizado.
		1.0f
	);

	vec4 lRayEnd_NDC = vec4(
		(mouseX / width - 0.5f) * 2.0f,
		-(mouseY / height - 0.5f) * 2.0f,
		0.0f,
		1.0f
	);

	// A ProjectionMatrix vai do espaço da câmera para as coordenadas do dispositivo normalizado.
	// Portanto, inverse(ProjectionMatrix) faz o caminho contrário.
	mat4 InverseProjectionMatrix = inverse(ProjectionMatrix);
	
	// A ViewMatrix vai do espaço do mundo para o espaço da câmera.
	// Portanto, inverse(ViewMatrix) faz o caminho contrário.
	mat4 InverseViewMatrix = inverse(ViewMatrix);
	
	vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;
	lRayStart_camera /= lRayStart_camera.w;
	
	vec4 lRayStart_world = InverseViewMatrix * lRayStart_camera;
	lRayStart_world /= lRayStart_world.w;
	
	vec4 lRayEnd_camera = InverseProjectionMatrix * lRayEnd_NDC;
	lRayEnd_camera /= lRayEnd_camera.w;
	
	vec4 lRayEnd_world = InverseViewMatrix * lRayEnd_camera;
	lRayEnd_world /= lRayEnd_world.w;
	
	// Faster way (just one inverse)
	/*
	mat4 M = inverse(ProjectionMatrix * ViewMatrix);

	vec4 lRayStart_world = M * lRayStart_NDC;
	lRayStart_world /= lRayStart_world.w;
	
	vec4 lRayEnd_world = M * lRayEnd_NDC;
	lRayEnd_world /= lRayEnd_world.w;
	*/
	
	vec3 lRayDir_world = vec3(lRayEnd_world - lRayStart_world);
	lRayDir_world = normalize(lRayDir_world);

	outOrigin = vec3(lRayStart_world);
	outDirection = normalize(lRayDir_world);
}

/**
 * Testa se houve intersecção com algum Bouding Box Orientado.
 * 
 * vec3 rayOrigin: origem do raio, espaço do mundo.
 * vec3 rayDirection: direção do raio (e não a posição do alvo), no espaço do mundo. Deve ser normalizada.
 * vec3 aabb_min: coordenadas X, Y e Z mínimas da malha quando não transformada.
 * vec3 aabb_max: coordenadas X, Y e Z máximas. Muitas vezes, equivale a aabb_min * -1, se a malha está centralizada, mas nem sempre é caso.
 * mat4 ModelMatrix: Transformação aplicada à malha (que será, assim, também aplicada ao seu bounding box).
 * float &intersection_distance: saída. Distância entre o rayOrigin e a intersecção com o OBB.
 */
bool testRayOBBIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 aabb_min, vec3 aabb_max, mat4 ModelMatrix, float &intersection_distance) {
	float e, f;

	float tMin = 0.0;
	float tMax = 100000.0;

	vec3 OBBposition_worldspace = vec3(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	vec3 delta = OBBposition_worldspace - rayOrigin;

	// Testa a intersecção com os dois planos perpendiculares com o eixo X do OBB.
	vec3 xaxis = vec3(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);

	e = dot(xaxis, delta);
	f = dot(rayDirection, xaxis);

	// Caso padrão.
	if (fabs(f) > 0.001) {
		// Intersecção com o plano "esquerdo".
		float t1 = (e + aabb_min.x) / f;
		
		// Intersecção com o plano "direito".
		float t2 = (e + aabb_max.x) / f;
		
		// t1 e t2 agora contém as distâncias entre a origem do raio e as intersecções raio-plano.

		// Queremos que o t1 represente a intersecção mais próxima.
		// Então, se este não é o caso, faremos o swap entre t1 e t2.
		if (t1 > t2) {
			float w = t1;
			t1 = t2;
			t2 = w;
		}

		// tMax é a intersecção "distante" mais próxima (entre os pares de plano X, Y e Z).
		if (t2 < tMax) tMax = t2;

		// tMin é a intersecção "próxima" mais distante (entre os pares de plano X, Y e Z).
		if (t1 > tMin) tMin = t1;

		// E aqui está o truque: se o "distante" está mais perto que o "próximo", então não há intersecção.
		if (tMax < tMin) return false;
	// Caso raro: o raio está quase paralelo aos planos, então eles não tem qualquer intersecção.
	} else if (-e + aabb_min.x > 0.0 || -e + aabb_max.x < 0.0) {
		return false;
	}
	
	// Testa a intersecção com os dois planos perpendiculares com o eixo Y do OBB.
	// Exatamente a mesma coisa que acontece acima.
	vec3 yaxis = vec3(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);

	e = dot(yaxis, delta);
	f = dot(rayDirection, yaxis);

	if (fabs(f) > 0.001) {
		float t1 = (e + aabb_min.y) / f;
		float t2 = (e + aabb_max.y) / f;

		if (t1 > t2) {
			float w = t1;
			t1 = t2;
			t2 = w;
		}

		if (t2 < tMax) tMax = t2;

		if (t1 > tMin) tMin = t1;

		if (tMin > tMax) return false;
	} else if (-e + aabb_min.y > 0.0 || -e + aabb_max.y < 0.0) {
		return false;
	}


	// Testa a intersecção com os dois planos perpendiculares com o eixo Z do OBB.
	// Exatamente a mesma coisa que acontece acima.
	vec3 zaxis = vec3(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);

	e = dot(zaxis, delta);
	f = dot(rayDirection, zaxis);

	if (fabs(f) > 0.001) {
		float t1 = (e + aabb_min.z) / f;
		float t2 = (e + aabb_max.z) / f;

		if (t1 > t2) {
			float w = t1;
			t1 = t2;
			t2 = w;
		}

		if (t2 < tMax) tMax = t2;
		
		if (t1 > tMin) tMin = t1;

		if (tMin > tMax) return false;
	} else if (-e + aabb_min.z > 0.0 || -e + aabb_max.z < 0.0) {
		return false;
	}

	intersection_distance = tMin;

	return true;
}

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
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    // Exibe o mouse.
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
		{"objects/piece1.obj", "textures/animal.dds"},
		{"objects/piece2.obj", "textures/person.dds"},
		{"objects/piece3.obj", "textures/person.dds"},
		{"objects/piece4.obj", "textures/animal.dds"},
		{"objects/board.obj", "textures/wood.dds"},
		{"objects/draw.obj", "textures/black.dds"},
		{"objects/env.obj", "textures/env.dds"},
		{"objects/name.obj", "textures/black.dds"},
		{"objects/table.obj", "textures/table.dds"},
		{"objects/text.obj", "textures/black.dds"}
	};

	// Lista os arquivos dos obbs.
	// Indica a relação entre um OBB com uma peça.
	vector<string> obbs = {
		"objects/bb1.obj",
		"objects/bb2.obj",
		"objects/bb3.obj",
		"objects/bb4.obj"
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

	int vectorsSize = objects.size() + obbs.size();

	// Cria os vetores de vértices, UVs e normals para cada objeto separadamente.
	vector<vector<vec3>> vertices(vectorsSize);
	vector<vector<vec2>> uvs(vectorsSize);
	vector<vector<vec3>> normals(vectorsSize);
	
	// Cria os buffers de vertex e UV para cada objeto.
	vector<GLuint> vertexbuffers(vectorsSize);
	vector<GLuint> uvbuffers(vectorsSize);
	vector<GLuint> normalbuffers(vectorsSize);

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

	// Carrega os OBBs nos seus devidos vetores e buffers.
	for (int i = objects.size(); i < vectorsSize; i++) {
		// Carrega os arquivos .obj.
		loadOBJ(&obbs[i - objects.size()][0], vertices[i], uvs[i], normals[i]);
		
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

	// Cria o vetor para os vértices dos bounding boxes.
	vector<vector<vec3>> obbvertices(obbs.size());
	vector<vector<vec2>> obbuvs(obbs.size());
	vector<vector<vec3>> obbnormals(obbs.size());

	// Carrega todos os bounding boxes nos seus devidos vetores.
	for (int i = 0; i < obbs.size(); i++) {
		// Carrega os arquivos .obj.
		loadOBJ(&obbs[i][0], obbvertices[i], obbuvs[i], obbnormals[i]);
	}

	// Vetor que será usado para parametrizar os teste de colisão com o ray-picking.
	vector<vector<vec3>> aabb_obbs;

	// Para cada bounding box, coleta a menor e maior coordenada e adiciona no vetor acima.
	for (vector<vec3> &i : obbvertices) {
		float min_x, min_y, min_z;
		float max_x, max_y, max_z;

		for (vec3 &j : i) {
			if (&j == &i.front() || j.x < min_x) min_x = j.x;
			if (&j == &i.front() || j.x > max_x) max_x = j.x;

			if (&j == &i.front() || j.y < min_y) min_y = j.y;
			if (&j == &i.front() || j.y > max_y) max_y = j.y;

			if (&j == &i.front() || j.z < min_z) min_z = j.z;
			if (&j == &i.front() || j.z > max_z) max_z = j.z;
		}

		vec3 this_aabb_min = vec3(min_x, min_y, min_z);
		vec3 this_aabb_max = vec3(max_x, max_y, max_z);

		vector<vec3> this_aabb = {
			this_aabb_min,
			this_aabb_max
		};
		
		aabb_obbs.push_back(this_aabb);
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
		for (int i = 0; i < vectorsSize; i++) {
			if (i < objects.size()) {
				if (objects[i][0] == "objects/draw.obj") continue;
				if (objects[i][0] == "objects/text.obj" && (int) (timer * 2) % 2 != 0) continue;
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

			if (i < objects.size()) {
				// Vincula a textura na Texture Unit 0.
				glActiveTexture(GL_TEXTURE0);
				
				// Assume a textura.
				glBindTexture(GL_TEXTURE_2D, Textures.find(objects[i][1])->second);
				
				// Define a amostra "Texture" para usar a Texture Unit 0.
				glUniform1i(TextureID, 0);

				// Desenha os triângulos do objeto.
				glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
			}
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

	/**
	 * Variáveis de visualização e lógica do programa.
	 */

	// Flag que permite ou não pressionar uma tecla. Usada para as teclas E e R.
	bool allowButton = true;
	// Ativa a animação da câmera.
	bool cameraRotate = false;
	// Alterna a exibição do ambiente de fundo.
	bool showEnvironment = true;
	// Flag para não aceitar nenhuma ação externa durante a tela inicial.
	bool startScreen = true;

	// Alterna a exibição das opções.
	int showSettings = 0;
	// Alterna a exibição das informações do mouse.
	int showMouseInfo = 0;

	// A distância em X = 17 e Z = 7 até o ponto de origem no mundo.
	float cameraDistance = hypotf(17, 7);
	// Distância focal da câmera.
	float focalLength = 35;
	// A rotação inicial da câmera quando em rotação.
	float rotation = 0.0;
	// Salva o tempo de quando a animação foi alternada pela última vez.
	float rotateStartTime;
	// Tempo de uma volta da câmera no modo de rotação, em segundos.
	float rotationTime = 4;
	// Tempo que a câmera leva para sair da tela inicial, em segundos.
	float startToFinalCamTime = 1;

	// Posição do cursor.
	double mouseX, mouseY;

	// Serve para suavizar a movimentação da câmera ao pressionar ENTER na tela inicial.
	vec3 camPositionSofter = vec3(0, 0, 0);
	// Look vector da câmera na tela de início.
	vec3 startCamLookVector = vec3(0, -2.85, 0);
	// Posição da câmera na tela de início.
	vec3 startCamPosition = vec3(-7, 1.1, 0);

	// Posições possíveis da câmera.
	vec3 viewPositions[4][3] = {
		{startCamPosition, startCamLookVector, vec3(0, 1, 0)}, // 0
		{vec3(-cameraDistance, 3, 0), vec3(0, 0, 0), vec3(0, 1, 0)}, // 1
		{vec3(cameraDistance, 3, 0), vec3(0, 0, 0), vec3(0, 1, 0)}, // 3
		{vec3(0, cameraDistance + 10, 0), vec3(0, 0, 0), vec3(-1, 1, 0)}, // 7
	};

	/**
	 * Variáveis da lógica de jogo.
	 */

	// Peça movida.
	string movedPiece;

	// Contador de jogadas. Conta de forma decrescente.
	int movesLeft = 50;

	// Coordenadas de cada casa do tabuleiro.
	vector<vec3> spotsCoords = {
		vec3(-2.81413, 0.2, -2.74802),
		vec3(2.90069, 0.2, -2.82106),
		vec3(2.9605, 0.2, 2.98074),
		vec3(-2.87974, 0.2, 2.85907),
		vec3(0.001157, 0.2, 0.023826)
	};
	
	// Matriz de adjacências das casas do tabuleiro, que indica as relações de uma com as outras.
	int spotsAdjMat[spotsCoords.size()][spotsCoords.size()] = {
		/******0**1**2**3**4*/
		/*0*/ {0, 1, 0, 1, 1},
		/*1*/ {1, 0, 1, 0, 1},
		/*2*/ {0, 1, 0, 0, 1},
		/*3*/ {1, 0, 0, 0, 1},
		/*4*/ {1, 1, 1, 1, 0}
	};

	/**
	 * Posições iniciais das peças.
	 */
	vector<int> randInitPos;

	// Define posições aleatórias para as peças, onde cada posição é uma casa e estas foram definidas anteriormente.
	int obbsSize = obbs.size();

	srand(time(NULL));

	for (int i = 0; i < obbsSize; i++) {
		int randPos = rand() % (obbsSize + 1);

		while (find(randInitPos.begin(), randInitPos.end(), randPos) != randInitPos.end()) {
			randPos = rand() % (obbsSize + 1);
		}

		randInitPos.push_back(randPos);
	}

	vector<int> piecesSpots(obbsSize);
	vector<quat> piecesQuats(obbsSize);

	piecesSpots[0] = 0;
	piecesSpots[1] = 1;
	piecesSpots[2] = 3;
	piecesSpots[3] = 2;

	for (int i = 0; i < obbsSize; i++) {
		piecesQuats[i] = quat(vec3(0, rand() % 360, 0));
	}
	
	float intersection_distance;
	
	vec3 rayOrigin;
	vec3 rayDirection;

	/**
	 * Implementação da GUI.
	 */

	// Inicializa a GUI.
	TwInit(TW_OPENGL_CORE, NULL);
	
	// Informa para a GUI o tamanho da janela GLFW.
	TwWindowSize(width, height);
	
	// Cria uma nova GUI.
	TwBar *Settings = TwNewBar("Configuracoes");
	TwBar *MouseInfo = TwNewBar("Informacoes do mouse");

	// Define alguns parâmetros da GUI.
	TwDefine("Configuracoes color='0 0 0' fontresizable=false iconified=true movable=false position='16 16' refresh=0.05 resizable=false size='249 259' valueswidth=84");
	TwDefine("'Informacoes do mouse' color='0 0 0' fontresizable=false iconified=true movable=false position='971 16' refresh=0.01 resizable=false size='379 339' valueswidth=144");
	TwDefine("TW_HELP visible=false");

	// Define tipos de dados enumerados para serem usados na GUI como dropdowns.
	typedef enum {PERSPECTIVE, ORTHOGONAL} Projectionmode;
	typedef enum {WIDE, FRONT, BACK, TOP} Viewmode;

	// Esta variável indica o modo de projeção da câmera.
	Projectionmode projectionMode = PERSPECTIVE;

	// Esta variável indica o modo de visão da câmera.
	Viewmode viewMode = WIDE;

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

	// Monta a GUI de Configurações com os dados abaixo.
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

	// Monta a GUI com informações do mouse com os dados abaixo.
	// Peça selecionada.
	TwAddVarRO(MouseInfo, "Peca movida", TW_TYPE_STDSTRING, &movedPiece, NULL);
	// Cursor.
	TwAddVarRO(MouseInfo, "Posicao X", TW_TYPE_DOUBLE, &mouseX, "group=Cursor");
	TwAddVarRO(MouseInfo, "Posicao Y", TW_TYPE_DOUBLE, &mouseY, "group=Cursor");
	// Ray Casting.
	TwAddVarRO(MouseInfo, "Direcao do raio", TW_TYPE_DIR3F, &rayDirection, "group='Ray Picking' opened=true");
 
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

		// Verifica a posição do cursor.
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// A seleção do objetos é feita aqui!
		// Ao invés de selecionar a cada frame em que o botão do mouse está pressionado,
		// verifica-se apenas quando o ele é solto.
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !startScreen) {
			if (allowButton) {
				// Faz as conversões das coordenadas do cursor do mouse na tela para o mundo.
				screenPositionToWorldRay(mouseX, mouseY, ViewMatrix, ProjectionMatrix, rayOrigin, rayDirection);

				// Testa cada OBB.
				// Uma engine de física pode ser mais eficiente que isto,
				// porque ela já possui algumas estruturas de particionamento espacial,
				// como uma árvore binária de particionamento espacial (BSP-Tree),
				// hierarquia de volume delimitador (BVH) ou outros.
				for (int i = 0; i < obbs.size(); i++) {
					vec3 aabb_min = aabb_obbs[i][0];
					vec3 aabb_max = aabb_obbs[i][1];
					
					mat4 RotationMatrix = toMat4(piecesQuats[i]);
					mat4 TranslationMatrix = translate(mat4(), spotsCoords[piecesSpots[i]]);
					
					ModelMatrix = TranslationMatrix * RotationMatrix;

					if (testRayOBBIntersection(rayOrigin, rayDirection, aabb_min, aabb_max, ModelMatrix, intersection_distance)) {
						int spot = piecesSpots[i];
						
						vector<int> neighbors;

						for (int neighbor = 0; neighbor < spotsCoords.size(); neighbor++) {
							if (!spotsAdjMat[spot][neighbor]) {
								continue;
							}

							neighbors.push_back(neighbor);
						}

						// for (int &jk : spotsAdjMat[spot]) {
							// cout << jk << " ";
						// }

						// cout << endl;

						// for (int &neighbor : neighbors) {
							// cout << neighbor << " ";
						// }

						// cout << endl;

						cout << spot << " " << to_string(spotsCoords[spot]) << endl;

						bool canMove = false;

						for (int &neighbor : neighbors) {
							if (find(piecesSpots.begin(), piecesSpots.end(), neighbor) == piecesSpots.end()) {
								canMove = true;

								piecesSpots[i] = neighbor;

								break;
							}
						}

						// if (!canMove) {}

						movedPiece = "Peca " + to_string(i + 1);

						break;
					}
				}

				allowButton = false;
			}

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
				allowButton = true;
			}
		}
		
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
		} else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !startScreen) { // M
			// Verifica se é possível alternar a exibição das informações do mouse.
			if (allowButton) {
				// Obtém o estado atual da janela de informações do mouse.
				TwGetParam(MouseInfo, NULL, "iconified", TW_PARAM_INT32, 1, &showMouseInfo);

				// Inverte o status da variável.
				showMouseInfo = !showMouseInfo;
				
				// Ao pressionar a tecla, bloqueia esta ação até soltá-la.
				allowButton = false;

				// Define o parâmetro "iconified" da janela de informações do mouse para o estado atual da variável.
				TwSetParam(MouseInfo, NULL, "iconified", TW_PARAM_INT32, 1, &showMouseInfo);
			}

			// Se a tecla foi solta, desbloqueia esta ação.
			if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
				allowButton = true;
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
		for (int i = 0; i < vectorsSize; i++) {
			if (i < objects.size()) {
				if (objects[i][0] == "objects/draw.obj" || objects[i][0] == "objects/text.obj") continue;
				if (objects[i][0] == "objects/env.obj" && !showEnvironment) continue;
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

			if (i < obbs.size()) {
				mat4 RotationMatrix = toMat4(piecesQuats[i]);
				mat4 TranslationMatrix = translate(mat4(1.0), spotsCoords[piecesSpots[i]]);

				ModelMatrix = TranslationMatrix * RotationMatrix;
			} else if (i >= objects.size()) {
				mat4 RotationMatrix = toMat4(piecesQuats[i - objects.size()]);
				mat4 TranslationMatrix = translate(mat4(1.0), spotsCoords[piecesSpots[i - objects.size()]]);

				ModelMatrix = TranslationMatrix * RotationMatrix;
			} else {
				ModelMatrix = mat4(1.0);
			}
			
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

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

			if (i < objects.size()) {
				// Vincula a textura na Texture Unit 0.
				glActiveTexture(GL_TEXTURE0);
				
				// Assume a textura.
				glBindTexture(GL_TEXTURE_2D, Textures.find(objects[i][1])->second);
				
				// Define a amostra "Texture" para usar a Texture Unit 0.
				glUniform1i(TextureID, 0);

				// Desenha os triângulos do objeto.
				glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
			}
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