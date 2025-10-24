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
Shape line; Shape cube; Shape corn;
GLUquadric* sphere; GLUquadric* cylinder;
glm::mat4 corMatrix(1.0f), rMatrix(1.0f), lMatrix(1.0f);
glm::mat4 Matrix(1.0f);
std::vector<glm::mat4*> chooseMatrix;
std::vector<glm::mat4*> animationMatrix;
glm::vec3 Axis(0.0f); glm::vec3 r_axis(0.0f); glm::vec3 l_axis(0.0f); 
glm::vec3 translate(0.0f);
float angle = 0.0f, self_angle = 0.0f;
float rscale = 1.0f, lscale = 1.0f;
bool change = false; bool swap = false, updown = false;
bool rstate[4] = { false, false, false, false };
bool lstate[4] = { false, false, false, false };
int sel = -1; int rmaxscale = 0, lmaxscale = 0, a = 0;

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
void CreateCorrdinate(Shape &line);
void CreateCube(Shape& cube);
void CreateCorn(Shape& corn);
void CreateMatrix();
void menu();

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
    glutCreateWindow("Example18");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    glEnable(GL_DEPTH_TEST);
    CreateCorrdinate(line);
    CreateCube(cube);
    CreateCorn(corn);
    CreateMatrix();
    menu();
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMainLoop();
}
// 메뉴
void menu() {
	std::cout << "x: X축 자전" << std::endl;
	std::cout << "y: Y축 자전" << std::endl;
	std::cout << "r: y축 공전" << std::endl;
	std::cout << "a: 자체 확대" << std::endl;
	std::cout << "b: 원점 기준 확대" << std::endl;
	std::cout << "d: x축 이동" << std::endl;
	std::cout << "e: y축 이동" << std::endl;
	std::cout << "t: 원점 통과 위치 교환 애니메이션" << std::endl;
	std::cout << "u: 위아래 이동 위치 교환 애니메이션" << std::endl;
	std::cout << "v: 확대 축소 공전/자전 애니메이션" << std::endl;
	std::cout << "1: 오른쪽 도형 선택" << std::endl;
	std::cout << "2: 왼쪽 도형 선택" << std::endl;
	std::cout << "3: 모든 도형 선택" << std::endl;
	std::cout << "c: 도형 변경" << std::endl;
	std::cout << "s: 리셋" << std::endl;
	std::cout << "q: 종료" << std::endl;
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

// 처음 행렬생성
void CreateMatrix() {
    Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    glm::mat4 rmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    glm::mat4 lmat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
    corMatrix = Matrix;
    rMatrix = rmat;
    lMatrix = lmat;
}

// 애니메이션 상태 수정
void SetState(int a, bool set) {
    for (int i = 0; i < chooseMatrix.size(); i++) {
        if (chooseMatrix[i] == &rMatrix) {
            rstate[a] = set;
        }
        else if (chooseMatrix[i] == &lMatrix) {
            lstate[a] = set;
		}
	}
}

// 회전 방향 설정
void SetAxis(glm::vec3 axis) {
    for (int i = 0; i < chooseMatrix.size(); i++) {
        if (chooseMatrix[i] == &rMatrix) {
			r_axis = axis;
        }
        else if (chooseMatrix[i] == &lMatrix) {
            l_axis = axis;
        }
	}
}

// 회전 행렬
void RotateMatrix(float angle, glm::vec3 axis, glm::mat4 &matrix) {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    matrix = rotMat * matrix;
}

// 회전 행렬 (자전)
void RotateMatrix_origin(float angle, glm::vec3 axis, glm::mat4 &matrix) {
	glm::vec3 center = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    glm::mat4 transMat1 = glm::translate(glm::mat4(1.0f), -center);
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    glm::mat4 transMat2 = glm::translate(glm::mat4(1.0f), center);
    matrix = transMat2 * rotMat * transMat1 * matrix;
}

// 확대/축소 행렬
void ScaleMatrix(float scale, glm::mat4 &matrix) {
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    matrix = scaleMat * matrix;
    if (&matrix == &rMatrix) {
        if (rmaxscale < 5) rmaxscale++;
        else rstate[3] = false;
    }
    else if (&matrix == &lMatrix) {
        if (lmaxscale < 5) lmaxscale++;
        else lstate[3] = false;
    }
}

// 확대/축소 행렬 (원점 기준)
void ScaleMatrix_origin(float scale, glm::mat4 &matrix) {
    glm::vec3 center = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    glm::mat4 transMat1 = glm::translate(glm::mat4(1.0f), -center);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
	glm::mat4 transMat2 = glm::translate(glm::mat4(1.0f), center);
	matrix = transMat2 * scaleMat * transMat1 * matrix;
    if (&matrix == &rMatrix) {
        if (rmaxscale < 5) rmaxscale++;
        else rstate[2] = false;
    }
    else if (&matrix == &lMatrix) {
        if (lmaxscale < 5) lmaxscale++;
        else lstate[2] = false;
    }
}

// 이동 행렬
void TranslateMatrix(glm::vec3 &trans, glm::mat4 &matrix) {
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), trans);
    matrix = transMat * matrix;
	trans = glm::vec3(0.0f);
}

// 두 도형이 원점을 통과하면 서로 위치 바꾸는 애니메이션
glm::vec3 rvec; glm::vec3 lvec;
glm::vec3 rvec0; glm::vec3 lvec0;
float tic = 0.0f;
void SwapPosition() {
    rvec0 = (1.0f - tic) * rvec + tic * lvec;
	lvec0 = (1.0f - tic) * lvec + tic * rvec;
    rMatrix[3][0] = rvec0.x;
    rMatrix[3][1] = rvec0.y;
    rMatrix[3][2] = rvec0.z;

    lMatrix[3][0] = lvec0.x;
    lMatrix[3][1] = lvec0.y;
    lMatrix[3][2] = lvec0.z;
    
    tic += 0.02f;

    if (tic > 1.0f) {
        tic = 0.0f;
        swap = false;
	}
}

// 두 도형이 위로 아래로 움직이면서 서로 위치 바꾸는 애니메이션
void UpDownSwap() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rMatrix = rotMat * rMatrix;
    lMatrix = rotMat * lMatrix;
}

// 두 도형이 확대 축소하면서 공전/자전 애니메이션
void ScaleRotate() {
	animationMatrix.push_back(&rMatrix);
    animationMatrix.push_back(&lMatrix);

	r_axis = glm::vec3(0.0f, 1.0f, 0.0f);
	l_axis = glm::vec3(0.0f, 1.0f, 0.0f);

	angle = 2.0f;
	self_angle = 2.0f;
	rscale = 1.1f;
    lscale = 0.9f;

	rstate[0] = true; rstate[2] = true;
	lstate[1] = true; lstate[2] = true;
}

// 리셋 함수
void Reset() {
    CreateMatrix();
	chooseMatrix.clear();
	animationMatrix.clear();
    rstate[0] = false; rstate[1] = false; rstate[2] = false; rstate[3] = false;
    lstate[0] = false; lstate[1] = false; lstate[2] = false; lstate[3] = false;
    angle = 0.0f;
    self_angle = 0.0f;
    sel = -1;
    rscale = 1.0f;
    lscale = 1.0f;
	rmaxscale = 0;
	lmaxscale = 0;
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
    case '1': // 1번 도형 선택
		chooseMatrix.clear();
		chooseMatrix.push_back(&rMatrix);
		animationMatrix.push_back(&rMatrix);
        std::cout << "choose right shape" << std::endl;
		break;
	case '2': // 2번 도형 선택
		chooseMatrix.clear();
		chooseMatrix.push_back(&lMatrix);
		animationMatrix.push_back(&lMatrix);
        std::cout << "choose left shape" << std::endl;
		break;
    case '3': // 도형 전체 선택
        chooseMatrix.clear();
        chooseMatrix.push_back(&rMatrix);
		chooseMatrix.push_back(&lMatrix);
		animationMatrix.clear();
		animationMatrix.push_back(&rMatrix);
		animationMatrix.push_back(&lMatrix);
		std::cout << "choose all shapes" << std::endl;
		break;
    case 'x': // x축 회전 (자전)
        self_angle = 2.0f;
		SetAxis(glm::vec3(1.0f, 0.0f, 0.0f));
		SetState(0, true);
		SetState(1, false);
        break;
    case 'X': // x축 반대 회전 (자전)
        self_angle = -2.0f;
		SetAxis(glm::vec3(1.0f, 0.0f, 0.0f));
        SetState(0, true);
        SetState(1, false);
        break;
    case 'y': // y축 회전 (자전)
		self_angle = 2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(0, true);
		SetState(1, false);
        break;
    case 'Y': // y축 반대 회전 (자전)
		self_angle = -2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(0, true);
		SetState(1, false);
        break;
    case 'r': // y축 회전 (공전)
		angle = 2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(1, true);
		SetState(0, false);
        break;
	case 'R': // y축 반대 회전 (공전)
		angle = -2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(1, true);
        SetState(0, false);
        break;
    case 'a': // 제자리에서 확대
		rscale = 1.1f;
		lscale = 1.1f;
		SetState(2, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'A': // 제자리에서 축소
		rscale = 0.9f;
		lscale = 0.9f;
		SetState(2, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'b': // 원점을 기준으로 확대
		rscale = 1.1f;
		lscale = 1.1f;
        SetState(3, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'B': // 원점을 기준으로 축소
        rscale = 0.9f;
        lscale = 0.9f;
        SetState(3, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
    case 'd': // x축 이동
        sel = 0;
        break;
	case 'D': // x축 반대 이동
        sel = 1;
        break;
	case 'e': // y축 이동
        sel = 2;
        break;
	case 'E': // y축 반대 이동
        sel = 3;
        break;
    case 't': // 두 도형이 원점을 통과하면 서로 위치 바꾸는 애니메이션
        rstate[0] = false; rstate[1] = false; rstate[2] = false; rstate[3] = false;
        lstate[0] = false; lstate[1] = false; lstate[2] = false; lstate[3] = false;
        rvec = glm::vec3(rMatrix[3][0], rMatrix[3][1], rMatrix[3][2]);
        lvec = glm::vec3(lMatrix[3][0], lMatrix[3][1], lMatrix[3][2]);
        tic = 0.0f;
        swap = true;
		break;
    case 'u': // 두 도형이 위로 아래로 움직이면서 서로 위치 바꾸는 애니메이션 
		Reset();
		updown = true;
        break;
	case 'v': // 두 도형이 확대 축소하면서 공전/자전 애니메이션
        rstate[0] = false; rstate[1] = false; rstate[2] = false; rstate[3] = false;
        lstate[0] = false; lstate[1] = false; lstate[2] = false; lstate[3] = false;
		ScaleRotate();
        break;
    case 'c': // 도형 변경
        change = !change;
        break;
    case 's': // 리셋
        Reset();
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
	glEnable(GL_DEPTH_TEST);
    // 좌표축 그리기
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // 도형 그리기
	glm::mat4 rfMatrix = Matrix * rMatrix;
	glm::mat4 lfMatrix = Matrix * lMatrix;
    if (change) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(rfMatrix));
        glBindVertexArray(cube.VAO);
        glDrawElements(GL_TRIANGLES, cube.index.size(), GL_UNSIGNED_INT, 0);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(lfMatrix));
        glBindVertexArray(corn.VAO);
        glDrawElements(GL_TRIANGLES, corn.index.size(), GL_UNSIGNED_INT, 0);

    }
	else {
       glUseProgram(0);
        
        glColor3f(0.0f, 0.0f, 1.0f);
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(rfMatrix));
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_LINE);
        gluSphere(sphere, 0.2, 20, 20);
        gluDeleteQuadric(sphere);
        glPopMatrix();
        
        glColor3f(0.0f, 1.0f, 0.0f);
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(lfMatrix));
        cylinder = gluNewQuadric();
        gluQuadricDrawStyle(cylinder, GLU_LINE);
        gluCylinder(cylinder, 0.2, 0.0, 0.4, 20, 10);
        gluDeleteQuadric(cylinder);
        glPopMatrix();
        
        glUseProgram(shaderProgramID);
	}

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    for (int i = 0; i < animationMatrix.size(); i++) {
        if (animationMatrix[i] == &rMatrix) {
            if (rstate[0]) RotateMatrix_origin(self_angle, r_axis, *animationMatrix[i]);
            if (rstate[1]) RotateMatrix(angle, r_axis, *animationMatrix[i]);
            if (rstate[2]) ScaleMatrix_origin(rscale, *animationMatrix[i]);
            if (rstate[3]) ScaleMatrix(rscale, *animationMatrix[i]);
        }
        else if (animationMatrix[i] == &lMatrix) {
            if (lstate[0]) RotateMatrix_origin(self_angle, l_axis, *animationMatrix[i]);
            if (lstate[1]) RotateMatrix(angle, l_axis, *animationMatrix[i]);
            if (lstate[2]) ScaleMatrix_origin(lscale, *animationMatrix[i]);
            if (lstate[3]) ScaleMatrix(lscale, *animationMatrix[i]);
        }
    }
    for (int i = 0; i < chooseMatrix.size(); i++) {
		if (sel == 0) translate.x = 0.01f;
		else if (sel == 1) translate.x = -0.01f;
		else if (sel == 2) translate.y = 0.01f;
		else if (sel == 3) translate.y = -0.01f;
        TranslateMatrix(translate, *chooseMatrix[i]);
		if (i == chooseMatrix.size() - 1) sel = -1;
	}
    if (swap) {
        SwapPosition();
    }
    if (updown) {
        UpDownSwap();
		a += 2;
        if (a > 180) {
            a = 0;
            updown = false;
		}
	}
	glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}