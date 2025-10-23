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

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    float color[3];
    int type;
    GLuint VAO, VBO;
};

Shape shapes[10];
int selected = -1, cnt = 0;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

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
GLvoid Mouse(int button, int state, int x, int y);
float MouseX(int x) {
    return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
    return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
void InitBuffers(Shape& shape);

void createPoint(float x, float y);
void createLine(float x, float y);
void createTriangle(float x, float y);
void createRectangle(float x, float y);

bool isPointInTriangle(float px, float py, const std::vector<float>& vertices);
bool isPointInRectangle(float px, float py, const std::vector<float>& vertices);
bool isPointNearLine(float px, float py, const std::vector<float>& vertices);
bool isPointNearPoint(float px, float py, const std::vector<float>& vertices);

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
    glutCreateWindow("Example8");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();    
    //--- 콜백 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMouseFunc(Mouse); // 마우스 입력
    glutMainLoop();
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glGenBuffers(1, &shape.VBO);                // 버퍼id 생성

    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);   // 버퍼객체에 저장할 데이터 타입 지정

    // 바인드 후에 호출하는 모든 버퍼는 바인딩 된 버퍼를 사용한다.
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    // 사용자가 정의한 데이터를 현재 바인딩된 버퍼에 복사한다.

    // 버텍스 속성 포인터 설정
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 0번 속성, 3개의 값, float형, 정규화 안함, 한 정점당 3개의 float, 시작위치 0

    glEnableVertexAttribArray(0);               // 버텍스 속성 배열 사용

    //glBindVertexArray(0);                       // 버텍스 배열 객체 바인딩 해제
}

// 점 생성 함수
void createPoint(float x, float y) {
    Shape point;
    point.type = 0;
    point.vertices = {
        x, y, 0.0f
    };
    
    point.color[0] = dis(gen);
    point.color[1] = dis(gen);
    point.color[2] = dis(gen);

    InitBuffers(point);
    shapes[cnt] = point;
    cnt++;
}
// 선 생성 함수
void createLine(float x, float y) {
    Shape line;
    float size = 0.15f;
    line.type = 1;
    line.vertices = {
        x - size, y, 0.0f,
        x + size, y, 0.0f
    };
    
    line.color[0] = dis(gen);
    line.color[1] = dis(gen);
    line.color[2] = dis(gen);
    
    InitBuffers(line);
    shapes[cnt] = line;
	cnt++;
}
// 삼각형 생성 함수
void createTriangle(float x, float y) {
    Shape triangle;
    float size = 0.1f;
    triangle.type = 2;
    triangle.vertices = {
        x, y + size, 0.0f,
        x - size, y - size, 0.0f,
        x + size, y - size, 0.0f
    };
    
    triangle.color[0] = dis(gen);
    triangle.color[1] = dis(gen);
    triangle.color[2] = dis(gen);
    
    InitBuffers(triangle);
    shapes[cnt] = triangle;
	cnt++;
}
// 사각형 생성 함수
void createRectangle(float x, float y) {
    Shape rectangle;
    float size = 0.1f;
    rectangle.type = 3;
    rectangle.vertices = {
        x - size, y + size, 0.0f,
        x - size, y - size, 0.0f,
        x + size, y - size, 0.0f,
        
        x - size, y + size, 0.0f,
        x + size, y - size, 0.0f,
        x + size, y + size, 0.0f
    };
    
    rectangle.color[0] = dis(gen);
    rectangle.color[1] = dis(gen);
    rectangle.color[2] = dis(gen);
    
    InitBuffers(rectangle);
    shapes[cnt] = rectangle;
	cnt++;
}

// 점이 점 근처에 있는지 확인
bool isPointNearPoint(float px, float py, const std::vector<float>& vertices) {    
    float dx = px - vertices[0];
    float dy = py - vertices[1];
    float distance = sqrt(dx * dx + dy * dy);
    
    return distance < 0.05f;
}
// 점이 선 근처에 있는지 확인
bool isPointNearLine(float px, float py, const std::vector<float>& vertices) {
    float x1 = vertices[0], y1 = vertices[1];
    float x2 = vertices[3], y2 = vertices[4];

    // 선분과 점 사이의 거리 계산
    float A = sqrt(pow(x1 - px, 2) + pow(y1 - py, 2));
    float B = sqrt(pow(x2 - px, 2) + pow(y2 - py, 2));
    float line = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

    return abs(line - A - B) < 0.05f;
}
// 점이 삼각형 내부에 있는지 확인
bool isPointInTriangle(float px, float py, const std::vector<float>& vertices) {
    float x1 = vertices[0], y1 = vertices[1];
    float x2 = vertices[3], y2 = vertices[4];
    float x3 = vertices[6], y3 = vertices[7];
    
	float Area = 0.5f * abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
	float a = 0.5f * abs(x1 * (y2 - py) + x2 * (py - y1) + px * (y1 - y2));
    float b = 0.5f * abs(x2 * (y3 - py) + x3 * (py - y2) + px * (y2 - y3));
	float c = 0.5f * abs(x3 * (y2 - y3) + x2 * (y3 - py) + x3 * (py - y2));
    if (abs(Area - (a + b + c)) <= 0.05f) return true;
	else return false;
}
// 점이 사각형 내부에 있는지 확인
bool isPointInRectangle(float px, float py, const std::vector<float>& vertices) {
    // 사각형의 경계 박스 계산
    float minX = vertices[0], maxX = vertices[0];
    float minY = vertices[1], maxY = vertices[1];
    
    for (int i = 0; i < vertices.size(); i += 3) {
        if (vertices[i] < minX) minX = vertices[i];
        if (vertices[i] > maxX) maxX = vertices[i];
        if (vertices[i + 1] < minY) minY = vertices[i + 1];
        if (vertices[i + 1] > maxY) maxY = vertices[i + 1];
    }
    
    return (px >= minX && px <= maxX && py >= minY && py <= maxY);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
		if (cnt > 9) break;
        createPoint(MouseX(x), MouseY(y));
        break;
    case 'm':
		if (cnt > 9) break;
        createLine(MouseX(x), MouseY(y));
        break;
    case 't':
		if (cnt > 9) break;
        createTriangle(MouseX(x), MouseY(y));
        break;
    case 'r':
		if (cnt > 9) break;
        createRectangle(MouseX(x), MouseY(y));
        break;
    case 'w':
        for (int i = 1; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'a':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 's':
        for (int i = 1; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'd':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'i':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.02f;
            shapes[selected].vertices[i + 1] += 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'j':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.02f;
            shapes[selected].vertices[i + 1] -= 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'k':
        for(int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.02f;
            shapes[selected].vertices[i + 1] += 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'l':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.02f;
            shapes[selected].vertices[i + 1] -= 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'c':
        for (int i = 0; i < cnt; i++) {
            glDeleteVertexArrays(1, &shapes[i].VAO);
            glDeleteBuffers(1, &shapes[i].VBO);
            shapes[i].vertices.clear();
            shapes[i].type = -1;
        }
        selected = -1;
        cnt = 0;
        break;
    case 'q':
        exit(0);
		break;
    }
    glutPostRedisplay();
}

// 마우스 콜백 함수
GLvoid Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (int i = cnt - 1; i >= 0; i--) {
            bool clicked = false;
            if (shapes[i].type == 0) { // 점
				clicked = isPointNearPoint(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 1) { // 선
				clicked = isPointNearLine(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 2) { // 삼각형
				clicked = isPointInTriangle(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 3) { // 사각형
				clicked = isPointInRectangle(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            if (clicked) {
                selected = i; // 도형 선택
				std::cout << "Selected shape index: " << selected << std::endl;
                glutPostRedisplay();
                break;
            }
        }
    }
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("8_vertex.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if(!result) {
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
    fragmentSource = filetobuf("8_fragment.glsl"); // 프래그세이더 읽어오기
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
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    
    // 모든 도형 그리기
    for (int i = 0; i < cnt; i++) {
        const auto& shape = shapes[i];
        
        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, shape.color[0], shape.color[1], shape.color[2], 1.0f);
        
        glBindVertexArray(shape.VAO);
        if (shape.type == 0) {
            glPointSize(10.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (shape.type == 1) {
            glLineWidth(3.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_LINES, 0, 2);
        }
        else if (shape.type == 2) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (shape.type == 3) {
            glLineWidth(1.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glBindVertexArray(0);
    }
    
    glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}