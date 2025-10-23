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
std::uniform_int_distribution<int> cu(0, 5);
std::uniform_int_distribution<int> co(0, 3);

// ��ü ����ü
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
bool showCubeFaces[6] = {false, false, false, false, false, false}; // �ո�, �޸�, �Ʒ���, ����, ������, �޸�
bool showCornFaces[4] = {false, false, false, false}; // �ո�, �޸�, �޸�, ������
bool showCornBottom = false; // �簢�� �ٴڸ�
bool showAllFaces = true; // ��ü �� ǥ�� ����

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
    glutCreateWindow("Example15");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    CreateCube(cube);
	CreateCorn(corn);
    DrawCorrdinate();
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
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
        -0.5f, -0.5f, -0.5f,  // 0
         0.5f, -0.5f, -0.5f,  // 1
         0.5f,  0.5f, -0.5f,  // 2
        -0.5f,  0.5f, -0.5f,  // 3
        
        // �޸�
        -0.5f, -0.5f,  0.5f,  // 4
         0.5f, -0.5f,  0.5f,  // 5
         0.5f,  0.5f,  0.5f,  // 6
        -0.5f,  0.5f,  0.5f,  // 7
        
        // �Ʒ���
        -0.5f, -0.5f, -0.5f,  // 8
         0.5f, -0.5f, -0.5f,  // 9
         0.5f, -0.5f,  0.5f,  // 10
        -0.5f, -0.5f,  0.5f,  // 11
        
        // ���� 
        -0.5f,  0.5f, -0.5f,  // 12
         0.5f,  0.5f, -0.5f,  // 13
         0.5f,  0.5f,  0.5f,  // 14
        -0.5f,  0.5f,  0.5f,  // 15
        
        // ������
         0.5f, -0.5f, -0.5f,  // 16
         0.5f,  0.5f, -0.5f,  // 17
         0.5f,  0.5f,  0.5f,  // 18
         0.5f, -0.5f,  0.5f,  // 19
        
        // �޸� 
        -0.5f, -0.5f, -0.5f,  // 20
        -0.5f,  0.5f, -0.5f,  // 21
        -0.5f,  0.5f,  0.5f,  // 22
        -0.5f, -0.5f,  0.5f   // 23
    };
    cube.index = {
        // �ո�
        0, 1, 2, 2, 3, 0,
        // �޸�
        4, 5, 6, 6, 7, 4,
        // �Ʒ���
        8, 9, 10, 10, 11, 8,
        // ����
        12, 13, 14, 14, 15, 12,
        // ������
        16, 17, 18, 18, 19, 16,
        // �޸�
        20, 21, 22, 22, 23, 20
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
    // �ʱ�ȭ
    for (int i = 0; i < 6; i++) showCubeFaces[i] = false;
    for (int i = 0; i < 4; i++) showCornFaces[i] = false;
    showCornBottom = false;
    showAllFaces = false;
    
    switch (key) {
    case '1': // ť�� �ո�
        showCubeFaces[0] = true;
        break;
    case '2': // ť�� �޸�
        showCubeFaces[1] = true;
        break;
    case '3': // ť�� �Ʒ���
        showCubeFaces[2] = true;
        break;
    case '4': // ť�� ����
        showCubeFaces[3] = true;
        break;
    case '5': // ť�� ������
        showCubeFaces[4] = true;
        break;
    case '6': // ť�� �޸�
        showCubeFaces[5] = true;
        break;
    case '7': // �簢�� �ո�
        showCornFaces[0] = true;
        break;
    case '8': // �簢�� �޸�
        showCornFaces[1] = true;
        break;
    case '9': // �簢�� ���ʸ�
        showCornFaces[2] = true;
        break;
    case '0': // �簢�� �����ʸ�
        showCornFaces[3] = true;
        break;
    case 'a': // ��� ����
        mode = !mode;
        showAllFaces = true;
        break;
    case 'c': // ť�� ������ 2�� ���� ���
		f1 = cu(gen);
        do {
            f2 = cu(gen);
        } while (f2 == f1);
		showCubeFaces[f1] = true;
		showCubeFaces[f2] = true;
        break;
    case 't': // �簢�� �ٴڰ� ������ �� ���� ���
        showCornBottom = true;
        showCornFaces[co(gen)] = true;
        break;
    case 's': // �ָ��� ���
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        showAllFaces = true;
        break;
    case 'w': // ���̾������� ���
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
    // ���� ȸ�� ��ȯ ���
    glm::mat4 Matrix(1.0f);
    Matrix = glm::rotate(Matrix, glm::radians(-30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    glm::mat4 rotMat(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    Matrix = rotMat * Matrix;
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(Matrix));    
    // ��ǥ�� �׸���
    glBindVertexArray(line.VAO);
    glDrawArrays(GL_LINES, 0, 6);
    // ���� �׸���
    glEnable(GL_DEPTH_TEST);

    if (mode) {
        // ť�� �׸���
        glBindVertexArray(cube.VAO);
        // ��ü ť�� �׸���
        if (showAllFaces) glDrawElements(GL_TRIANGLES, cube.index.size(), GL_UNSIGNED_INT, 0);
        // ���õ� �鸸 �׸���
        else {
            for (int i = 0; i < 6; i++) {
                if (showCubeFaces[i]) {
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
                }
            }
        }
    }
    else {
        // �簢�� �׸���
        glBindVertexArray(corn.VAO);
        // ��ü �簢�� �׸���
        if (showAllFaces) glDrawElements(GL_TRIANGLES, corn.index.size(), GL_UNSIGNED_INT, 0);
        // ���õ� �鸸 �׸���
        else { 
            // �ٴڸ� �׸���
            if (showCornBottom) {
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            }
            // ���õ� ����� �׸���
            for (int i = 0; i < 4; i++) {
                if (showCornFaces[i]) {
                    int startIndex = 6 + (i * 3); // �ٴڸ� 6�� �ε��� �� �� �鸶�� 3����
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }
            }
        }
    }

    glutSwapBuffers();
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}