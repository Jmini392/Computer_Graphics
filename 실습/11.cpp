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
    std::vector<float> vertices;  // 나선형의 모든 점들을 저장
    GLuint VAO, VBO;
    int currentDrawCount = 0;  // 그려야 할 점의 개수
    int totalPoints = 0;       // 전체 점의 개수
    bool isAnimating = false;  // 애니메이션 중인지
};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> bis(-0.8f, 0.7f);

Shape shape[6];
float R = 1.0f, G = 1.0f, B = 1.0f; // 배경색상
bool isPoint = true, clicked = false;
int selectes = 1;

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
void TimerFunction(int value);
float MouseX(int x) {
    return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
    return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
void InitBuffers(Shape& shape);
void createConnectedSpiral(float x, float y, int a);

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
    glutCreateWindow("Example11");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();

    // 타이머 시작
    glutTimerFunc(50, TimerFunction, 1);

    //--- 콜백 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMouseFunc(Mouse); // 마우스 입력
    glutMainLoop();
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    if (shape.VAO != 0) {
        glDeleteVertexArrays(1, &shape.VAO);
        glDeleteBuffers(1, &shape.VBO);
    }

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

/*void createConnectedSpiral(float x, float y, int a) {
	//R = dis(gen), G = dis(gen), B = dis(gen); // 배경색 랜덤 설정
    shape[a].vertices.clear();
    //shape[a].centerX = x;
    //shape[a].centerY = y;

    const int pointsPerSpiral = 60;  // 각 나선당 점 개수
    const float maxRadius = 0.2f;    // 최대 반지름
    const float spiralTurns = 3.0f;  // 3바퀴

    // 첫 번째 나선: 클릭한 점에서 반시계방향으로 바깥으로 3바퀴
    float firstSpiralEndX, firstSpiralEndY;
    
    for (int i = 0; i < pointsPerSpiral; i++) {
        float t = (float)i / (pointsPerSpiral - 1);  // 0.0 ~ 1.0
        float angle = t * spiralTurns * 2.0f * 3.14159f;  // 반시계방향 각도 (양수)
        float radius = maxRadius * t;  // 안에서 바깥으로 (반지름 증가)

        float dx = x + radius * cos(angle);
        float dy = y + radius * sin(angle);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);

        // 마지막 점의 좌표를 저장 (첫 번째 나선의 끝점)
        if (i == pointsPerSpiral - 1) {
            firstSpiralEndX = dx;
            firstSpiralEndY = dy;
        }
    }

    // 두 번째 스파이럴의 중심점 계산 (첫 번째 스파이럴의 오른쪽에 위치)
    float distance = maxRadius * 2.0f;  // 두 반지름의 합
    float secondCenterX = x + distance;  // 첫 번째 중심의 오른쪽에 위치
    float secondCenterY = y;             // 같은 높이

    // 두 번째 스파이럴: 새로운 중심점에서 180도부터 시작하여 시계방향으로 안으로 3바퀴
    for (int i = 0; i < pointsPerSpiral; i++) {
        float t = (float)i / (pointsPerSpiral - 1);  // 0.0 ~ 1.0
        
        // 180도(π 라디안)에서 시작하여 시계방향으로 회전 (음수 각도)
        float startAngle = 3.14159f;  // 180도
        float angle = startAngle - t * spiralTurns * 2.0f * 3.14159f;  // 시계방향 회전
        float radius = maxRadius * (1.0f - t);  // 바깥에서 안으로 (반지름 감소)

        // 두 번째 중심점을 기준으로 두 번째 나선 생성
        float dx = secondCenterX + radius * cos(angle);
        float dy = secondCenterY + radius * sin(angle);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);
    }

    shape[a].totalPoints = pointsPerSpiral * 2;  // 전체 점의 개수 (두 나선)
    shape[a].currentDrawCount = 0;
    //shape[a].isActive = true;
    shape[a].isAnimating = true;

    InitBuffers(shape[a]);
}*/

// 나선형 생성 함수
void createSpiral(float x, float y, int a) {
    shape[a].vertices.clear();
    
    float mx, my;
    for (float theta = 0.0f; theta <= 18.9f; theta += 0.3f) {
        float r = 0.01f * theta;
        float dx = x + r * cos(theta);
        float dy = y + r * sin(theta);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);

		shape[a].totalPoints++;
        if (abs(theta - 18.9f) < 0.001f) {
            mx = dx; 
            my = dy;
		}
    }
	std::cout << shape[a].totalPoints << "개 생성됨" << std::endl;
	float dis = sqrt((mx - x) * (mx - x) + (my - y) * (my - y));
    for (float theta = 21.9; theta >= 0.0; theta -= 0.3) {
        float r = 0.01f * theta;       
        float dx = (mx + dis + 0.03f) + r * cos(theta);
        float dy = my + r * sin(theta);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);

        shape[a].totalPoints++;
    }
    std::cout << shape[a].totalPoints << "개 생성됨" << std::endl;

    shape[a].isAnimating = true;
    InitBuffers(shape[a]);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        if (!clicked) selectes = 1;
        std::cout << selectes << "개만 생성" << std::endl;
        break;
    case '2':
        if (!clicked) selectes = 2;
        std::cout << selectes << "개만 생성" << std::endl;
        break;
    case '3':
        if (!clicked) selectes = 3;
        std::cout << selectes << "개만 생성" << std::endl;
        break;
    case '4':
        if (!clicked) selectes = 4;
        std::cout << selectes << "개만 생성" << std::endl;
        break;
    case '5':
        if (!clicked) selectes = 5;
        std::cout << selectes << "개만 생성" << std::endl;
        break;
    case 'p':
        isPoint = true;
        break;
    case 'l':
        isPoint = false;
        break;
    case 'c':
        // 모든 스파이럴 초기화
        for (int i = 1; i <= 5; i++) {
            if (shape[i].VAO != 0) {
                glDeleteVertexArrays(1, &shape[i].VAO);
                glDeleteBuffers(1, &shape[i].VBO);
            }
            shape[i].vertices.clear();
            shape[i].isAnimating = false;
            shape[i].currentDrawCount = 0;
            shape[i].totalPoints = 0;
        }
        selectes = 1;
        clicked = false;
		R = G = B = 1.0f;
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
        if (!clicked) {
            R = dis(gen), G = dis(gen), B = dis(gen); // 배경색 랜덤 설정
            for (int i = 1; i <= selectes; i++) {
                std::cout << "마우스 클릭됨 - 연결된 이중 나선 생성" << std::endl;
                float mx = bis(gen);
                float my = bis(gen);
                //createConnectedSpiral(mx, my, i);
                createSpiral(mx, my, i);
            }
        }
        clicked = true;
        glutPostRedisplay();
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
    glClearColor(R, G, B, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");

    glPointSize(3.0f);
    for (int i = 1; i <= selectes; i++) {
        glBindVertexArray(shape[i].VAO);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

        if (isPoint) glDrawArrays(GL_POINTS, 0, shape[i].currentDrawCount);
        else glDrawArrays(GL_LINE_STRIP, 0, shape[i].currentDrawCount);
    }

    glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}

//--- 타이머 콜백 함수
void TimerFunction(int value) {
    for (int i = 1; i <= selectes; i++) {
        if (shape[i].isAnimating) {
            if (shape[i].currentDrawCount < shape[i].totalPoints) shape[i].currentDrawCount++;
            else shape[i].isAnimating = false;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);  // 계속 타이머 실행
}