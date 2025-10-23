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
    std::vector<glm::mat4> Matrices;
    GLuint VAO, VBO[2], EBO;
};
Shape line; Shape cube; Shape corn;
glm::mat4 corMatrix(1.0f); glm::mat4 rotMatrix(1.0f);
bool showcube = true, rotate = false, depth = false, curling = false;
bool z_rotate = false, open = false, side = false, back = false;
bool allopen = false, rotateopen = false;

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
void CreateMatrix();
void RotateMatrix();
void CubeTopRotate();
void CubeFrontOpen();
void CubeSideRotate();
void CubeBackScale();
void CornAllOpen();
void CornSideOpen();

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
    glutCreateWindow("Example17");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    glEnable(GL_DEPTH_TEST);
    CreateCube(cube);
    CreateCorn(corn);
    DrawCorrdinate();
    CreateMatrix();
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMainLoop();
}

// 처음 행렬생성
void CreateMatrix() {
    cube.Matrices.resize(6);
    corn.Matrices.resize(6);
    for (int i = 0; i < 6; i++) {
        cube.Matrices[i] = glm::mat4(1.0f);
        corn.Matrices[i] = glm::mat4(1.0f);
    }
    glm::mat4 matrix = glm::rotate(glm::mat4(1.0f), glm::radians(-20.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    rotMatrix = matrix;
    corMatrix = matrix;
}

// y축 회전 행렬
void RotateMatrix() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotMatrix = rotMat * rotMatrix;
}

// 큐브 윗면 회전
void CubeTopRotate() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    cube.Matrices[3] = rotMat * cube.Matrices[3];
}

// 큐브 앞면 열기
float open_angle = 0.0f;
bool open_dir = true;
void CubeFrontOpen() {
    if (open_dir) {
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
        cube.Matrices[0] = translateBack * rotMat * translateToOrigin * cube.Matrices[0];
        open_angle += 2.0f;
		if (open_angle >= 90.0f) open_dir = false;
    }
    else {
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
        cube.Matrices[0] = translateBack * rotMat * translateToOrigin * cube.Matrices[0];
        open_angle -= 2.0f;
		if (open_angle <= 0.0f) open_dir = true;
	}
}

// 큐브 옆면 회전
void CubeSideRotate() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    cube.Matrices[5] = rotMat * cube.Matrices[5];
    cube.Matrices[4] = rotMat * cube.Matrices[4];
}

// 큐브 뒷면 스케일
float scale_factor = 0.0f;
bool scale_dir = true;
void CubeBackScale() {
    if (scale_dir) {
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 1.0f));
        cube.Matrices[1] = scaleMat * cube.Matrices[1];
        scale_factor -= 1.0f;
        if (scale_factor <= -10.0f) scale_dir = false;
    }
    else {
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 1.25f, 1.0f));
        cube.Matrices[1] = scaleMat * cube.Matrices[1];
        scale_factor += 1.0f;
        if (scale_factor >= 0.0f) scale_dir = true;
	}
}

// 삼각뿔 모두 열기
float corn_open_angle = 0.0f;
bool corn_open_dir = true;
void CornAllOpen() {
    if (corn_open_dir) {
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
        corn.Matrices[2] = translateBack * rotMat * translateToOrigin * corn.Matrices[2];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
        corn.Matrices[3] = translateBack * rotMat * translateToOrigin * corn.Matrices[3];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.5f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -0.5f));
        corn.Matrices[4] = translateBack * rotMat * translateToOrigin * corn.Matrices[4];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
        corn.Matrices[5] = translateBack * rotMat * translateToOrigin * corn.Matrices[5];
		
        corn_open_angle += 2.0f;
		if (corn_open_angle >= 235.0f) corn_open_dir = false;
    }
    else {
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
        corn.Matrices[2] = translateBack * rotMat * translateToOrigin * corn.Matrices[2];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
        corn.Matrices[3] = translateBack * rotMat * translateToOrigin * corn.Matrices[3];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.5f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -0.5f));
		corn.Matrices[4] = translateBack * rotMat * translateToOrigin * corn.Matrices[4];

        translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
        rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
        corn.Matrices[5] = translateBack * rotMat * translateToOrigin * corn.Matrices[5];

		corn_open_angle -= 2.0f;
		if (corn_open_angle <= 0.0f) corn_open_dir = true;
    }
}

// 삼각뿔 옆면 돌아가면서 열기
float corn_angle = 0.0f;
bool corn_rotate_dir = true;
int i = 4;
void CornSideOpen() {
    if (corn_rotate_dir) {
        if (i == 2) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
            corn.Matrices[2] = translateBack * rotMat * translateToOrigin * corn.Matrices[2];
        }
        else if (i == 3) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
            corn.Matrices[3] = translateBack * rotMat * translateToOrigin * corn.Matrices[3];
        }
        else if (i == 4) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.5f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -0.5f));
            corn.Matrices[4] = translateBack * rotMat * translateToOrigin * corn.Matrices[4];
        }
        else if (i == 5) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
            corn.Matrices[5] = translateBack * rotMat * translateToOrigin * corn.Matrices[5];
		}
        corn_angle += 2.0f;
        if (corn_angle >= 120.0f) corn_rotate_dir = false;
    }
    else {
        if (i == 2) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -0.5f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.5f));
            corn.Matrices[2] = translateBack * rotMat * translateToOrigin * corn.Matrices[2];
        }
        else if (i == 3) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
            corn.Matrices[3] = translateBack * rotMat * translateToOrigin * corn.Matrices[3];
        }
        else if (i == 4) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.5f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -0.5f));
            corn.Matrices[4] = translateBack * rotMat * translateToOrigin * corn.Matrices[4];
        }
        else if (i == 5) {
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
            corn.Matrices[5] = translateBack * rotMat * translateToOrigin * corn.Matrices[5];
		}
        corn_angle -= 2.0f;
        if (corn_angle <= 0.0f) {
            corn_rotate_dir = true;
            if (i < 5) i++;
			else i = 2;
        }
    }
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
        -0.5f, 0.5f, -0.5f, // 0
        -0.5f, -0.5f, -0.5f,// 1
        0.5f, -0.5f, -0.5f, // 2
        0.5f, 0.5f, -0.5f,  // 3

        // 뒷면
        0.5f, 0.5f, 0.5f,   // 4
        0.5f, -0.5f, 0.5f,  // 5
        -0.5f, -0.5f, 0.5f, // 6
        -0.5f, 0.5f, 0.5f,  // 7

        // 아래면
        -0.5f, -0.5f, -0.5f, // 1
        -0.5f, -0.5f, 0.5f,  // 6
        0.5f, -0.5f, 0.5f,   // 5
        0.5f, -0.5f, -0.5f,  // 2

        // 윗면 
        -0.5f, 0.5f, 0.5f,   // 7
        -0.5f, 0.5f, -0.5f,  // 0
        0.5f, 0.5f, -0.5f,   // 3
        0.5f, 0.5f, 0.5f,    // 4

        // 오른면
        0.5f, 0.5f, -0.5f,   // 3
        0.5f, -0.5f, -0.5f,  // 2
        0.5f, -0.5f, 0.5f,   // 5
        0.5f, 0.5f, 0.5f,    // 4

        // 왼면 
        -0.5f, 0.5f, 0.5f,  // 7
        -0.5f, -0.5f, 0.5f, // 6
        -0.5f, -0.5f, -0.5f,// 1
        -0.5f, 0.5f, -0.5f  // 0
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
    switch (key) {
    case 'h': // 은면 제거
        if (depth) {
            glEnable(GL_DEPTH_TEST);
            depth = false;
        }
        else {
            glDisable(GL_DEPTH_TEST);
            depth = true;
        }
        break;
    case 'p': // 모양 변경
		showcube = !showcube;
        break;
    case 'u': // 컬링
        if (curling) {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            curling = false;
        }
        else {
            glDisable(GL_CULL_FACE);
            curling = true;
        }
        break;
    case 'y': // 도형전체가 y축 회전
        rotate = !rotate;
        break;
    case 'c': // 리셋
		showcube = true;
        rotate = false;
		depth = false;
		curling = false;
		z_rotate = false;
		open = false;
		side = false;
		back = false;
		allopen = false;
		rotateopen = false;
		cube.Matrices.clear();
		corn.Matrices.clear();
        CreateMatrix();
		open_angle = 0.0f;
		scale_factor = 0.0f;
		scale_dir = true;
		corn_open_angle = 0.0f;
		corn_open_dir = true;
		corn_angle = 0.0f;
		corn_rotate_dir = true;
		i = 4;
        break;
	case 't': // 큐브 윗면이 Z축을 중심으로 회전했다 멈췄다
		z_rotate = !z_rotate;
        break;
    case 'f': // 큐브 앞면이 위로 열렸다 닫혔다
		open = !open;
        break;
    case 's': // 큐브 옆면이 회전했다 멈췄다
		side = !side;
        break;
    case 'b': // 큐브 뒷면이 작아졌다가 커졌다
		back = !back;
        break;
    case 'o': // 사각뿔 전체가 열렸다 닫혔다
		allopen = !allopen;
        break;
    case 'r': // 사각뿔 한면씩 열렸다 닫혔다
		rotateopen = !rotateopen;
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
    // 좌표축 그리기
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // 도형 그리기
    if (showcube) {
        for (int i = 0; i < 6; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = rotMatrix * cube.Matrices[i];
			glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(cube.VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));        
		}
    }
    else {
        for (int i = 0; i < 6; i++) {
			glm::mat4 model1 = glm::mat4(1.0f);
			model1 = rotMatrix * corn.Matrices[i];
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model1));
            glBindVertexArray(corn.VAO);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * 3 * sizeof(unsigned int)));
        }
    }

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    if (rotate) RotateMatrix();
    if (z_rotate) CubeTopRotate();
    if (open) CubeFrontOpen();
    if (side) CubeSideRotate();
    if (back) CubeBackScale();
    if (allopen) CornAllOpen();
    if (rotateopen) CornSideOpen();
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}