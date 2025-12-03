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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// 객체 구조체
struct Shape {
	std::vector<float> vertices;
	std::vector<unsigned int> index;
	std::vector<float> colors;
	std::vector<float> normals;
	std::vector<float> texcoords;
	GLuint VAO, VBO[4], EBO;
};
Shape line; Shape cube; Shape corn;
bool showcube = true; bool rotate = false;
float angle = 0.0f; glm::vec3 axis(0.0f, 1.0f, 0.0f);
glm::mat4 rmat(1.0f);

// 카메라
struct Camera {
	glm::vec3 eye;
	glm::vec3 at;
	glm::vec3 up;
} camera = { glm::vec3(-1.0f, 1.0f, 3.0f),
			 glm::vec3(0.0f, 0.0f, 0.0f),
			 glm::vec3(0.0f, 1.0f, 0.0f) };

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
void CreateCube(Shape& cube, float x, float y, float z);
void CreateCorn(Shape& corn);
void CreateCorrdinate();
void CreateOrbit(Shape& orbit, float radius, int segments);
GLuint loadTexture(const char* path);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint textureID; //--- 텍스처 객체
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

// 메뉴
void menu() {
	std::cout << "c: 육면체 그리기" << std::endl;
	std::cout << "p: 사각뿔 그리기" << std::endl;
	std::cout << "x/X: x축 중심 회전" << std::endl;
	std::cout << "y/Y: y축 중심 회전" << std::endl;
	std::cout << "s: 초기화" << std::endl;
	std::cout << "q: 프로그램 종료" << std::endl;
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Example25");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();
	//--- 콜백 함수 등록
	glEnable(GL_DEPTH_TEST);
	CreateCube(cube, 0.5f, 0.5f, 0.5f);
	CreateCorn(corn);
	CreateCorrdinate();
	menu();
	glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력
	glutMainLoop();
}

// 삼각뿔 생성 함수
void CreateCorn(Shape& corn) {
	corn.vertices = {
		// 바닥면
		-0.5f, -0.5f, -0.5f,  // 0
		0.5f, -0.5f, -0.5f,   // 1
		-0.5f, -0.5f, 0.5f,   // 2
		0.5f, -0.5f, 0.5f,    // 3
		// 앞면
		0.0f, 0.5f, 0.0f,     // 4
		0.5f, -0.5f, 0.5f,    // 5
		-0.5f, -0.5f, 0.5f,   // 6
		// 오른면
		0.0f, 0.5f, 0.0f,     // 7
		0.5f, -0.5f, -0.5f,   // 8
		0.5f, -0.5f, 0.5f,    // 9
		// 뒷면
		0.0f, 0.5f, 0.0f,     // 10
		-0.5f, -0.5f, -0.5f,  // 11
		0.5f, -0.5f, -0.5f,   // 12
		// 왼면
		0.0f, 0.5f, 0.0f,     // 13
		-0.5f, -0.5f, 0.5f,   // 14
		-0.5f, -0.5f, -0.5f,  // 15
	};
	corn.index = {
		// 바닥면
		0, 1, 2,
		1, 2, 3,
		// 앞면
		4, 5, 6,
		// 오른면
		7, 8, 9,
		// 뒷면
		10, 11, 12,
		// 왼면
		13, 14, 15
	};
	corn.normals = {
		// 바닥면 - 아래쪽
		0.0f, -1.0f, 0.0f,  // 0
		0.0f, -1.0f, 0.0f,  // 1
		0.0f, -1.0f, 0.0f,  // 2
		0.0f, -1.0f, 0.0f,  // 3
		// 앞면 - 앞쪽
		0.0f, 0.447f, 0.894f,  // 4
		0.0f, 0.447f, 0.894f,  // 5
		0.0f, 0.447f, 0.894f,  // 6
		// 오른면 - 오른쪽
		0.894f, 0.447f, 0.0f,  // 7
		0.894f, 0.447f, 0.0f,  // 8
		0.894f, 0.447f, 0.0f,  // 9
		// 뒷면 - 뒤쪽
		0.0f, 0.447f, -0.894f, // 10
		0.0f, 0.447f, -0.894f, // 11
		0.0f, 0.447f, -0.894f, // 12
		// 왼면 - 왼쪽
		-0.894f, 0.447f, 0.0f, // 13
		-0.894f, 0.447f, 0.0f, // 14
		-0.894f, 0.447f, 0.0f  // 15
	};
	corn.colors = {
		// 바닥면 - 마젠타
		1.0f, 0.0f, 1.0f,  // 0
		1.0f, 0.0f, 1.0f,  // 1
		1.0f, 0.0f, 1.0f,  // 2
		1.0f, 0.0f, 1.0f,  // 3
		// 앞면 - 노란색
		1.0f, 1.0f, 0.0f,  // 4
		1.0f, 1.0f, 0.0f,  // 5
		1.0f, 1.0f, 0.0f,  // 6
		// 오른면 - 초록색
		0.0f, 1.0f, 0.0f,  // 7
		0.0f, 1.0f, 0.0f,  // 8
		0.0f, 1.0f, 0.0f,  // 9
		// 뒷면 - 파란색
		0.0f, 0.0f, 1.0f,  // 10
		0.0f, 0.0f, 1.0f,  // 11
		0.0f, 0.0f, 1.0f,  // 12
		// 왼면 - 빨강색
		1.0f, 0.0f, 0.0f,  // 13
		1.0f, 0.0f, 0.0f,  // 14
		1.0f, 0.0f, 0.0f   // 15
	};
	// 텍스처 좌표 추가
	corn.texcoords = {
		// 바닥면
		0.0f, 0.0f,   // 0
		1.0f, 0.0f,   // 1
		0.0f, 1.0f,   // 2
		1.0f, 1.0f,   // 3
		// 앞면
		0.5f, 1.0f,   // 4 (피라미드 꼭짓점)
		1.0f, 0.0f,   // 5
		0.0f, 0.0f,   // 6
		// 오른면
		0.5f, 1.0f,   // 7 (피라미드 꼭짓점)
		0.0f, 0.0f,   // 8
		1.0f, 0.0f,   // 9
		// 뒷면
		0.5f, 1.0f,   // 10 (피라미드 꼭짓점)
		1.0f, 0.0f,   // 11
		0.0f, 0.0f,   // 12
		// 왼면
		0.5f, 1.0f,   // 13 (피라미드 꼭짓점)
		0.0f, 0.0f,   // 14
		1.0f, 0.0f    // 15
	};
	InitBuffers(corn);
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
	const float w = 1.0f / 3.0f; // 가로 1칸 크기 (약 0.333)
	const float h = 0.5f;        // 세로 1칸 크기
	cube.texcoords = {
		// 1. 앞면 (이미지 1번: 좌측 상단) -> U: 0~1/3, V: 0.5~1.0
		0.0f, 1.0f,     // 0: 좌상
		0.0f, 0.5f,     // 1: 좌하
		w,    0.5f,     // 2: 우하
		w,    1.0f,     // 3: 우상

		// 2. 뒷면 (이미지 2번: 중앙 상단) -> U: 1/3~2/3, V: 0.5~1.0
		w,    0.5f,     // 4: 좌하 (뒷면 기준)
		w,    1.0f,     // 5: 좌상
		w * 2,  1.0f,     // 6: 우상
		w * 2,  0.5f,     // 7: 우하

		// 3. 윗면 (이미지 3번: 우측 상단) -> U: 2/3~1.0, V: 0.5~1.0
		w * 2,  1.0f,     // 8: 좌상
		w * 2,  0.5f,     // 9: 좌하
		1.0f, 0.5f,     // 10: 우하
		1.0f, 1.0f,     // 11: 우상

		// 4. 아래면 (이미지 4번: 좌측 하단) -> U: 0~1/3, V: 0.0~0.5
		0.0f, 0.5f,     // 12: 좌상
		0.0f, 0.0f,     // 13: 좌하
		w,    0.0f,     // 14: 우하
		w,    0.5f,     // 15: 우상

		// 5. 왼면 (이미지 5번: 중앙 하단) -> U: 1/3~2/3, V: 0.0~0.5
		w,    0.5f,     // 16: 좌상
		w,    0.0f,     // 17: 좌하
		w * 2,  0.0f,     // 18: 우하
		w * 2,  0.5f,     // 19: 우상

		// 6. 오른면 (이미지 6번: 우측 하단) -> U: 2/3~1.0, V: 0.0~0.5
		w * 2,  0.5f,     // 20: 좌상
		w * 2,  0.0f,     // 21: 좌하
		1.0f, 0.0f,     // 22: 우하
		1.0f, 0.5f      // 23: 우상
	};
	cube.colors = {
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
	};
	InitBuffers(cube);
}

// 좌표축 그리기
void CreateCorrdinate() {
	line.vertices = {
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f
	};
	line.index = {
		0, 1,
		2, 3,
		4, 5
	};
	line.colors = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	InitBuffers(line);
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
	glGenVertexArrays(1, &shape.VAO);      // 버텍스 배열 객체id 생성
	glBindVertexArray(shape.VAO);      // 버텍스 배열 객체 바인딩

	glGenBuffers(4, shape.VBO);          // 버퍼id 4개 생성

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
		glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[2]);	// 법선
		glBufferData(GL_ARRAY_BUFFER, shape.normals.size() * sizeof(float), shape.normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
	}
	
	// 텍스처 로딩 - 한 번만 실행
	if (textureID == 0) {
		textureID = loadTexture("img.png");
	}
	
	// 텍스처 좌표 버퍼 (texcoords가 비어있지 않은 경우에만)
	if (!shape.texcoords.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[3]);	// 텍스처 좌표
		glBufferData(GL_ARRAY_BUFFER, shape.texcoords.size() * sizeof(float), shape.texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);
	}
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'c':
		showcube = true;
		break;
	case 'p':
		showcube = false;
		break;
	case 'x':
		rotate = true;
		angle = 2.0f;
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'X':
		rotate = true;
		angle = -2.0f;
		axis = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'y':
		rotate = true;
		angle = 2.0f;
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case 'Y':
		rotate = true;
		angle = -2.0f;
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case 's':
		rotate = false;
		rmat = glm::mat4(1.0f);
		angle = 0.0f;
		axis = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders() {
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("29_vertex.glsl");
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
void make_fragmentShaders() {
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("29_fragment.glsl"); // 프래그세이더 읽어오기
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

// --- 텍스처 로딩 함수 ---
GLuint loadTexture(const char* path) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// 텍스처 파라미터 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 이미지 로드
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // OpenGL은 Y축이 뒤집어져 있음
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		std::cout << "Texture loaded successfully: " << path << " (" << width << "x" << height << ", channels: " << nrChannels << ")" << std::endl;
	}
	else {
		std::cout << "Failed to load texture: " << path << std::endl;
	}

	stbi_image_free(data);
	return texture;
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram() {
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
	// 배경을 흰색으로 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");

	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLocation = glGetUniformLocation(shaderProgramID, "projection");
	
	// 조명 설정
	glUniform3f(lightPosLocation, 0.0f, 10.0f, 5.0f);
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);

	// 텍스처 설정
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(glGetUniformLocation(shaderProgramID, "utexture"), 0);

	// 카메라 위치
	glUniform3f(viewPosLocation, camera.eye.x, camera.eye.y, camera.eye.z);
	// 뷰 행렬
	glm::mat4 view = glm::lookAt(camera.eye, camera.at, camera.up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	// 투영 행렬
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));
	// 좌표축 그리기
	glm::mat4 corMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
	glBindVertexArray(line.VAO);
	glDrawArrays(GL_LINES, 0, 6);
	
	// 도형 그리기
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = rmat * matrix;
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	if (showcube) {
		glBindVertexArray(cube.VAO);
		glDrawElements(GL_TRIANGLES, cube.index.size(), GL_UNSIGNED_INT, 0);
	}
	else {
		glBindVertexArray(corn.VAO);
		glDrawElements(GL_TRIANGLES, corn.index.size(), GL_UNSIGNED_INT, 0);
	}

	glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

// 타이머 콜백 함수
void TimerFunction(int value) {
	if (rotate) {
		rmat = glm::rotate(rmat, glm::radians(angle), axis);
	}
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}