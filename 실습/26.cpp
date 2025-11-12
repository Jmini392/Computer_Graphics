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
Shape lightbox; Shape orbit;
GLUquadric* sphere[3];
glm::vec3 spherePositions[3] = {
	glm::vec3(-1.6f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 0.0f)
};
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
glm::vec3 lightPosition(0.0f, 0.0f, lightDistance);
int color = 0;

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

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

// 메뉴
void menu() {
	std::cout << "c: 조명 색상 변경" << std::endl;
	std::cout << "r/R: 조명을 객체의 중심 y축에 대하여 양 / 음 방향으로 공전시키기" << std::endl;
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
	CreateCube(lightbox, 0.2f, 0.2f, 0.2f);
	CreateOrbit(orbit, lightDistance, 100);
	menu();
	glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력
	glutMainLoop();
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
	case 'c': // 조명 색상 변경
		color++;
		if (color > 3) color = 0;
		break;
	case 'r': // 조명 공전
		lightRotate = !lightRotate;
		angle = 2.0f;
		break;
	case 'R':
		lightRotate = !lightRotate;
		angle = -2.0f;
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

	// 조명 설정
	glUniform3f(lightPosLocation, lightPosition.x, lightPosition.y, lightPosition.z);
	if (color == 0) glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	else if (color == 1) glUniform3f(lightColorLocation, 1.0, 0.0, 0.0); // 빨강
	else if (color == 2) glUniform3f(lightColorLocation, 0.0, 0.0, 1.0); // 파랑
	else if (color == 3) glUniform3f(lightColorLocation, 0.0, 1.0, 0.0); // 초록
	// 카메라 위치
	glUniform3f(viewPosLocation, camera.eye.x, camera.eye.y, camera.eye.z);
	// 뷰 행렬
	glm::mat4 view = glm::lookAt(camera.eye, camera.at, camera.up);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	// 투영 행렬
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));
	// 궤도 그리기
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
	
	// 구체 그리기 - 각각 다른 색상으로
	for (int i = 0; i < 3; i++) {
		// 셰이더를 비활성화하고 고정 파이프라인 사용
		glUseProgram(0);
		
		// OpenGL 고정 파이프라인의 조명 및 재질 설정
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		
		// 조명 설정 (기존 조명 설정 유지)
		GLfloat light_pos[] = {lightPosition.x, lightPosition.y, lightPosition.z, 1.0f};
		GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
		GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
		
		// 조명 색상 변경 적용
		if (color == 1) {
			light_diffuse[0] = 1.0f; light_diffuse[1] = 0.0f; light_diffuse[2] = 0.0f;
			light_specular[0] = 1.0f; light_specular[1] = 0.0f; light_specular[2] = 0.0f;
		} else if (color == 2) {
			light_diffuse[0] = 0.0f; light_diffuse[1] = 0.0f; light_diffuse[2] = 1.0f;
			light_specular[0] = 0.0f; light_specular[1] = 0.0f; light_specular[2] = 1.0f;
		} else if (color == 3) {
			light_diffuse[0] = 0.0f; light_diffuse[1] = 1.0f; light_diffuse[2] = 0.0f;
			light_specular[0] = 0.0f; light_specular[1] = 1.0f; light_specular[2] = 0.0f;
		}
		
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
		GLfloat material_ambient[] = {0.2f * sphereColors[i].x, 0.2f * sphereColors[i].y, 0.2f * sphereColors[i].z, 1.0f};
		GLfloat material_diffuse[] = {sphereColors[i].x, sphereColors[i].y, sphereColors[i].z, 1.0f};
		GLfloat material_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
		GLfloat material_shininess[] = {50.0f};
		
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
		gluSphere(sphere[i], (i + 1) * 0.2f, 20, 20);
		gluDeleteQuadric(sphere[i]);
		
		glPopMatrix();
		
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
	
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
		if (lightAngle < 0.0f) lightAngle = 360.0f;
	}
	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}