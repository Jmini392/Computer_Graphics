#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// 객체 구조체
struct Shape {
	std::vector<float> vertices;
	std::vector<unsigned int> index;
	std::vector<float> colors;
	std::vector<float> normals;
	GLuint VAO, VBO[3], EBO; // VBO 배열 크기를 3으로 변경 (normals 포함)
};
Shape sub_body; Shape mid_body; Shape ground;  
Shape top_body[2]; Shape cannon[2]; Shape flagpole[2];
Shape lightbox;
glm::vec3 tankPosition = glm::vec3(0.0f);
glm::mat4 middle_rotation_matrix = glm::mat4(1.0f);
glm::mat4 cannon_rotation_matrix = glm::mat4(1.0f);
glm::mat4 flagpole_rotation_matrix = glm::mat4(1.0f);
float trans = 0.0f; int trans_cnt = 0; int cnt = 0; int flag = 1;
bool lightRotate = false; bool lightOn = true;
float angle = 0.0f;
float lightAngle = 90.0f;
float lightDistance = 4.0f;
glm::vec3 lightPosition(0.0f, 0.0f, lightDistance);
int color = 0;


// 카메라
struct Camera {
	glm::vec3 eye;
	glm::vec3 at;
	glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.5f, 3.0f),
			 glm::vec3(0.0f, 0.0f, 0.0f),
			 glm::vec3(0.0f, 1.0f, 0.0f) };
float cangle; float cos_angle; float sin_angle;
float new_eye_x; float new_eye_z; glm::vec3 direction;
bool camera_rotate = false;


char* filetobuf(const char* file) {
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

//--- 사용자 정의 함수 선언
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void CreateCube(Shape& cube, float x, float height, float y);
void CreateGround(Shape& ground, float width, float length);  
void menu();
void set_tank_color(); 

//--- 필요한 변수 선언
GLint width = 800, height = 600;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example27"); // 윈도우 제목 수정
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();
	//--- 콜백 함수 등록
	menu();
	CreateCube(sub_body, 1.0f, 0.3f, 0.4f);
	CreateCube(mid_body, 0.5f, 0.15f, 0.3f);
	for (int i = 0; i < 2; i++) {
		CreateCube(top_body[i], 0.3f, 0.2f, 0.4f);
		CreateCube(cannon[i], 0.05f, 0.05f, 0.4f);
		CreateCube(flagpole[i], 0.05f, 0.4f, 0.05f);
	}
	CreateGround(ground, 5.0f, 5.0f);
	CreateCube(lightbox, 0.2f, 0.2f, 0.2f);
	set_tank_color(); // 색상 설정 호출
	glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력
	glutMainLoop();
}
// 메뉴
void menu() {
	std::cout << "m: 조명켜기/끄기" << std::endl;
	std::cout << "c: 조명 색상 변경" << std::endl;
	std::cout << "y/Y: 조명을 객체의 중심 y축에 대하여 양 / 음 방향으로 공전시키기" << std::endl;
	std::cout << "s: 회전 멈추기" << std::endl;
	std::cout << "z/Z: 카메라 z축 이동" << std::endl;
	std::cout << "x/X: 카메라 x축 이동" << std::endl;
	std::cout << "r/R: 카메라 y축 공전" << std::endl;
	std::cout << "q: 종료" << std::endl;
}

// 바닥 생성 함수
void CreateGround(Shape& ground, float width, float length) {
	ground.vertices = {
		// 바닥면 (y = 0 평면에서 큰 사각형)
		-width, 0.0f, -length,  // 앞왼쪽
		-width, 0.0f, length,   // 뒤왼쪽
		width, 0.0f, length,    // 뒤오른쪽
		width, 0.0f, -length,   // 앞오른쪽
	};
	
	ground.index = {
		0, 1, 2,  // 첫 번째 삼각형
		0, 2, 3   // 두 번째 삼각형
	};
	
	// 바닥 법선 벡터 추가 (위쪽을 향함)
	ground.normals = {
		0.0f, 1.0f, 0.0f,  // 첫 번째 정점 법선
		0.0f, 1.0f, 0.0f,  // 두 번째 정점 법선
		0.0f, 1.0f, 0.0f,  // 세 번째 정점 법선
		0.0f, 1.0f, 0.0f   // 네 번째 정점 법선
	};
	
	InitBuffers(ground);
}

// 큐브 생성 함수
void CreateCube(Shape& cube, float x, float y, float z) {
	cube.vertices = {
		// 앞면
	   -x, y, z,
	   -x, -y, z,
	   x, -y, z,
	   x, y, z,

	   // 뒷면
	   -x, -y, -z,
	   -x, y, -z,
	   x, y, -z,
	   x, -y, -z,

	   // 윗면
	   -x, y, -z,
	   -x, y, z,
	   x, y, z,
	   x, y, -z,

	   // 아래면 
	   -x, -y, z,
	   -x, -y, -z,
	   x, -y, -z,
	   x, -y, z,

	   // 왼면 
	   -x, y, -z,
	   -x, -y, -z,
	   -x, -y, z,
	   -x, y, z,

	   // 오른면
	   x, y, z,
	   x, -y, z,
	   x, -y, -z,
	   x, y, -z
	};
	cube.index = {
		// 앞면
		0, 1, 2, 0, 2, 3,
		// 뒷면
		4, 5, 6, 4, 6, 7,
		// 아래면
		8, 9, 10, 8, 10, 11,
		// 위면
		12, 13, 14, 12, 14, 15,
		// 오른면
		16, 17, 18, 16, 18, 19,
		// 왼면
		20, 21, 22, 20, 22, 23
	};
	cube.normals = {
		// 앞면
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		// 뒷면
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		// 윗면
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		// 아랫면
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		// 왼면
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		// 오른면
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};
	cube.colors = {
		// 모든 면을 밝은 노란색으로 설정 (조명 효과)
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
		1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f
	};
	InitBuffers(cube);
}

// 탱크 색깔 설정 함수
void set_tank_color() {
	sub_body.colors.clear();
	mid_body.colors.clear();
	ground.colors.clear();
	top_body[0].colors.clear();
	top_body[1].colors.clear();
	cannon[0].colors.clear();
	cannon[1].colors.clear();
	flagpole[0].colors.clear();
	flagpole[1].colors.clear();
	
	// 각 부품별 색상 설정
	for (size_t i = 0; i < sub_body.vertices.size() / 3; i++) {
		sub_body.colors.push_back(0.5f); // R
		sub_body.colors.push_back(0.5f); // G
		sub_body.colors.push_back(0.5f); // B
	}
	
	for (size_t i = 0; i < mid_body.vertices.size() / 3; i++) {
		mid_body.colors.push_back(0.8f); // R
		mid_body.colors.push_back(0.8f); // G
		mid_body.colors.push_back(0.8f); // B
	}
	
	for (size_t i = 0; i < top_body[0].vertices.size() / 3; i++) {
		top_body[0].colors.push_back(0.0f); // R
		top_body[0].colors.push_back(0.8f); // G
		top_body[0].colors.push_back(0.0f); // B
		top_body[1].colors.push_back(0.0f); // R
		top_body[1].colors.push_back(0.8f); // G
		top_body[1].colors.push_back(0.0f); // B
	}
	
	for (size_t i = 0; i < cannon[0].vertices.size() / 3; i++) {
		cannon[0].colors.push_back(1.0f); // R
		cannon[0].colors.push_back(1.0f); // G
		cannon[0].colors.push_back(0.0f); // B
		cannon[1].colors.push_back(1.0f); // R
		cannon[1].colors.push_back(1.0f); // G
		cannon[1].colors.push_back(0.0f); // B
	}
	
	for (size_t i = 0; i < flagpole[0].vertices.size() / 3; i++) {
		flagpole[0].colors.push_back(1.0f); // R
		flagpole[0].colors.push_back(0.7f); // G
		flagpole[0].colors.push_back(0.0f); // B
		flagpole[1].colors.push_back(1.0f); // R
		flagpole[1].colors.push_back(0.7f); // G
		flagpole[1].colors.push_back(0.0f); // B
	}
	
	for (size_t j = 0; j < ground.vertices.size() / 3; j++) {
		ground.colors.push_back(0.3f); // R
		ground.colors.push_back(0.9f); // G
		ground.colors.push_back(0.3f); // B
	}
	
	// 버퍼 업데이트
	InitBuffers(sub_body);
	InitBuffers(mid_body);
	InitBuffers(ground);
	for (int k = 0; k < 2; k++) {
		InitBuffers(top_body[k]);
		InitBuffers(cannon[k]);
		InitBuffers(flagpole[k]);
	}
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
	glGenVertexArrays(1, &shape.VAO);      // 버텍스 배열 객체id 생성
	glBindVertexArray(shape.VAO);      // 버텍스 배열 객체 바인딩

	glGenBuffers(3, shape.VBO);          // 버퍼id 생성

	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // 좌표
	glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &shape.EBO);                 // 인덱스 버퍼id 생성
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // 색상
	glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// 법선 버퍼 (normals가 비어있지 않은 경우에만)
	if (!shape.normals.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, shape.normals.size() * sizeof(float), shape.normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
	}
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("25_vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("25_fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
	GLuint shaderID;
	GLint result;
	GLchar errorLog[512];
	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
	//--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
	//--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
	//--- 사용하기 직전에 호출할 수 있다.
	return shaderID;
}

//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(shaderProgramID);

	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");

	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");

	// 조명 위치를 동적으로 계산
	lightPosition.x = lightDistance * cos(glm::radians(lightAngle));
	lightPosition.z = lightDistance * sin(glm::radians(lightAngle));
	lightPosition.y = 0.0f;

	// 조명 설정
	glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	if (lightOn) {
		if (color == 0) glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
		else if (color == 1) glUniform3f(lightColorLocation, 1.0, 0.0, 0.0); // 빨강
		else if (color == 2) glUniform3f(lightColorLocation, 0.0, 0.0, 1.0); // 파랑
		else if (color == 3) glUniform3f(lightColorLocation, 0.0, 1.0, 0.0); // 초록
	}
	else glUniform3f(lightColorLocation, 0.0, 0.0, 0.0);

	// 투영 행렬 설정
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// 뷰 행렬 설정
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(camera.eye, camera.at, camera.up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	// 조명상자 그리기
	glm::mat4 lightboxMatrix = glm::mat4(1.0f);
	lightboxMatrix = glm::translate(lightboxMatrix, lightPosition);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(lightboxMatrix));
	glBindVertexArray(lightbox.VAO);
	glDrawElements(GL_TRIANGLES, lightbox.index.size(), GL_UNSIGNED_INT, 0);

	// 바닥 그리기 (탱크보다 먼저 그려서 아래에 위치)
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // 탱크 아래로 위치
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(ground.VAO);
	glDrawElements(GL_TRIANGLES, ground.index.size(), GL_UNSIGNED_INT, 0);

	// 탱크 그리기
	// 아래 몸체
	model = glm::mat4(1.0f);
	model = glm::translate(model, tankPosition);
	model = glm::translate(model, glm::vec3(0.0f, -0.45f, 0.0f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(sub_body.VAO);
	glDrawElements(GL_TRIANGLES, sub_body.index.size(), GL_UNSIGNED_INT, 0);

	// 중간 몸체
	model = glm::mat4(1.0f);
	model = glm::translate(model, tankPosition);
	model = middle_rotation_matrix * model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(mid_body.VAO);
	glDrawElements(GL_TRIANGLES, mid_body.index.size(), GL_UNSIGNED_INT, 0);

	for (int i = 0; i < 2; i++) {
		// 왼쪽, 오른쪽 배치
		float x;
		if (i == 0) {
			if (flag == 1) x = -0.5f + trans;
			else x = 0.5f - trans;
		}
		else {
			if (flag == 1) x = 0.5f - trans;
			else x = -0.5f + trans;
		}

		// 상부 몸체
		model = glm::mat4(1.0f);
		model = glm::translate(model, tankPosition);
		model = middle_rotation_matrix * model;
		model = glm::translate(model, glm::vec3(x, 0.35f, 0.0f));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(top_body[i].VAO);
		glDrawElements(GL_TRIANGLES, top_body[i].index.size(), GL_UNSIGNED_INT, 0);

		// 포신
		model = glm::mat4(1.0f);
		model = glm::translate(model, tankPosition);
		model = glm::translate(model, glm::vec3(x, 0.35f, 0.0f));
		model = middle_rotation_matrix * model;

		glm::mat4 individual_cannon_rotation = glm::mat4(1.0f);
		if (x < 0) individual_cannon_rotation = cannon_rotation_matrix;
		else individual_cannon_rotation = glm::inverse(cannon_rotation_matrix);
		model = model * individual_cannon_rotation;

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.4f));

		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cannon[i].VAO);
		glDrawElements(GL_TRIANGLES, cannon[i].index.size(), GL_UNSIGNED_INT, 0);

		// 깃대
		model = glm::mat4(1.0f);
		model = glm::translate(model, tankPosition);
		model = glm::translate(model, glm::vec3(x, 0.65f, 0.0f));
		model = middle_rotation_matrix * model;

		glm::mat4 individual_flagpole_rotation = glm::mat4(1.0f);
		if (i == 0) individual_flagpole_rotation = flagpole_rotation_matrix;
		else individual_flagpole_rotation = glm::inverse(flagpole_rotation_matrix);
		glm::mat4 translate_down = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.2f, 0.0f));
		model = inverse(translate_down) * model * individual_flagpole_rotation * translate_down;

		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(flagpole[i].VAO);
		glDrawElements(GL_TRIANGLES, flagpole[i].index.size(), GL_UNSIGNED_INT, 0);
	}

	glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'z':
		camera.eye.z += 0.1f;
		break;
	case 'Z':
		camera.eye.z -= 0.1f;
		break;
	case 'x':
		camera.eye.x += 0.1f;
		camera.at.x += 0.1f;
		break;
	case 'X':
		camera.eye.x -= 0.1f;
		camera.at.x -= 0.1f;
		break;
	case 'r':
		cangle = glm::radians(5.0f);
		cos_angle = cos(cangle);
		sin_angle = sin(cangle);

		new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
		new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

		camera.eye.x = new_eye_x;
		camera.eye.z = new_eye_z;
		break;
	case 'R':
		cangle = glm::radians(-5.0f);
		cos_angle = cos(cangle);
		sin_angle = sin(cangle);

		new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
		new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

		camera.eye.x = new_eye_x;
		camera.eye.z = new_eye_z;
		break;
	case 'm':
		lightOn = !lightOn;
		break;
	case 'c':
		color++;
		if (color > 3) color = 0;
		break;
	case 'y':
		lightRotate = true;
		angle = 2.0f;
		break;
	case 'Y':
		lightRotate = true;
		angle = -2.0f;
		break;
	case 's':
		lightRotate = false;
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

// 타이머 콜백 함수
void TimerFunction(int value) {
	if (lightRotate) {
		lightAngle += angle;
		if (lightAngle >= 360.0f) lightAngle = 0.0f;
		if (lightAngle < 0.0f) lightAngle = 360.0f;
	}
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}