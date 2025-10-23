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

//--- ����� ���� �Լ� ����
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
    glutCreateWindow("Example17");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    glEnable(GL_DEPTH_TEST);
    CreateCube(cube);
    CreateCorn(corn);
    DrawCorrdinate();
    CreateMatrix();
    glutTimerFunc(50, TimerFunction, 1); // Ÿ�̸� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
}

// ó�� ��Ļ���
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

// y�� ȸ�� ���
void RotateMatrix() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotMatrix = rotMat * rotMatrix;
}

// ť�� ���� ȸ��
void CubeTopRotate() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    cube.Matrices[3] = rotMat * cube.Matrices[3];
}

// ť�� �ո� ����
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

// ť�� ���� ȸ��
void CubeSideRotate() {
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    cube.Matrices[5] = rotMat * cube.Matrices[5];
    cube.Matrices[4] = rotMat * cube.Matrices[4];
}

// ť�� �޸� ������
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

// �ﰢ�� ��� ����
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

// �ﰢ�� ���� ���ư��鼭 ����
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

// �ﰢ�� ���� �Լ�
void CreateCorn(Shape& corn) {
    corn.vertices = {
        // �ٴڸ�
        -0.5f, -0.5f, -0.5f,  // 0
        0.5f, -0.5f, -0.5f,   // 1
        -0.5f, -0.5f, 0.5f,   // 2
        0.5f, -0.5f, 0.5f,    // 3
        // �ո�
        0.0f, 0.5f, 0.0f,     // 4
        0.5f, -0.5f, 0.5f,    // 5
        -0.5f, -0.5f, 0.5f,   // 6
        // ������
        0.0f, 0.5f, 0.0f,     // 7
        0.5f, -0.5f, -0.5f,   // 8
        0.5f, -0.5f, 0.5f,    // 9
        // �޸�
        0.0f, 0.5f, 0.0f,     // 10
        -0.5f, -0.5f, -0.5f,  // 11
        0.5f, -0.5f, -0.5f,   // 12
        // �޸�
        0.0f, 0.5f, 0.0f,     // 13
        -0.5f, -0.5f, 0.5f,   // 14
        -0.5f, -0.5f, -0.5f,  // 15
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
        -0.5f, 0.5f, -0.5f, // 0
        -0.5f, -0.5f, -0.5f,// 1
        0.5f, -0.5f, -0.5f, // 2
        0.5f, 0.5f, -0.5f,  // 3

        // �޸�
        0.5f, 0.5f, 0.5f,   // 4
        0.5f, -0.5f, 0.5f,  // 5
        -0.5f, -0.5f, 0.5f, // 6
        -0.5f, 0.5f, 0.5f,  // 7

        // �Ʒ���
        -0.5f, -0.5f, -0.5f, // 1
        -0.5f, -0.5f, 0.5f,  // 6
        0.5f, -0.5f, 0.5f,   // 5
        0.5f, -0.5f, -0.5f,  // 2

        // ���� 
        -0.5f, 0.5f, 0.5f,   // 7
        -0.5f, 0.5f, -0.5f,  // 0
        0.5f, 0.5f, -0.5f,   // 3
        0.5f, 0.5f, 0.5f,    // 4

        // ������
        0.5f, 0.5f, -0.5f,   // 3
        0.5f, -0.5f, -0.5f,  // 2
        0.5f, -0.5f, 0.5f,   // 5
        0.5f, 0.5f, 0.5f,    // 4

        // �޸� 
        -0.5f, 0.5f, 0.5f,  // 7
        -0.5f, -0.5f, 0.5f, // 6
        -0.5f, -0.5f, -0.5f,// 1
        -0.5f, 0.5f, -0.5f  // 0
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

// ��ǥ�� �׸���
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
    case 'h': // ���� ����
        if (depth) {
            glEnable(GL_DEPTH_TEST);
            depth = false;
        }
        else {
            glDisable(GL_DEPTH_TEST);
            depth = true;
        }
        break;
    case 'p': // ��� ����
		showcube = !showcube;
        break;
    case 'u': // �ø�
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
    case 'y': // ������ü�� y�� ȸ��
        rotate = !rotate;
        break;
    case 'c': // ����
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
	case 't': // ť�� ������ Z���� �߽����� ȸ���ߴ� �����
		z_rotate = !z_rotate;
        break;
    case 'f': // ť�� �ո��� ���� ���ȴ� ������
		open = !open;
        break;
    case 's': // ť�� ������ ȸ���ߴ� �����
		side = !side;
        break;
    case 'b': // ť�� �޸��� �۾����ٰ� Ŀ����
		back = !back;
        break;
    case 'o': // �簢�� ��ü�� ���ȴ� ������
		allopen = !allopen;
        break;
    case 'r': // �簢�� �Ѹ龿 ���ȴ� ������
		rotateopen = !rotateopen;
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
    // ��ǥ�� �׸���
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(corMatrix));
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // ���� �׸���
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

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// Ÿ�̸� �ݹ� �Լ�
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