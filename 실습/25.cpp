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
	GLuint VAO, VBO[3], EBO;
};
Shape line; Shape cube; Shape corn;
Shape lightbox; Shape orbit;
bool showcube = true; bool rotate = false;
float angle = 0.0f;
bool lightRotate = false; bool lightOn = true;
float lightAngle = 0.0f; float lightDistance = 1.0f;
glm::vec3 lightPosition(0.0f);

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

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

// 메뉴
void menu() {
	std::cout << "n: 육면체/사각뿔 그리기" << std::endl;
	std::cout << "m: 조명 켜기 / 끄기" << std::endl;
	std::cout << "y: 객체를 y축에 대하여 회전(제자리에서 자전)" << std::endl;
	std::cout << "r: 조명을 객체의 중심 y축에 대하여 양 / 음 방향으로 공전시키기" << std::endl;
	std::cout << "z/Z: 조명을 객체에 가깝게 / 멀게 이동하기" << std::endl;
	std::cout << "q: 프로그램 종료"<< std::endl;
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
	CreateCube(lightbox, 0.2f, 0.2f, 0.2f);
	CreateCorn(corn);
	CreateCorrdinate();
	CreateOrbit(orbit, lightDistance, 100);
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
	/*cube.colors = {
		 //앞면 - 연한회색
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,

		// 뒷면 - 연한 회색
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,
		0.8f, 0.8f, 0.8f,

		// 아랫면 - 진한 회색
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,

		// 윗면 - 진한 회색
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,
		0.3f, 0.3f, 0.3f,

		// 오른면 - 회색
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,

		// 왼면 - 회색
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f
	};*/
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
	case 'n':
		showcube = !showcube;
		break;
	case 'm': // 조명 켜기/끄기 
		lightOn = !lightOn;
		break;
	case 'y': // y축 회전
		rotate = !rotate;
		break;
	case 'r': // 조명 공전
		lightRotate = !lightRotate;
		break;
	case 'z': // 조명 가까이
		lightDistance -= 0.1f;
		if (lightDistance < 0.5f) lightDistance = 0.5f;
		CreateOrbit(orbit, lightDistance, 100);
		break;
	case 'Z': // 조명 멀리
		lightDistance += 0.1f;
		if (lightDistance > 3.0f) lightDistance = 3.0f;
		CreateOrbit(orbit, lightDistance, 100);
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

	// 조명 설정
	glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	if (lightOn) glUniform3f(lightColorLocation, 1.5, 1.5, 1.5);
	else glUniform3f(lightColorLocation, 0.0, 0.0, 0.0);
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
	// 궤도 그리기
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
	glBindVertexArray(orbit.VAO);
	glDrawElements(GL_LINE_LOOP, orbit.index.size(), GL_UNSIGNED_INT, 0);
	// 조명상자 그리기
	glm::mat4 lightboxMatrix = glm::mat4(1.0f);
	lightboxMatrix = glm::translate(lightboxMatrix, lightPosition);
	
	// 조명상자가 원점(도형 위치)을 바라보도록 회전 계산
	glm::vec3 lightAt = glm::normalize(-lightPosition);
	// 회전 행렬 계산 - lookAt 방식
	glm::mat4 lookAtMatrix = glm::lookAt(glm::vec3(0.0f), lightAt, glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 lookAtMatrix = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 orientationMatrix = glm::inverse(lookAtMatrix);
	lightboxMatrix = lightboxMatrix * orientationMatrix;
	//lightboxMatrix = lightboxMatrix * lookAtMatrix;

	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(lightboxMatrix));
	glBindVertexArray(lightbox.VAO);
	glDrawElements(GL_TRIANGLES, lightbox.index.size(), GL_UNSIGNED_INT, 0);
	// 도형 그리기
	glm::mat4 matrix = glm::mat4(1.0f);
	matrix = glm::rotate(matrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
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
		angle += 2.0f;
		if (angle >= 360.0f) angle = 0.0f;
	}
	if (lightRotate) {
		lightAngle += 2.0f;
		if (lightAngle >= 360.0f) lightAngle = 0.0f;
	}
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}