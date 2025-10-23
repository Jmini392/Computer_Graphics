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
std::uniform_int_distribution<int> cu(0, 5);
std::uniform_int_distribution<int> co(0, 3);

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    std::vector<unsigned int> index;
    std::vector<float> colors;
    float centerX, centerY;
    GLuint VAO, VBO[2], EBO;
};
Shape line;
Shape cube;
Shape corn;
bool mode = true;
int f1, f2;
bool showCubeFaces[6] = {false, false, false, false, false, false}; // 앞면, 뒷면, 아래면, 위면, 오른면, 왼면
bool showCornFaces[4] = {false, false, false, false}; // 앞면, 뒷면, 왼면, 오른면
bool showCornBottom = false; // 사각뿔 바닥면
bool showAllFaces = true; // 전체 면 표시 여부

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
void DrawCorrdinate();
void CreateCube(Shape& cube);
void CreateCorn(Shape& corn);

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Example15");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    CreateCube(cube);
	CreateCorn(corn);
    DrawCorrdinate();
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
        -0.5f, -0.5f, -0.5f,  // 0
         0.5f, -0.5f, -0.5f,  // 1
         0.5f,  0.5f, -0.5f,  // 2
        -0.5f,  0.5f, -0.5f,  // 3
        
        // 뒷면
        -0.5f, -0.5f,  0.5f,  // 4
         0.5f, -0.5f,  0.5f,  // 5
         0.5f,  0.5f,  0.5f,  // 6
        -0.5f,  0.5f,  0.5f,  // 7
        
        // 아래면
        -0.5f, -0.5f, -0.5f,  // 8
         0.5f, -0.5f, -0.5f,  // 9
         0.5f, -0.5f,  0.5f,  // 10
        -0.5f, -0.5f,  0.5f,  // 11
        
        // 위면 
        -0.5f,  0.5f, -0.5f,  // 12
         0.5f,  0.5f, -0.5f,  // 13
         0.5f,  0.5f,  0.5f,  // 14
        -0.5f,  0.5f,  0.5f,  // 15
        
        // 오른면
         0.5f, -0.5f, -0.5f,  // 16
         0.5f,  0.5f, -0.5f,  // 17
         0.5f,  0.5f,  0.5f,  // 18
         0.5f, -0.5f,  0.5f,  // 19
        
        // 왼면 
        -0.5f, -0.5f, -0.5f,  // 20
        -0.5f,  0.5f, -0.5f,  // 21
        -0.5f,  0.5f,  0.5f,  // 22
        -0.5f, -0.5f,  0.5f   // 23
    };
    cube.index = {
        // 앞면
        0, 1, 2, 2, 3, 0,
        // 뒷면
        4, 5, 6, 6, 7, 4,
        // 아래면
        8, 9, 10, 10, 11, 8,
        // 위면
        12, 13, 14, 14, 15, 12,
        // 오른면
        16, 17, 18, 18, 19, 16,
        // 왼면
        20, 21, 22, 22, 23, 20
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

// 좌표축 그리기
void DrawCorrdinate() {
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

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    // 초기화
    for (int i = 0; i < 6; i++) showCubeFaces[i] = false;
    for (int i = 0; i < 4; i++) showCornFaces[i] = false;
    showCornBottom = false;
    showAllFaces = false;
    
    switch (key) {
    case '1': // 큐브 앞면
        showCubeFaces[0] = true;
        break;
    case '2': // 큐브 뒷면
        showCubeFaces[1] = true;
        break;
    case '3': // 큐브 아래면
        showCubeFaces[2] = true;
        break;
    case '4': // 큐브 위면
        showCubeFaces[3] = true;
        break;
    case '5': // 큐브 오른면
        showCubeFaces[4] = true;
        break;
    case '6': // 큐브 왼면
        showCubeFaces[5] = true;
        break;
    case '7': // 사각뿔 앞면
        showCornFaces[0] = true;
        break;
    case '8': // 사각뿔 뒷면
        showCornFaces[1] = true;
        break;
    case '9': // 사각뿔 왼쪽면
        showCornFaces[2] = true;
        break;
    case '0': // 사각뿔 오른쪽면
        showCornFaces[3] = true;
        break;
    case 'a': // 모양 변경
        mode = !mode;
        showAllFaces = true;
        break;
    case 'c': // 큐브 랜덤한 2면 같이 출력
		f1 = cu(gen);
        do {
            f2 = cu(gen);
        } while (f2 == f1);
		showCubeFaces[f1] = true;
		showCubeFaces[f2] = true;
        break;
    case 't': // 사각뿔 바닥과 랜덤한 면 같이 출력
        showCornBottom = true;
        showCornFaces[co(gen)] = true;
        break;
    case 's': // 솔리드 모드
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        showAllFaces = true;
        break;
    case 'w': // 와이어프레임 모드
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        showAllFaces = true;
        break;
    case 'q':
        exit(0);
        break;
    default:
        showAllFaces = true;
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
    vertexSource = filetobuf("15_vertex.glsl");
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
    fragmentSource = filetobuf("15_fragment.glsl"); // 프래그세이더 읽어오기
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
    // 도형 회전 변환 행렬
    glm::mat4 Matrix(1.0f);
    Matrix = glm::rotate(Matrix, glm::radians(-30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    glm::mat4 rotMat(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    Matrix = rotMat * Matrix;
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(Matrix));    
    // 좌표축 그리기
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // 도형 그리기
    glEnable(GL_DEPTH_TEST);

    if (mode) {
        // 큐브 그리기
        glBindVertexArray(cube.VAO);
        // 전체 큐브 그리기
        if (showAllFaces) glDrawElements(GL_TRIANGLES, cube.index.size(), GL_UNSIGNED_INT, 0);
        // 선택된 면만 그리기
        else {
            for (int i = 0; i < 6; i++) {
                if (showCubeFaces[i]) {
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
                }
            }
        }
    }
    else {
        // 사각뿔 그리기
        glBindVertexArray(corn.VAO);
        // 전체 사각뿔 그리기
        if (showAllFaces) glDrawElements(GL_TRIANGLES, corn.index.size(), GL_UNSIGNED_INT, 0);
        // 선택된 면만 그리기
        else { 
            // 바닥면 그리기
            if (showCornBottom) {
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            }
            // 선택된 옆면들 그리기
            for (int i = 0; i < 4; i++) {
                if (showCornFaces[i]) {
                    int startIndex = 6 + (i * 3); // 바닥면 6개 인덱스 후 각 면마다 3개씩
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }
            }
        }
    }

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}