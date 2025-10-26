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
    GLuint VAO, VBO[2], EBO;
};

// 카메라
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.1f, 1.0f),
             glm::vec3(0.0f, 0.0f, 0.0f),
			 glm::vec3(0.0f, 1.0f, 0.0f) };

GLUquadric* sphere;
GLUquadric* spheres[3];
GLUquadric* s_spheres[3];
Shape orbits[3];
Shape s_orbits[3];
glm::mat4 Matrix[3];
glm::mat4 s_Matrix[3];
glm::mat4 smat[3];
glm::mat4 big_Matrix;
glm::mat4 scalemat(1.0f);
glm::mat4 zmat(1.0f);
glm::mat4 movemat(1.0f);
bool solid = true, angle = false, z_rotate = false;
float scale = 1.0f, zangle = 1.0f;


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
void CreateCorrdinate(Shape& line);
void CreateCube(Shape& cube);
void CreateCorn(Shape& corn);
void CreateOrbit(Shape& orbit, float radius, int segments);
void CreateMatrix();
void menu();

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
    glutCreateWindow("Example19");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    menu();
    CreateMatrix();
    for (int i = 0; i < 3; i++) {
        CreateOrbit(orbits[i], 0.5f, 100);
		CreateOrbit(s_orbits[i], 0.2f, 100);
    }
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMainLoop();
}
// 메뉴
void menu() {
    std::cout << "p: 직각투영 / 원근투영" << std::endl;
    std::cout << "m: 솔리드 / 와이어" << std::endl;
    std::cout << "w: 상 이동" << std::endl;
    std::cout << "a: 좌 이동" << std::endl;
    std::cout << "s: 하 이동" << std::endl;
    std::cout << "d: 우 이동" << std::endl;
    std::cout << "+: 카메라 z이동" << std::endl;
    std::cout << "y: 궤도들 반지름 크기조절" << std::endl;
    std::cout << "z: 주변구가 z축으로 회전" << std::endl;
    std::cout << "q: 종료" << std::endl;
}

void CreateMatrix() {
    big_Matrix = glm::mat4(1.0f);
	glm::mat4 transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
	glm::mat4 s_transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.0f, 0.0f));
	smat[0] = glm::mat4(1.0f);
    smat[1] = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    smat[2] = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    for (int i = 0; i < 3; i++) {
        Matrix[i] = transmat;
		s_Matrix[i] = s_transmat;
    }
}

void scaling(float s) {
    scalemat = glm::scale(glm::mat4(1.0f), glm::vec3(s, s, s));
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

        // 궤도 색상 (회색)
        orbit.colors.push_back(0.5f);
        orbit.colors.push_back(0.5f);
        orbit.colors.push_back(0.5f);

        orbit.index.push_back(i);
    }

    InitBuffers(orbit);
}

// 좌표축 생성 함수
void CreateCorrdinate(Shape& line) {
    line.vertices = {
        -2.0f, 0.0f, 0.0f,
        2.0f, 0.0f, 0.0f,

        0.0f, -2.0f, 0.0f,
        0.0f, 2.0f, 0.0f,

        0.0f, 0.0f, -2.0f,
        0.0f, 0.0f, 2.0f
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

// 삼각뿔 생성 함수
void CreateCorn(Shape& corn) {
    corn.vertices = {
        // 바닥면
        -0.2f, -0.2f, -0.2f,  // 0
        0.2f, -0.2f, -0.2f,   // 1
        -0.2f, -0.2f, 0.2f,   // 2
        0.2f, -0.2f, 0.2f,    // 3
        // 앞면
        0.0f, 0.2f, 0.0f,     // 4
        0.2f, -0.2f, 0.2f,    // 5
        -0.2f, -0.2f, 0.2f,   // 6
        // 오른면
        0.0f, 0.2f, 0.0f,     // 7
        0.2f, -0.2f, -0.2f,   // 8
        0.2f, -0.2f, 0.2f,    // 9
        // 뒷면
        0.0f, 0.2f, 0.0f,     // 10
        -0.2f, -0.2f, -0.2f,  // 11
        0.2f, -0.2f, -0.2f,   // 12
        // 왼면
        0.0f, 0.2f, 0.0f,     // 13
        -0.2f, -0.2f, 0.2f,   // 14
        -0.2f, -0.2f, -0.2f,  // 15
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
void CreateCube(Shape& cube) {
    cube.vertices = {
        // 앞면
        -0.2f, 0.2f, -0.2f, // 0
        -0.2f, -0.2f, -0.2f,// 1
        0.2f, -0.2f, -0.2f, // 2
        0.2f, 0.2f, -0.2f,  // 3

        // 뒷면
        0.2f, 0.2f, 0.2f,   // 4
        0.2f, -0.2f, 0.2f,  // 5
        -0.2f, -0.2f, 0.2f, // 6
        -0.2f, 0.2f, 0.2f,  // 7

        // 아래면
        -0.2f, -0.2f, -0.2f, // 1
        -0.2f, -0.2f, 0.2f,  // 6
        0.2f, -0.2f, 0.2f,   // 5
        0.2f, -0.2f, -0.2f,  // 2

        // 윗면 
        -0.2f, 0.2f, 0.2f,   // 7
        -0.2f, 0.2f, -0.2f,  // 0
        0.2f, 0.2f, -0.2f,   // 3
        0.2f, 0.2f, 0.2f,    // 4

        // 오른면
        0.2f, 0.2f, -0.2f,   // 3
        0.2f, -0.2f, -0.2f,  // 2
        0.2f, -0.2f, 0.2f,   // 5
        0.2f, 0.2f, 0.2f,    // 4

        // 왼면 
        -0.2f, 0.2f, 0.2f,  // 7
        -0.2f, -0.2f, 0.2f, // 6
        -0.2f, -0.2f, -0.2f,// 1
        -0.2f, 0.2f, -0.2f  // 0
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
    cube.colors = {
        // 앞면 - 초록색
        0.0f, 1.0f, 0.0f,  // 0
        0.0f, 1.0f, 0.0f,  // 1
        0.0f, 1.0f, 0.0f,  // 2
        0.0f, 1.0f, 0.0f,  // 3

        // 뒷면 - 빨간색
        1.0f, 0.0f, 0.0f,  // 4
        1.0f, 0.0f, 0.0f,  // 5
        1.0f, 0.0f, 0.0f,  // 6
        1.0f, 0.0f, 0.0f,  // 7

        // 아랫면 - 마젠타
        1.0f, 0.0f, 1.0f,  // 8
        1.0f, 0.0f, 1.0f,  // 9
        1.0f, 0.0f, 1.0f,  // 10
        1.0f, 0.0f, 1.0f,  // 11

        // 윗면 - 시안
        0.0f, 1.0f, 1.0f,  // 12
        0.0f, 1.0f, 1.0f,  // 13
        0.0f, 1.0f, 1.0f,  // 14
        0.0f, 1.0f, 1.0f,  // 15

        // 오른면 - 노란색
        1.0f, 1.0f, 0.0f,  // 16
        1.0f, 1.0f, 0.0f,  // 17
        1.0f, 1.0f, 0.0f,  // 18
        1.0f, 1.0f, 0.0f,  // 19

        // 완면 - 파란색
        0.0f, 0.0f, 1.0f,  // 20
        0.0f, 0.0f, 1.0f,  // 21
        0.0f, 0.0f, 1.0f,  // 22
        0.0f, 0.0f, 1.0f   // 23
    };
    InitBuffers(cube);
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩

    glGenBuffers(2, shape.VBO);                 // 버퍼id 생성

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
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("19_vertex.glsl");
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
    fragmentSource = filetobuf("19_fragment.glsl"); // 프래그세이더 읽어오기
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
    // 배경을 흰색으로 설정
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);
    
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");
    unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	// 투영 행렬 설정
    glm::mat4 projection = glm::mat4(1.0f);
    if (angle) projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 10.0f);
    else projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    
    // 뷰 행렬 설정
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    // 궤도 그리기
    glUniform3f(colorLocation, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; i++) {
        glm::mat4 orbitMatrix = movemat * zmat * scalemat * smat[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(orbitMatrix));
        glBindVertexArray(orbits[i].VAO);
        glDrawElements(GL_LINE_LOOP, orbits[i].index.size(), GL_UNSIGNED_INT, 0);

        // 작은 구 궤도 그리기 (spheres 주위)
        glm::mat4 s_orbitMatrix = movemat * zmat * scalemat * smat[i] * Matrix[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(s_orbitMatrix));
        glBindVertexArray(s_orbits[i].VAO);
		glDrawElements(GL_LINE_LOOP, s_orbits[i].index.size(), GL_UNSIGNED_INT, 0);
    }

    // 도형 그리기
    sphere = gluNewQuadric();
    for (int i = 0; i < 3; i++) {
		spheres[i] = gluNewQuadric();
		s_spheres[i] = gluNewQuadric();
    }
    if (solid) {
        gluQuadricDrawStyle(sphere, GLU_FILL);
        for (int i = 0; i < 3; i++) {
			gluQuadricDrawStyle(spheres[i], GLU_FILL);
			gluQuadricDrawStyle(s_spheres[i], GLU_FILL);
        }
    }
    else {
        gluQuadricDrawStyle(sphere, GLU_LINE);
        for (int i = 0; i < 3; i++) {
            gluQuadricDrawStyle(spheres[i], GLU_LINE);
			gluQuadricDrawStyle(s_spheres[i], GLU_LINE);
        }
    }
	
    // 중심 구 (가장 큰 구) - 빨간색
	glm::mat4 trans = glm::mat4(1.0f);
	trans = movemat * big_Matrix;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // 빨간색
    gluSphere(sphere, 0.2, 20, 20);
    
    for (int i = 0; i < 3; i++) {
        // 중간 크기 구 - 초록색
        glm::mat4 translate = glm::mat4(1.0f);
		translate = movemat * zmat * scalemat * smat[i] * Matrix[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(translate));
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f); // 초록색
        gluSphere(spheres[i], 0.05, 10, 10);

        // 작은 구 - 파란색
		glm::mat4 s_translate = glm::mat4(1.0f);
		s_translate = movemat * zmat * scalemat * smat[i] * Matrix[i] *  s_Matrix[i];
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(s_translate));
		glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f); // 파란색
		gluSphere(s_spheres[i], 0.02, 10, 10);

		gluDeleteQuadric(s_spheres[i]);
        gluDeleteQuadric(spheres[i]);
	}
    gluDeleteQuadric(sphere);
    
    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
	case 'p': // 직각 투영
        angle = true;
        break;
	case 'P': // 원근 투영
		angle = false;
		break;
    case 'm': // 솔리드
        solid = true;
        break;
    case 'M': // 와이어
        solid = false;
		break;
    case '+':
        camera.eye.z -= 0.1f;
        break;
    case '-':
        camera.eye.z += 0.1f;
        break;
    case 'w':
		movemat = glm::translate(movemat, glm::vec3(0.0f, 0.1f, 0.0f));
        break;
    case 'a':
        movemat = glm::translate(movemat, glm::vec3(-0.1f, 0.0f, 0.0f));
        break;
    case 's':
		movemat = glm::translate(movemat, glm::vec3(0.0f, -0.1f, 0.0f));
		break;
    case 'd':
		movemat = glm::translate(movemat, glm::vec3(0.1f, 0.0f, 0.0f));
        break;
	case 'y': // 궤도들 반지름 크기조절
		scale += 0.1f;
        scaling(scale);
		break;
	case 'Y':
		scale -= 0.1f;
        scaling(scale);
		break;
	case 'z': // 주변구가 z축으로 회전
		z_rotate = true;
        zangle = 2.0f;
		break;
	case 'Z':
		z_rotate = true;
        zangle = -2.0f;
		break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    glm::mat4 a = glm::rotate(glm::mat4(1.0f), glm::radians(1.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 b = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 c = glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    Matrix[0] = a * Matrix[0];
    Matrix[1] = b * Matrix[1];
    Matrix[2] = c * Matrix[2];

    glm::mat4 s_a = glm::rotate(glm::mat4(1.0f), glm::radians(2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 s_b = glm::rotate(glm::mat4(1.0f), glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 s_c = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    s_Matrix[0] = s_a * s_Matrix[0];
    s_Matrix[1] = s_b * s_Matrix[1];
    s_Matrix[2] = s_c * s_Matrix[2];

	if (z_rotate) zmat = glm::rotate(glm::mat4(1.0f), glm::radians(zangle), glm::vec3(0.0f, 0.0f, 1.0f)) * zmat;

    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}