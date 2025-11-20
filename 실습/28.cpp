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

// 작은 구체들을 위한 구조체
struct FallingSphere {
    glm::vec3 position;
    float speed;
    bool active;
};

// 객체 구조체
struct Shape {
	std::vector<float> vertices;
	std::vector<unsigned int> index;
	std::vector<float> colors;
	std::vector<float> normals;
	GLuint VAO, VBO[3], EBO;
};
Shape lightbox; Shape orbit;
Shape corn; Shape ground;
GLUquadric* sphere[3];
GLUquadric* fallingSphereQuadric[30]; // 떨어지는 구체들을 위한 quadric
Shape sorbit[3];

// 떨어지는 구체들 배열 (30개)
FallingSphere fallingSpheres[30];
bool snowAnimation = false; // 눈 애니메이션 상태

// 구체들의 초기 반지름과 회전 각도
float sphereRadii[3] = { 2.0f, 2.0f, 2.0f };
float sphereAngles[3] = { 0.0f, 0.0f, 0.0f };
float sphereSpeed[3] = { 2.0f, 1.5f, 1.0f }; // 각각 다른 속도
glm::vec3 spherePositions[3]; // 동적으로 계산될 위치들
// 구체 색상 배열 (빨강, 초록, 파랑)
glm::vec3 sphereColors[3] = {
	glm::vec3(1.0f, 0.0f, 0.0f),  // 빨강
	glm::vec3(0.0f, 1.0f, 0.0f),  // 초록
	glm::vec3(0.0f, 0.0f, 1.0f)   // 파랑
};
bool lightRotate = false;
float angle = 0.0f;
float lightAngle = 90.0f;
float lightDistance = 4.0f;
float lightIntensity = 1.0f; // 조명 세기 변수 추가
glm::vec3 lightPosition(0.0f, 0.0f, lightDistance);

// 카메라
struct Camera {
	glm::vec3 eye;
	glm::vec3 at;
	glm::vec3 up;
} camera = { glm::vec3(0.0f, 2.0f, 8.0f),
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
void CreateOrbit(Shape& orbit, float radius, int segments);
void CreateCorn(Shape& corn);
void CreateGround(Shape& ground, float width, float length);
void UpdateSpherePositions(); // 구체 위치 업데이트 함수 추가
void InitializeFallingSpheres(); // 떨어지는 구체 초기화 함수
void UpdateFallingSpheres(); // 떨어지는 구체 업데이트 함수

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

// 메뉴
void menu() {
	std::cout << "s: 하늘에서 눈이 내린다 / 멈춘다" << std::endl;
	std::cout << "r : 조명이 화면 중앙의 y축에 대하여 공전한다 / 멈춘다." << std::endl;
	std::cout << "n / f : 조명이 가까워지기 / 멀어지기" << std::endl;
	std::cout << "+ / -: 조명의 세기 높아지기 / 낮춰지기" << std::endl;
	std::cout << "q : 프로그램 종료" << std::endl;
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Example28");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();
	//--- 콜백 함수 등록
	glEnable(GL_DEPTH_TEST);
	CreateCube(lightbox, 0.2f, 0.2f, 0.2f);
	CreateOrbit(orbit, lightDistance, 100);
	for (int i = 0; i < 3; i++) {
		CreateOrbit(sorbit[i], sphereRadii[i], 100);
	}
	CreateCorn(corn); // 사각뿔 생성
	CreateGround(ground, 2.0f, 2.0f);
	UpdateSpherePositions(); // 초기 구체 위치 설정
	InitializeFallingSpheres(); // 떨어지는 구체 초기화
	menu();
	glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력
	glutMainLoop();
}

// 떨어지는 구체 초기화 함수
void InitializeFallingSpheres() {
	std::uniform_real_distribution<float> posRange(-2.0f, 2.0f); // 바닥 범위에 맞춤 (-3.0f에서 -2.0f로 변경)
	std::uniform_real_distribution<float> speedRange(0.02f, 0.08f);
	std::uniform_real_distribution<float> heightRange(3.0f, 8.0f);
	
	for (int i = 0; i < 30; i++) {
		fallingSpheres[i].position.x = posRange(gen);
		fallingSpheres[i].position.y = heightRange(gen);
		fallingSpheres[i].position.z = posRange(gen);
		fallingSpheres[i].speed = speedRange(gen);
		fallingSpheres[i].active = false; // 처음에는 비활성화
		
		// quadric 초기화
		fallingSphereQuadric[i] = gluNewQuadric();
		gluQuadricDrawStyle(fallingSphereQuadric[i], GLU_FILL);
		gluQuadricNormals(fallingSphereQuadric[i], GLU_SMOOTH);
	}
}

// 떨어지는 구체 업데이트 함수
void UpdateFallingSpheres() {
	if (!snowAnimation) return;
	
	std::uniform_real_distribution<float> posRange(-2.0f, 2.0f); // 바닥 범위에 맞춤 (-3.0f에서 -2.0f로 변경)
	std::uniform_real_distribution<float> speedRange(0.02f, 0.08f);
	
	for (int i = 0; i < 30; i++) {
		if (fallingSpheres[i].active) {
			// 구체를 아래로 이동
			fallingSpheres[i].position.y -= fallingSpheres[i].speed;
			
			// 바닥에 닿으면 랜덤한 위치에서 다시 시작
			if (fallingSpheres[i].position.y < -0.5f) {
				fallingSpheres[i].position.x = posRange(gen);
				fallingSpheres[i].position.y = 4.0f;
				fallingSpheres[i].position.z = posRange(gen);
				fallingSpheres[i].speed = speedRange(gen);
			}
		}
	}
}

// 구체 위치 업데이트 함수 - 궤도 회전 적용
void UpdateSpherePositions() {
	for (int i = 0; i < 3; i++) {
		// 기본 원형 궤도상의 위치 계산
		float basicX = sphereRadii[i] * cos(glm::radians(sphereAngles[i]));
		float basicY = 0.0f;
		float basicZ = sphereRadii[i] * sin(glm::radians(sphereAngles[i]));
		
		glm::mat4 transform = glm::mat4(1.0f);
		
		transform = glm::rotate(transform, glm::radians((i + 1) * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
		glm::vec4 position = transform * glm::vec4(basicX, basicY, basicZ, 1.0f);
		
		spherePositions[i] = glm::vec3(position.x, position.y, position.z);
	}
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

	ground.colors = {
		0.6f, 0.6f, 0.6f,
		0.6f, 0.6f, 0.6f,
		0.6f, 0.6f, 0.6f,
		0.6f, 0.6f, 0.6f
	};

	InitBuffers(ground);
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
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
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
	
	if (x < 0.3f) { // lightbox 구분 (크기가 작은 것)
		cube.colors = {
			// 모든 면을 밝은 노란색으로 설정 (조명 효과)
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f,
			1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f
		};
	}
	else {
		cube.colors = {
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
		};
	}
	InitBuffers(cube);
}

// 궤도 생성 함수
void CreateOrbit(Shape& orbit, float radius, int segments) {
	orbit.vertices.clear();
	orbit.index.clear();
	orbit.colors.clear();
	// 원 형태의 정점 생성
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * 3.14159265f * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		orbit.vertices.push_back(x);
		orbit.vertices.push_back(0.0f);
		orbit.vertices.push_back(z);

		// 궤도 색상
		orbit.colors.push_back(1.0f);
		orbit.colors.push_back(1.0f);
		orbit.colors.push_back(1.0f);

		orbit.index.push_back(i);
	}

	InitBuffers(orbit);
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

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 's': // 눈내리기
		snowAnimation = !snowAnimation;
		// 애니메이션이 시작되면 모든 구체를 활성화
		for (int i = 0; i < 30; i++) {
			fallingSpheres[i].active = snowAnimation;
		}
		break;
	case 'r': // 조명 공전
		lightRotate = !lightRotate;
		angle = 2.0f;
		break;
	case 'n': // 조명 가까이
		lightDistance -= 0.1f;
		if (lightDistance < 2.0f) lightDistance = 2.0f;
		CreateOrbit(orbit, lightDistance, 100);
		break;
	case 'f': // 조명 멀리
		lightDistance += 0.1f;
		if (lightDistance > 5.0f) lightDistance = 5.0f;
		CreateOrbit(orbit, lightDistance, 100);
		break;
	case '+': // 조명 세기 증가
		lightIntensity += 0.1f;
		if (lightIntensity > 3.0f) lightIntensity = 3.0f; // 최대값 제한
		break;
	case '-': // 조명 세기 감소
		lightIntensity -= 0.1f;
		if (lightIntensity < 0.3f) lightIntensity = 0.3f; // 최소값 제한
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
void make_fragmentShaders() {
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");

	unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLocation = glGetUniformLocation(shaderProgramID, "projection");
	// 조명 위치를 동적으로 계산
	lightPosition.x = lightDistance * cos(glm::radians(lightAngle));
	lightPosition.z = lightDistance * sin(glm::radians(lightAngle));
	lightPosition.y = 0.0f;

	// 조명 설정 - lightIntensity를 반영
	glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(lightColorLocation, lightIntensity, lightIntensity, lightIntensity);
	// 카메라 위치
	glUniform3f(viewPosLocation, camera.eye.x, camera.eye.y, camera.eye.z);
	// 뷰 행렬
	glm::mat4 view = glm::lookAt(camera.eye, camera.at, camera.up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	// 투영 행렬
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));
	
	// 조명 궤도 그리기
	glm::mat4 corMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
	glBindVertexArray(orbit.VAO);
	glDrawElements(GL_LINE_LOOP, orbit.index.size(), GL_UNSIGNED_INT, 0);
	
	// 조명상자 그리기
	glm::mat4 lightboxMatrix = glm::mat4(1.0f);
	lightboxMatrix = glm::translate(lightboxMatrix, lightPosition);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(lightboxMatrix));
	glBindVertexArray(lightbox.VAO);
	glDrawElements(GL_TRIANGLES, lightbox.index.size(), GL_UNSIGNED_INT, 0);

	// 사각뿔 그리기
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(corn.VAO);
	glDrawElements(GL_TRIANGLES, corn.index.size(), GL_UNSIGNED_INT, 0);

	// 바닥 그리기 (탱크보다 먼저 그려서 아래에 위치)
	glm::mat4 groundmodel = glm::mat4(1.0f);
	groundmodel = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(groundmodel));
	glBindVertexArray(ground.VAO);
	glDrawElements(GL_TRIANGLES, ground.index.size(), GL_UNSIGNED_INT, 0);

	// 회전된 구체 궤도 그리기
	for (int i = 0; i < 3; i++) {
		glm::mat4 sorbitMatrix = glm::mat4(1.0f);
		
		// 각 궤도마다 (i+1) * 45도씩 회전하여 궤도들이 같은 선상에 있지 않도록 함
		sorbitMatrix = glm::rotate(sorbitMatrix, glm::radians((i + 1) * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		// 궤도를 수직으로 세움 (X축 기준 90도 회전)
		sorbitMatrix = glm::rotate(sorbitMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(sorbitMatrix));
		glBindVertexArray(sorbit[i].VAO);
		glDrawElements(GL_LINE_LOOP, sorbit[i].index.size(), GL_UNSIGNED_INT, 0);
	}

	// 구체 그리기 - 각각 다른 색상으로
	for (int i = 0; i < 3; i++) {
		// 셰이더를 비활성화하고 고정 파이프라인 사용
		glUseProgram(0);

		// OpenGL 고정 파이프라인의 조명 및 재질 설정
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		// 조명 설정 - lightIntensity를 반영
		GLfloat light_pos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
		GLfloat light_ambient[] = { 0.2f * lightIntensity, 0.2f * lightIntensity, 0.2f * lightIntensity, 1.0f };
		GLfloat light_diffuse[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };
		GLfloat light_specular[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

		// 카메라 설정
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(camera.eye.x, camera.eye.y, camera.eye.z,
			camera.at.x, camera.at.y, camera.at.z,
			camera.up.x, camera.up.y, camera.up.z);

		// 구체별 재질 색상 설정 (빨강, 초록, 파랑)
		GLfloat material_ambient[] = { 0.2f * sphereColors[i].x, 0.2f * sphereColors[i].y, 0.2f * sphereColors[i].z, 1.0f };
		GLfloat material_diffuse[] = { sphereColors[i].x, sphereColors[i].y, sphereColors[i].z, 1.0f };
		GLfloat material_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat material_shininess[] = { 50.0f };

		glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);

		// 구체 위치 설정 및 그리기
		glPushMatrix();
		glTranslatef(spherePositions[i].x, spherePositions[i].y, spherePositions[i].z);

		sphere[i] = gluNewQuadric();
		gluQuadricDrawStyle(sphere[i], GLU_FILL);
		gluQuadricNormals(sphere[i], GLU_SMOOTH);
		gluSphere(sphere[i], 0.2f, 20, 20);
		gluDeleteQuadric(sphere[i]);

		glPopMatrix();

		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	// 떨어지는 작은 구체들 그리기
	if (snowAnimation) {
		for (int i = 0; i < 30; i++) {
			if (fallingSpheres[i].active) {
				// 셰이더를 비활성화하고 고정 파이프라인 사용
				glUseProgram(0);

				// OpenGL 고정 파이프라인의 조명 및 재질 설정
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);

				// 조명 설정
				GLfloat light_pos[] = { lightPosition.x, lightPosition.y, lightPosition.z, 1.0f };
				GLfloat light_ambient[] = { 0.2f * lightIntensity, 0.2f * lightIntensity, 0.2f * lightIntensity, 1.0f };
				GLfloat light_diffuse[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };
				GLfloat light_specular[] = { lightIntensity, lightIntensity, lightIntensity, 1.0f };

				glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
				glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
				glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
				glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

				// 카메라 설정
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				gluLookAt(camera.eye.x, camera.eye.y, camera.eye.z,
					camera.at.x, camera.at.y, camera.at.z,
					camera.up.x, camera.up.y, camera.up.z);

				// 흰색 재질 설정 (눈처럼 보이도록)
				GLfloat material_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
				GLfloat material_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				GLfloat material_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
				GLfloat material_shininess[] = { 100.0f };

				glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
				glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
				glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);

				// 구체 위치 설정 및 그리기 (매우 작게)
				glPushMatrix();
				glTranslatef(fallingSpheres[i].position.x, fallingSpheres[i].position.y, fallingSpheres[i].position.z);

				gluQuadricDrawStyle(fallingSphereQuadric[i], GLU_FILL);
				gluQuadricNormals(fallingSphereQuadric[i], GLU_SMOOTH);
				gluSphere(fallingSphereQuadric[i], 0.05f, 8, 8); // 매우 작은 구체

				glPopMatrix();

				glDisable(GL_LIGHTING);
				glDisable(GL_LIGHT0);
			}
		}
	}
	
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

// 타이머 콜백 함수
void TimerFunction(int value) {
	if (lightRotate) {
		lightAngle += angle;
		if (lightAngle >= 360.0f) lightAngle = 0.0f;
	}
	for (int i = 0; i < 3; i++) {
		sphereAngles[i] += sphereSpeed[i];
		if (sphereAngles[i] >= 360.0f) sphereAngles[i] = 0.0f;
	}
	UpdateSpherePositions();
	UpdateFallingSpheres(); // 떨어지는 구체 업데이트
	
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}