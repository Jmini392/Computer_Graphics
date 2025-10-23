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

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    std::vector<float> colors = {
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f
    };
    float size = 0.2f;
	float centerX, centerY;
    int type;
    GLuint VAO, VBO[2];
};
Shape shape[8];
bool move[3] = {false, false, false}, size = false;
float pi = 3.141592;
float angle = 0.0f;

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

void firstcreateShape();
void createTriangle(Shape& triangle, float x, float y);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Example14");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    firstcreateShape();
    glutTimerFunc(50, TimerFunction, 1);
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMainLoop();
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩

    glGenBuffers(2, shape.VBO);                 // 버퍼id 생성

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // 좌표
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // 색상
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

// 처음 도형 생성
void firstcreateShape() {
    for (int i = 0; i < 8; i++) {
        shape[i].type = i;
        if (i < 4) {
            shape[i].centerX = -0.5f;
            shape[i].centerY = 0.0f;
        }
        else {
            shape[i].centerX = 0.5f;
            shape[i].centerY = 0.0f;
        }
        createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
	}
}

// 삼각형 생성 함수
void createTriangle(Shape& triangle, float x, float y) {
	triangle.centerX = x;
	triangle.centerY = y;
    if (triangle.type == 0) {
        triangle.vertices = {
            x + triangle.size * cos(pi / 4 + angle), y + triangle.size * sin(pi / 4 + angle), 0.0f,
            x + triangle.size * cos(7 * pi / 4 + angle), y + triangle.size * sin(7 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) * cos(2 * pi + angle), y + triangle.size * (float)sqrt(2) * sin(2 * pi + angle), 0.0f
        };
    }
    if (triangle.type == 1) {
        triangle.vertices = {
            x + triangle.size * cos(3 * pi / 4 + angle), y + triangle.size * sin(3 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(pi / 4 + angle), y + triangle.size * sin(pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) * cos(pi / 2 + angle), y + triangle.size * (float)sqrt(2) * sin(pi / 2 + angle), 0.0f
        };
    }
    if (triangle.type == 2) {
        triangle.vertices = {
            x + triangle.size * cos(5 * pi / 4 + angle), y + triangle.size * sin(5 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(3 * pi / 4 + angle), y + triangle.size * sin(3 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) * cos(pi + angle), y + triangle.size * (float)sqrt(2) * sin(pi + angle), 0.0f
        };
	}
    if (triangle.type == 3) {
        triangle.vertices = {
            x + triangle.size * cos(5 * pi / 4 + angle), y + triangle.size * sin(5 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(7 * pi / 4 + angle), y + triangle.size * sin(7 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) * cos(3 * pi / 2 + angle), y + triangle.size * (float)sqrt(2) * sin(3 * pi / 2 + angle), 0.0f,
        };
    }
    if (triangle.type == 4) {
        triangle.vertices = {
            x + triangle.size * cos(pi / 4 + angle), y + triangle.size * sin(pi / 4 + angle), 0.0f,
            x + triangle.size * cos(7 * pi / 4 + angle), y + triangle.size * sin(7 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) / 8 * cos(2 * pi + angle), y + triangle.size * (float)sqrt(2) / 8 * sin(2 * pi + angle), 0.0f
        };
    }
    if (triangle.type == 5) {
        triangle.vertices = {
            x + triangle.size * cos(3 * pi / 4 + angle), y + triangle.size * sin(3 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(pi / 4 + angle), y + triangle.size * sin(pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) / 8 * cos(pi / 2 + angle), y + triangle.size * (float)sqrt(2) / 8 * sin(pi / 2 + angle), 0.0f
        };
    }
    if (triangle.type == 6) {
        triangle.vertices = {
            x + triangle.size * cos(5 * pi / 4 + angle), y + triangle.size * sin(5 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(3 * pi / 4 + angle), y + triangle.size * sin(3 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) / 8 * cos(pi + angle), y + triangle.size * (float)sqrt(2) / 8 * sin(pi + angle), 0.0f
        };
    }
    if (triangle.type == 7) {
        triangle.vertices = {
            x + triangle.size * cos(5 * pi / 4 + angle), y + triangle.size * sin(5 * pi / 4 + angle), 0.0f,
            x + triangle.size * cos(7 * pi / 4 + angle), y + triangle.size * sin(7 * pi / 4 + angle), 0.0f,
            x + triangle.size * (float)sqrt(2) / 8 * cos(3 * pi / 2 + angle), y + triangle.size * (float)sqrt(2) / 8 * sin(3 * pi / 2 + angle), 0.0f,
        };
    }
    InitBuffers(triangle);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c':
        move[0] = true;
        move[1] = false;
		move[2] = false;
        break;
    case 't':
        move[0] = false;
        move[1] = true;
		move[2] = true;
        break;
	case 'p':
        if (size) size = false;
		else size = true;
		break;
    case 's':
		if (move[0] && !move[1] && !move[2]) move[0] = false;
		else if (!move[0] && move[1] && !move[2]) move[1] = false;
        else if (move[0] || move[1]) {
            move[0] = false;
            move[1] = false;
        }
		if (size) size = false;
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("14_vertex.glsl");
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
    fragmentSource = filetobuf("14_fragment.glsl"); // 프래그세이더 읽어오기
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
    glUseProgram(shaderProgramID);

    // 모든 도형 그리기
    for (int i = 0; i < 8; i++) {
        glBindVertexArray(shape[i].VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
	}
    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

bool big = false;
//--- 타이머 콜백 함수
void TimerFunction(int value) {
    if (move[0]) { // 시계방향 회전
        angle += 0.1f;
        for (int i = 0; i < 8; i++) {
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
        }
    }
    if (move[1]) { // 반시계방향 회전
        angle -= 0.1f;
        for (int i = 0; i < 8; i++) {
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
        }
    }
    if (size) {
        for (int i = 0; i < 8; i++) {
            if (big) { // 커지기
                shape[i].size += 0.005f;
				if (shape[i].size >= 0.5f) big = false;
            }
            else { // 작아지기
                shape[i].size -= 0.005f;
				if (shape[i].size <= 0.1f) big = true;
            }
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
		}
    }
    glutPostRedisplay();
    // 계속 타이머를 실행
    glutTimerFunc(50, TimerFunction, 1);
}