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

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// ��ü ����ü
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

//--- ����� ���� �Լ� ����
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Example14");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    firstcreateShape();
    glutTimerFunc(50, TimerFunction, 1);
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
}

// ���� ���� �Լ�
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // ���ؽ� �迭 ��üid ����
    glBindVertexArray(shape.VAO);               // ���ؽ� �迭 ��ü ���ε�

    glGenBuffers(2, shape.VBO);                 // ����id ����

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // ��ǥ
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // ����
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

// ó�� ���� ����
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

// �ﰢ�� ���� �Լ�
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

// Ű���� �ݹ� �Լ�
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

//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
    //--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
    vertexSource = filetobuf("14_vertex.glsl");
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
    fragmentSource = filetobuf("14_fragment.glsl"); // �����׼��̴� �о����
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

    // ��� ���� �׸���
    for (int i = 0; i < 8; i++) {
        glBindVertexArray(shape[i].VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
	}
    glutSwapBuffers();
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

bool big = false;
//--- Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    if (move[0]) { // �ð���� ȸ��
        angle += 0.1f;
        for (int i = 0; i < 8; i++) {
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
        }
    }
    if (move[1]) { // �ݽð���� ȸ��
        angle -= 0.1f;
        for (int i = 0; i < 8; i++) {
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
        }
    }
    if (size) {
        for (int i = 0; i < 8; i++) {
            if (big) { // Ŀ����
                shape[i].size += 0.005f;
				if (shape[i].size >= 0.5f) big = false;
            }
            else { // �۾�����
                shape[i].size -= 0.005f;
				if (shape[i].size <= 0.1f) big = true;
            }
            createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
		}
    }
    glutPostRedisplay();
    // ��� Ÿ�̸Ӹ� ����
    glutTimerFunc(50, TimerFunction, 1);
}