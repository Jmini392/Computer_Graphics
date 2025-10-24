#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
//--- �ʿ��� ������� include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// ��ü ����ü
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

//--- ����� ���� �Լ� ����
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

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Example18");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    glEnable(GL_DEPTH_TEST);
    CreateCorrdinate(line);
    CreateCube(cube);
    CreateCorn(corn);
    CreateMatrix();
    menu();
    glutTimerFunc(50, TimerFunction, 1); // Ÿ�̸� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
}
// �޴�
void menu() {
	std::cout << "x: X�� ����" << std::endl;
	std::cout << "y: Y�� ����" << std::endl;
	std::cout << "r: y�� ����" << std::endl;
	std::cout << "a: ��ü Ȯ��" << std::endl;
	std::cout << "b: ���� ���� Ȯ��" << std::endl;
	std::cout << "d: x�� �̵�" << std::endl;
	std::cout << "e: y�� �̵�" << std::endl;
	std::cout << "t: ���� ��� ��ġ ��ȯ �ִϸ��̼�" << std::endl;
	std::cout << "u: ���Ʒ� �̵� ��ġ ��ȯ �ִϸ��̼�" << std::endl;
	std::cout << "v: Ȯ�� ��� ����/���� �ִϸ��̼�" << std::endl;
	std::cout << "1: ������ ���� ����" << std::endl;
	std::cout << "2: ���� ���� ����" << std::endl;
	std::cout << "3: ��� ���� ����" << std::endl;
	std::cout << "c: ���� ����" << std::endl;
	std::cout << "s: ����" << std::endl;
	std::cout << "q: ����" << std::endl;
}

// ��ǥ�� ���� �Լ�
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

// �ﰢ�� ���� �Լ�
void CreateCorn(Shape& corn) {
    corn.vertices = {
        // �ٴڸ�
        -0.2f, -0.2f, -0.2f,  // 0
        0.2f, -0.2f, -0.2f,   // 1
        -0.2f, -0.2f, 0.2f,   // 2
        0.2f, -0.2f, 0.2f,    // 3
        // �ո�
        0.0f, 0.2f, 0.0f,     // 4
        0.2f, -0.2f, 0.2f,    // 5
        -0.2f, -0.2f, 0.2f,   // 6
        // ������
        0.0f, 0.2f, 0.0f,     // 7
        0.2f, -0.2f, -0.2f,   // 8
        0.2f, -0.2f, 0.2f,    // 9
        // �޸�
        0.0f, 0.2f, 0.0f,     // 10
        -0.2f, -0.2f, -0.2f,  // 11
        0.2f, -0.2f, -0.2f,   // 12
        // �޸�
        0.0f, 0.2f, 0.0f,     // 13
        -0.2f, -0.2f, 0.2f,   // 14
        -0.2f, -0.2f, -0.2f,  // 15
    };
    corn.index = {
        // �ٴڸ�
        0, 1, 2,
        1, 2, 3,
        // �ո�
        4, 5, 6,
        // ������
        7, 8, 9,
        // �޸�
        10, 11, 12,
        // �޸�
        13, 14, 15
    };
    corn.colors = {
        // �ٴڸ� - ����Ÿ
        1.0f, 0.0f, 1.0f,  // 0
        1.0f, 0.0f, 1.0f,  // 1
        1.0f, 0.0f, 1.0f,  // 2
        1.0f, 0.0f, 1.0f,  // 3
        // �ո� - �����
        1.0f, 1.0f, 0.0f,  // 4
        1.0f, 1.0f, 0.0f,  // 5
        1.0f, 1.0f, 0.0f,  // 6
        // ������ - �ʷϻ�
        0.0f, 1.0f, 0.0f,  // 7
        0.0f, 1.0f, 0.0f,  // 8
        0.0f, 1.0f, 0.0f,  // 9
        // �޸� - �Ķ���
        0.0f, 0.0f, 1.0f,  // 10
        0.0f, 0.0f, 1.0f,  // 11
        0.0f, 0.0f, 1.0f,  // 12
        // �޸� - ������
        1.0f, 0.0f, 0.0f,  // 13
        1.0f, 0.0f, 0.0f,  // 14
        1.0f, 0.0f, 0.0f   // 15
    };
    InitBuffers(corn);
}

// ť�� ���� �Լ�
void CreateCube(Shape& cube) {
    cube.vertices = {
        // �ո�
        -0.2f, 0.2f, -0.2f, // 0
        -0.2f, -0.2f, -0.2f,// 1
        0.2f, -0.2f, -0.2f, // 2
        0.2f, 0.2f, -0.2f,  // 3

        // �޸�
        0.2f, 0.2f, 0.2f,   // 4
        0.2f, -0.2f, 0.2f,  // 5
        -0.2f, -0.2f, 0.2f, // 6
        -0.2f, 0.2f, 0.2f,  // 7

        // �Ʒ���
        -0.2f, -0.2f, -0.2f, // 1
        -0.2f, -0.2f, 0.2f,  // 6
        0.2f, -0.2f, 0.2f,   // 5
        0.2f, -0.2f, -0.2f,  // 2

        // ���� 
        -0.2f, 0.2f, 0.2f,   // 7
        -0.2f, 0.2f, -0.2f,  // 0
        0.2f, 0.2f, -0.2f,   // 3
        0.2f, 0.2f, 0.2f,    // 4

        // ������
        0.2f, 0.2f, -0.2f,   // 3
        0.2f, -0.2f, -0.2f,  // 2
        0.2f, -0.2f, 0.2f,   // 5
        0.2f, 0.2f, 0.2f,    // 4

        // �޸� 
        -0.2f, 0.2f, 0.2f,  // 7
        -0.2f, -0.2f, 0.2f, // 6
        -0.2f, -0.2f, -0.2f,// 1
        -0.2f, 0.2f, -0.2f  // 0
    };
    cube.index = {
        // �ո�
        0, 1, 2, 0, 2, 3,
        // �޸�
        4, 5, 6, 4, 6, 7,
        // �Ʒ���
        8, 9, 10, 8, 10, 11,
        // ����
        12, 13, 14, 12, 14, 15,
        // ������
        16, 17, 18, 16, 18, 19,
        // �޸�
        20, 21, 22, 20, 22, 23
    };
    cube.colors = {
        // �ո� - �ʷϻ�
        0.0f, 1.0f, 0.0f,  // 0
        0.0f, 1.0f, 0.0f,  // 1
        0.0f, 1.0f, 0.0f,  // 2
        0.0f, 1.0f, 0.0f,  // 3

        // �޸� - ������
        1.0f, 0.0f, 0.0f,  // 4
        1.0f, 0.0f, 0.0f,  // 5
        1.0f, 0.0f, 0.0f,  // 6
        1.0f, 0.0f, 0.0f,  // 7

        // �Ʒ��� - ����Ÿ
        1.0f, 0.0f, 1.0f,  // 8
        1.0f, 0.0f, 1.0f,  // 9
        1.0f, 0.0f, 1.0f,  // 10
        1.0f, 0.0f, 1.0f,  // 11

        // ���� - �þ�
        0.0f, 1.0f, 1.0f,  // 12
        0.0f, 1.0f, 1.0f,  // 13
        0.0f, 1.0f, 1.0f,  // 14
        0.0f, 1.0f, 1.0f,  // 15

        // ������ - �����
        1.0f, 1.0f, 0.0f,  // 16
        1.0f, 1.0f, 0.0f,  // 17
        1.0f, 1.0f, 0.0f,  // 18
        1.0f, 1.0f, 0.0f,  // 19

        // �ϸ� - �Ķ���
        0.0f, 0.0f, 1.0f,  // 20
        0.0f, 0.0f, 1.0f,  // 21
        0.0f, 0.0f, 1.0f,  // 22
        0.0f, 0.0f, 1.0f   // 23
    };
    InitBuffers(cube);
}

// ó�� ��Ļ���
void CreateMatrix() {
    Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    glm::mat4 rmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    glm::mat4 lmat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
    corMatrix = Matrix;
    rMatrix = rmat;
    lMatrix = lmat;
}

// �ִϸ��̼� ���� ����
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

// ȸ�� ���� ����
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

// ȸ�� ���
void RotateMatrix(float angle, glm::vec3 axis, glm::mat4 &matrix) {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    matrix = rotMat * matrix;
}

// ȸ�� ��� (����)
void RotateMatrix_origin(float angle, glm::vec3 axis, glm::mat4 &matrix) {
	glm::vec3 center = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
    glm::mat4 transMat1 = glm::translate(glm::mat4(1.0f), -center);
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    glm::mat4 transMat2 = glm::translate(glm::mat4(1.0f), center);
    matrix = transMat2 * rotMat * transMat1 * matrix;
}

// Ȯ��/��� ���
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

// Ȯ��/��� ��� (���� ����)
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

// �̵� ���
void TranslateMatrix(glm::vec3 &trans, glm::mat4 &matrix) {
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), trans);
    matrix = transMat * matrix;
	trans = glm::vec3(0.0f);
}

// �� ������ ������ ����ϸ� ���� ��ġ �ٲٴ� �ִϸ��̼�
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

// �� ������ ���� �Ʒ��� �����̸鼭 ���� ��ġ �ٲٴ� �ִϸ��̼�
void UpDownSwap() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rMatrix = rotMat * rMatrix;
    lMatrix = rotMat * lMatrix;
}

// �� ������ Ȯ�� ����ϸ鼭 ����/���� �ִϸ��̼�
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

// ���� �Լ�
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

// ���� ���� �Լ�
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // ���ؽ� �迭 ��üid ����
    glBindVertexArray(shape.VAO);               // ���ؽ� �迭 ��ü ���ε�

    glGenBuffers(2, shape.VBO);                 // ����id ����

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // ��ǥ
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &shape.EBO);                 // �ε��� ����id ����
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // ����
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1': // 1�� ���� ����
		chooseMatrix.clear();
		chooseMatrix.push_back(&rMatrix);
		animationMatrix.push_back(&rMatrix);
        std::cout << "choose right shape" << std::endl;
		break;
	case '2': // 2�� ���� ����
		chooseMatrix.clear();
		chooseMatrix.push_back(&lMatrix);
		animationMatrix.push_back(&lMatrix);
        std::cout << "choose left shape" << std::endl;
		break;
    case '3': // ���� ��ü ����
        chooseMatrix.clear();
        chooseMatrix.push_back(&rMatrix);
		chooseMatrix.push_back(&lMatrix);
		animationMatrix.clear();
		animationMatrix.push_back(&rMatrix);
		animationMatrix.push_back(&lMatrix);
		std::cout << "choose all shapes" << std::endl;
		break;
    case 'x': // x�� ȸ�� (����)
        self_angle = 2.0f;
		SetAxis(glm::vec3(1.0f, 0.0f, 0.0f));
		SetState(0, true);
		SetState(1, false);
        break;
    case 'X': // x�� �ݴ� ȸ�� (����)
        self_angle = -2.0f;
		SetAxis(glm::vec3(1.0f, 0.0f, 0.0f));
        SetState(0, true);
        SetState(1, false);
        break;
    case 'y': // y�� ȸ�� (����)
		self_angle = 2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(0, true);
		SetState(1, false);
        break;
    case 'Y': // y�� �ݴ� ȸ�� (����)
		self_angle = -2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(0, true);
		SetState(1, false);
        break;
    case 'r': // y�� ȸ�� (����)
		angle = 2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(1, true);
		SetState(0, false);
        break;
	case 'R': // y�� �ݴ� ȸ�� (����)
		angle = -2.0f;
		SetAxis(glm::vec3(0.0f, 1.0f, 0.0f));
        SetState(1, true);
        SetState(0, false);
        break;
    case 'a': // ���ڸ����� Ȯ��
		rscale = 1.1f;
		lscale = 1.1f;
		SetState(2, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'A': // ���ڸ����� ���
		rscale = 0.9f;
		lscale = 0.9f;
		SetState(2, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'b': // ������ �������� Ȯ��
		rscale = 1.1f;
		lscale = 1.1f;
        SetState(3, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
	case 'B': // ������ �������� ���
        rscale = 0.9f;
        lscale = 0.9f;
        SetState(3, true);
        rmaxscale = 0;
        lmaxscale = 0;
        break;
    case 'd': // x�� �̵�
        sel = 0;
        break;
	case 'D': // x�� �ݴ� �̵�
        sel = 1;
        break;
	case 'e': // y�� �̵�
        sel = 2;
        break;
	case 'E': // y�� �ݴ� �̵�
        sel = 3;
        break;
    case 't': // �� ������ ������ ����ϸ� ���� ��ġ �ٲٴ� �ִϸ��̼�
        rstate[0] = false; rstate[1] = false; rstate[2] = false; rstate[3] = false;
        lstate[0] = false; lstate[1] = false; lstate[2] = false; lstate[3] = false;
        rvec = glm::vec3(rMatrix[3][0], rMatrix[3][1], rMatrix[3][2]);
        lvec = glm::vec3(lMatrix[3][0], lMatrix[3][1], lMatrix[3][2]);
        tic = 0.0f;
        swap = true;
		break;
    case 'u': // �� ������ ���� �Ʒ��� �����̸鼭 ���� ��ġ �ٲٴ� �ִϸ��̼� 
		Reset();
		updown = true;
        break;
	case 'v': // �� ������ Ȯ�� ����ϸ鼭 ����/���� �ִϸ��̼�
        rstate[0] = false; rstate[1] = false; rstate[2] = false; rstate[3] = false;
        lstate[0] = false; lstate[1] = false; lstate[2] = false; lstate[3] = false;
		ScaleRotate();
        break;
    case 'c': // ���� ����
        change = !change;
        break;
    case 's': // ����
        Reset();
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
    //--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
    vertexSource = filetobuf("15_vertex.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
        return;
    }
}

//--- �����׸�Ʈ ���̴� ��ü �����
void make_fragmentShaders()
{
    GLchar* fragmentSource;
    //--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
    fragmentSource = filetobuf("15_fragment.glsl"); // �����׼��̴� �о����
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
        return;
    }
}

//--- ���̴� ���α׷� ����� ���̴� ��ü ��ũ�ϱ�
GLuint make_shaderProgram()
{
    GLuint shaderID;
    GLint result;
    GLchar errorLog[512];
    shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
    glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
    glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
    glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�
    glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
        return false;
    }
    glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�
    //--- ���� ���� ���̴����α׷� ���� �� �ְ�, �� �� �Ѱ��� ���α׷��� ����Ϸ���
    //--- glUseProgram �Լ��� ȣ���Ͽ� ��� �� Ư�� ���α׷��� �����Ѵ�.
    //--- ����ϱ� ������ ȣ���� �� �ִ�.
    return shaderID;
}

//--- ��� �ݹ� �Լ�
GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
    // ����� ������� ����
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
	glEnable(GL_DEPTH_TEST);
    // ��ǥ�� �׸���
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // ���� �׸���
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

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// Ÿ�̸� �ݹ� �Լ�
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