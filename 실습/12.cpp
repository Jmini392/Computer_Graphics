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
    std::vector<float> steps;
    float color[3];
    int type;
    int cnt = 0;
    float size = 0.3f;
	float centerX, centerY;
	bool ptol = false;
    GLuint VAO, VBO;
};
Shape shape[4];
Shape Bigshape;

// 타이머 실행 상태를 추가
bool timerRunning = false;
bool extense = false;

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
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void firstcreateShape();

void createLine(Shape& line, float x, float y);
void createTriangle(Shape& triangle, float x, float y);
void createRectangle(Shape& rectangle, float x, float y);
void createPentagon(Shape& pentagon, float x, float y);

void LinetoTriangle(Shape& line);
void TriangletoRectangle(Shape& triangle);
void RectangletoPentagon(Shape& rectangle);
void PentagontoLine(Shape& pentagon);

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
    glutInitWindowSize(600, 600);
    glutCreateWindow("Example12");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
	firstcreateShape();
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
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
}

// 도형 생성 함수
void firstcreateShape() {
    shape[0].centerX = -0.5f; shape[0].centerY = 0.5f;
    shape[1].centerX = 0.5f; shape[1].centerY = 0.5f;
    shape[2].centerX = -0.5f; shape[2].centerY = -0.5f;
    shape[3].centerX = 0.5f; shape[3].centerY = -0.5f;
	createLine(shape[0], shape[0].centerX, shape[0].centerY);
	createTriangle(shape[1], shape[1].centerX, shape[1].centerY);
	createRectangle(shape[2], shape[2].centerX, shape[2].centerY);
	createPentagon(shape[3], shape[3].centerX, shape[3].centerY);
	LinetoTriangle(shape[0]);
	TriangletoRectangle(shape[1]);
	RectangletoPentagon(shape[2]);
	PentagontoLine(shape[3]);
}
// 선 생성 함수
void createLine(Shape &line, float x, float y) {
    line.type = 0;
    line.centerX = x;
    line.centerY = y;
    line.vertices = {
        x + line.size, y + line.size, 0.0f,
        x - line.size, y - line.size, 0.0f,
        x - line.size, y - line.size, 0.0f,
        
        x + line.size, y + line.size, 0.0f,
        x - line.size, y - line.size, 0.0f,
        x - line.size, y - line.size, 0.0f,

        x + line.size, y + line.size, 0.0f,
        x - line.size, y - line.size, 0.0f,
        x + line.size, y + line.size, 0.0f,
    };

    line.color[0] = dis(gen);
    line.color[1] = dis(gen);
    line.color[2] = dis(gen);

    InitBuffers(line);
}
// 삼각형 생성 함수
void createTriangle(Shape& triangle, float x, float y) {
    triangle.type = 1;
    triangle.centerX = x;
    triangle.centerY = y;
    triangle.vertices = {
        x, y + triangle.size, 0.0f,
        x - (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f,
        x + (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f,

        x, y + triangle.size, 0.0f,
        x + (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f,
        x, y + triangle.size, 0.0f,

        x, y + triangle.size, 0.0f,
        x + (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f,
        x, y + triangle.size, 0.0f
    };
    triangle.color[0] = dis(gen);
    triangle.color[1] = dis(gen);
    triangle.color[2] = dis(gen);

    InitBuffers(triangle);
}
// 사각형 생성 함수
void createRectangle(Shape& rectangle, float x, float y) {
    rectangle.type = 2;
    rectangle.centerX = x;
    rectangle.centerY = y;
    rectangle.vertices = {
        x - rectangle.size, y + rectangle.size, 0.0f,
        x - rectangle.size, y - rectangle.size, 0.0f,
        x + rectangle.size, y - rectangle.size, 0.0f,

        x - rectangle.size, y + rectangle.size, 0.0f,
        x + rectangle.size, y - rectangle.size, 0.0f,
        x + rectangle.size, y + rectangle.size, 0.0f,

        x - rectangle.size, y + rectangle.size, 0.0f,
        x + rectangle.size, y + rectangle.size, 0.0f,
		x - rectangle.size, y + rectangle.size, 0.0f
    };

    rectangle.color[0] = dis(gen);
    rectangle.color[1] = dis(gen);
    rectangle.color[2] = dis(gen);

    InitBuffers(rectangle);
}
// 오각형 생성 함수
void createPentagon(Shape& pentagon, float x, float y) {
    const float pi = 3.141592;
    pentagon.type = 3;
    pentagon.centerX = x;
    pentagon.centerY = y;
    pentagon.vertices = {
        x + pentagon.size * cos(pi / 2), y + pentagon.size * sin(pi / 2), 0.0f,
        x + pentagon.size * cos((pi / 2) + (2 * pi / 5)), y + pentagon.size * sin((pi / 2) + (2 * pi / 5)), 0.0f,
        x + pentagon.size * cos((pi / 2) + (4 * pi / 5)), y + pentagon.size * sin((pi / 2) + (4 * pi / 5)), 0.0f,

        x + pentagon.size * cos(pi / 2), y + pentagon.size * sin(pi / 2), 0.0f,
        x + pentagon.size * cos((pi / 2) + (4 * pi / 5)), y + pentagon.size * sin((pi / 2) + (4 * pi / 5)), 0.0f,
        x + pentagon.size * cos((pi / 2) + (6 * pi / 5)), y + pentagon.size * sin((pi / 2) + (6 * pi / 5)), 0.0f,

        x + pentagon.size * cos(pi / 2), y + pentagon.size * sin(pi / 2), 0.0f,
        x + pentagon.size * cos((pi / 2) + (6 * pi / 5)), y + pentagon.size * sin((pi / 2) + (6 * pi / 5)), 0.0f,
        x + pentagon.size * cos((pi / 2) + (8 * pi / 5)), y + pentagon.size * sin((pi / 2) + (8 * pi / 5)), 0.0f,
    };
    pentagon.color[0] = dis(gen);
    pentagon.color[1] = dis(gen);
    pentagon.color[2] = dis(gen);
    InitBuffers(pentagon);
}

// 선으로 삼각형 그리기
void LinetoTriangle(Shape& line) {
    line.type = 1;
    std::cout << "Line center: " << line.centerX << " " << line.centerY << std::endl;
    // steps 벡터를 적절한 크기로 초기화
    line.steps.clear();
    line.steps.resize(line.vertices.size(), 0.0f);
    
    // 목표 삼각형 좌표 계산
    std::vector<float> goalVertices = {
        line.centerX, line.centerY + line.size, 0.0f,
        line.centerX - ((line.size / 2) * (float)sqrt(3)), line.centerY - (line.size / 2), 0.0f,
        line.centerX + ((line.size / 2) * (float)sqrt(3)), line.centerY - (line.size / 2), 0.0f,

        line.centerX, line.centerY + line.size, 0.0f,
        line.centerX + (line.size / 2) * (float)sqrt(3), line.centerY - line.size / 2, 0.0f,
        line.centerX, line.centerY + line.size, 0.0f,

        line.centerX, line.centerY + line.size, 0.0f,
        line.centerX + (line.size / 2) * (float)sqrt(3), line.centerY - line.size / 2, 0.0f,
        line.centerX, line.centerY + line.size, 0.0f
    };

    // 각 점에 대해 이동 스텝 계산
    for (int i = 0; i < line.vertices.size(); i += 3) {
        if (i + 2 < line.vertices.size() && i + 2 < goalVertices.size() && i + 2 < line.steps.size()) {
            line.steps[i] = (goalVertices[i] - line.vertices[i]) / 10.0f;
            line.steps[i + 1] = (goalVertices[i + 1] - line.vertices[i + 1]) / 10.0f;
            std::cout << line.steps[i] << line.steps[i + 1] << std::endl;
        }
    }
    
    line.cnt = 0;  // 애니메이션 카운터 초기화
    
    // 타이머가 실행 중이 아닐 때만 타이머 시작
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// 삼각형으로 사각형 그리기
void TriangletoRectangle(Shape& triangle) {
    triangle.type = 2;
    // steps 벡터를 적절한 크기로 초기화
    triangle.steps.clear();
    triangle.steps.resize(triangle.vertices.size(), 0.0f);
    // 목표 사각형 좌표 계산
    std::vector<float> goalVertices = {
        triangle.centerX - triangle.size, triangle.centerY + triangle.size, 0.0f,
        triangle.centerX - triangle.size, triangle.centerY - triangle.size, 0.0f,
        triangle.centerX + triangle.size, triangle.centerY - triangle.size, 0.0f,
        
        triangle.centerX - triangle.size, triangle.centerY + triangle.size, 0.0f,
        triangle.centerX + triangle.size, triangle.centerY - triangle.size, 0.0f,
        triangle.centerX + triangle.size, triangle.centerY + triangle.size, 0.0f,

        triangle.centerX - triangle.size, triangle.centerY + triangle.size, 0.0f,
        triangle.centerX + triangle.size, triangle.centerY + triangle.size, 0.0f,
        triangle.centerX - triangle.size, triangle.centerY + triangle.size, 0.0f
    };
    // 각 점에 대해 이동 스텝 계산
    for (int i = 0; i < triangle.vertices.size(); i += 3) {
        if (i + 2 < triangle.vertices.size() && i + 2 < goalVertices.size() && i + 2 < triangle.steps.size()) {
            std::cout << "Point " << i / 3 << " - Current: (" << triangle.vertices[i] << ", " << triangle.vertices[i + 1] << ")" << std::endl;
            std::cout << "Point " << i / 3 << " - Goal: (" << goalVertices[i] << ", " << goalVertices[i + 1] << ")" << std::endl;
            triangle.steps[i] = (goalVertices[i] - triangle.vertices[i]) / 10.0f;
            triangle.steps[i + 1] = (goalVertices[i + 1] - triangle.vertices[i + 1]) / 10.0f;
            //triangle.steps[i + 2] = 0.0f;
        }
    }
    triangle.cnt = 0;  // 애니메이션 카운터 초기화
    
    // 타이머가 실행 중이 아닐 때만 타이머 시작
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// 사각형으로 오각형 그리기
void RectangletoPentagon(Shape& rectangle) {
    rectangle.type = 3;
    const float pi = 3.141592;
	// steps 벡터를 적절한 크기로 초기화
	rectangle.steps.clear();
	rectangle.steps.resize(rectangle.vertices.size(), 0.0f);
	// 목표 오각형 좌표 계산
	std::vector<float> goalVertices = {
		rectangle.centerX + rectangle.size * cos(pi / 2), rectangle.centerY + rectangle.size * sin(pi / 2), 0.0f,
		rectangle.centerX + rectangle.size * cos((pi / 2) + (2 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (2 * pi / 5)), 0.0f,
		rectangle.centerX + rectangle.size * cos((pi / 2) + (4 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (4 * pi / 5)), 0.0f,

		rectangle.centerX + rectangle.size * cos(pi / 2), rectangle.centerY + rectangle.size * sin(pi / 2), 0.0f,
		rectangle.centerX + rectangle.size * cos((pi / 2) + (4 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (4 * pi / 5)), 0.0f,
		rectangle.centerX + rectangle.size * cos((pi / 2) + (6 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (6 * pi / 5)), 0.0f,
		
        rectangle.centerX + rectangle.size * cos(pi / 2), rectangle.centerY + rectangle.size * sin(pi / 2), 0.0f,
		rectangle.centerX + rectangle.size * cos((pi / 2) + (6 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (6 * pi / 5)), 0.0f,
        rectangle.centerX + rectangle.size * cos((pi / 2) + (8 * pi / 5)), rectangle.centerY + rectangle.size * sin((pi / 2) + (8 * pi / 5)), 0.0f,
	};
	// 각 점에 대해 이동 스텝 계산
    for (int i = 0; i < rectangle.vertices.size(); i += 3) {
        if (i + 2 < rectangle.vertices.size() && i + 2 < goalVertices.size() && i + 2 < rectangle.steps.size()) {
            std::cout << "Point " << i / 3 << " - Current: (" << rectangle.vertices[i] << ", " << rectangle.vertices[i + 1] << ")" << std::endl;
            std::cout << "Point " << i / 3 << " - Goal: (" << goalVertices[i] << ", " << goalVertices[i + 1] << ")" << std::endl;
            rectangle.steps[i] = (goalVertices[i] - rectangle.vertices[i]) / 10.0f;
            rectangle.steps[i + 1] = (goalVertices[i + 1] - rectangle.vertices[i + 1]) / 10.0f;
        }
    }
	rectangle.cnt = 0;  // 애니메이션 카운터 초기화
    
    // 타이머가 실행 중이 아닐 때만 타이머 시작
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// 오각형에서 선으로 그리기
void PentagontoLine(Shape& pentagon) {
	std::cout << "Pentagon center: " << pentagon.centerX << " " << pentagon.centerY << std::endl;
	// steps 벡터를 적절한 크기로 초기화
	pentagon.steps.clear();
	pentagon.steps.resize(pentagon.vertices.size(), 0.0f);
	// 목표 선 좌표 계산
	std::vector<float> goalVertices = {
		pentagon.centerX + pentagon.size, pentagon.centerY + pentagon.size, 0.0f,
		pentagon.centerX - pentagon.size, pentagon.centerY - pentagon.size, 0.0f,
		pentagon.centerX - pentagon.size, pentagon.centerY - pentagon.size, 0.0f,

        pentagon.centerX + pentagon.size, pentagon.centerY + pentagon.size, 0.0f,
        pentagon.centerX - pentagon.size, pentagon.centerY - pentagon.size, 0.0f,
        pentagon.centerX - pentagon.size, pentagon.centerY - pentagon.size, 0.0f,
		
        pentagon.centerX + pentagon.size, pentagon.centerY + pentagon.size, 0.0f,
        pentagon.centerX - pentagon.size, pentagon.centerY - pentagon.size, 0.0f,
        pentagon.centerX + pentagon.size, pentagon.centerY + pentagon.size, 0.0f,
	};
	// 각 점에 대해 이동 스텝 계산
    for (int i = 0; i < pentagon.vertices.size(); i += 3) {
        if (i + 2 < pentagon.vertices.size() && i + 2 < goalVertices.size() && i + 2 < pentagon.steps.size()) {
            std::cout << "Point " << i / 3 << " - Current: (" << pentagon.vertices[i] << ", " << pentagon.vertices[i + 1] << ")" << std::endl;
            std::cout << "Point " << i / 3 << " - Goal: (" << goalVertices[i] << ", " << goalVertices[i + 1] << ")" << std::endl;
            pentagon.steps[i] = (goalVertices[i] - pentagon.vertices[i]) / 10.0f;
            pentagon.steps[i + 1] = (goalVertices[i + 1] - pentagon.vertices[i + 1]) / 10.0f;
        }
    }    
    pentagon.cnt = 0;
    pentagon.type = 0;
    pentagon.ptol = true;
    
    // 타이머가 실행 중이 아닐 때만 타이머 시작
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'l':
        extense = true;
		createLine(Bigshape, 0.0f, 0.0f);
		LinetoTriangle(Bigshape);
        break;
    case 't':
        extense = true;
		createTriangle(Bigshape, 0.0f, 0.0f);
		TriangletoRectangle(Bigshape);
        break;
    case 'r':
        extense = true;
		createRectangle(Bigshape, 0.0f, 0.0f);
		RectangletoPentagon(Bigshape);
        break;
    case 'p':
        extense = true;
		createPentagon(Bigshape, 0.0f, 0.0f);
		PentagontoLine(Bigshape);
        break;
    case 'a':
        if (extense) {
            extense = false;
            glutTimerFunc(100, TimerFunction, 1);
        }
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
    vertexSource = filetobuf("8_vertex.glsl");
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

    if (!extense) {
        Shape line;
        line.vertices = {
            -1.0f, 0.0f, 0.0f,
             1.0f, 0.0f, 0.0f,
             0.0f, -1.0f, 0.0f,
             0.0f, 1.0f, 0.0f
        };
        line.color[0] = 0.0f;
        line.color[1] = 0.0f;
        line.color[2] = 0.0f;
        InitBuffers(line);

        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, line.color[0], line.color[1], line.color[2], 1.0f);
        glBindVertexArray(line.VAO);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 4);

        // 모든 도형 그리기
        for (int i = 0; i < 4; i++) {
            GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
            glUniform4f(colorLocation, shape[i].color[0], shape[i].color[1], shape[i].color[2], 1.0f);

            glBindVertexArray(shape[i].VAO);
            if (shape[i].type == 0) {
                if (!shape[i].ptol) {
                    glLineWidth(2.0f);
                    glDrawArrays(GL_LINES, 0, 3);
                }
                else glDrawArrays(GL_TRIANGLES, 0, 9);
            }
            else if (shape[i].type == 1) {
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            else if (shape[i].type == 2) {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            else if (shape[i].type == 3) {
                glDrawArrays(GL_TRIANGLES, 0, 9);
            }

            glBindVertexArray(0);
        }
    }
    if (Bigshape.type == 0) {
        if (!Bigshape.ptol) {
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, 3);
        }
        else glDrawArrays(GL_TRIANGLES, 0, 9);
    }
    else if (Bigshape.type == 1) {
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else if (Bigshape.type == 2) {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else if (Bigshape.type == 3) {
        glDrawArrays(GL_TRIANGLES, 0, 9);
    }

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

//--- 타이머 콜백 함수
void TimerFunction(int value) {
    bool animationActive = false;
    if (!extense) {
        for (int i = 0; i < 4; i++) {
            if (shape[i].type == 1 && shape[i].cnt < 10) {  // 삼각형이고 애니메이션이 아직 완료되지 않았으면
                animationActive = true;
                // 각 점을 목표 위치로 이동
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X 좌표 업데이트
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y 좌표 업데이트
                    }
                }
                shape[i].cnt++;  // 애니메이션 진행 단계 증가
                InitBuffers(shape[i]);  // 버퍼 업데이트
            }
            if (shape[i].type == 1 && shape[i].cnt >= 10) {
				createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
                TriangletoRectangle(shape[i]);
            }
            if (shape[i].type == 2 && shape[i].cnt < 10) {  // 사각형이고 애니메이션이 아직 완료되지 않았으면
                animationActive = true;
                // 각 점을 목표 위치로 이동
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X 좌표 업데이트
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y 좌표 업데이트
                    }
                }
                shape[i].cnt++;  // 애니메이션 진행 단계 증가
                InitBuffers(shape[i]);  // 버퍼 업데이트
            }
            if (shape[i].type == 2 && shape[i].cnt >= 10) {
				createRectangle(shape[i], shape[i].centerX, shape[i].centerY);
                RectangletoPentagon(shape[i]);
            }
            if (shape[i].type == 3 && shape[i].cnt < 10) {  // 오각형이고 애니메이션이 아직 완료되지 않았으면
                animationActive = true;
                // 각 점을 목표 위치로 이동
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X 좌표 업데이트
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y 좌표 업데이트
                    }
                }
                shape[i].cnt++;  // 애니메이션 진행 단계 증가
                InitBuffers(shape[i]);  // 버퍼 업데이트
            }
            if (shape[i].type == 3 && shape[i].cnt >= 10) {
				createPentagon(shape[i], shape[i].centerX, shape[i].centerY);
                PentagontoLine(shape[i]);
            }
            if (shape[i].type == 0 && shape[i].cnt < 10) {  // 선이고 애니메이션이 아직 완료되지 않았으면
                animationActive = true;
                // 각 점을 목표 위치로 이동
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X 좌표 업데이트
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y 좌표 업데이트
                    }
                }
                shape[i].cnt++;  // 애니메이션 진행 단계 증가
                InitBuffers(shape[i]);  // 버퍼 업데이트
            }
            if (shape[i].type == 0 && shape[i].cnt >= 10 && shape[i].ptol) {
                shape[i].ptol = false;
				createLine(shape[i], shape[i].centerX, shape[i].centerY);
                LinetoTriangle(shape[i]);
            }
        }
    }
    
    // 애니메이션이 진행 중이면 타이머 계속 실행
    else {
        if (Bigshape.cnt < 10) {  // 삼각형이고 애니메이션이 아직 완료되지 않았으면
            animationActive = true;
            // 각 점을 목표 위치로 이동
            for (int j = 0; j < Bigshape.vertices.size() && j < Bigshape.steps.size(); j += 3) {
                if (j + 2 < Bigshape.vertices.size() && j + 2 < Bigshape.steps.size()) {
                    Bigshape.vertices[j] += Bigshape.steps[j];       // X 좌표 업데이트
                    Bigshape.vertices[j + 1] += Bigshape.steps[j + 1]; // Y 좌표 업데이트
                }
            }
            Bigshape.cnt++;  // 애니메이션 진행 단계 증가
            InitBuffers(Bigshape);  // 버퍼 업데이트
        }
        if (Bigshape.type == 0 && Bigshape.cnt >= 10) {
			Bigshape.ptol = false;
        }
    }
    glutPostRedisplay();
    if (animationActive) {
        glutTimerFunc(100, TimerFunction, 1);
    }
    else {
        // 모든 애니메이션이 완료되면 타이머 플래그를 false로 설정
        timerRunning = false;
    }
}