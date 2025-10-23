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

// ��ü ����ü
struct Shape {
    std::vector<float> vertices;  // �������� ��� ������ ����
    GLuint VAO, VBO;
    int currentDrawCount = 0;  // �׷��� �� ���� ����
    int totalPoints = 0;       // ��ü ���� ����
    bool isAnimating = false;  // �ִϸ��̼� ������
};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> bis(-0.8f, 0.7f);

Shape shape[6];
float R = 1.0f, G = 1.0f, B = 1.0f; // ������
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

//--- ����� ���� �Լ� ����
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
    glutInitWindowSize(600, 600);
    glutCreateWindow("Example11");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();

    // Ÿ�̸� ����
    glutTimerFunc(50, TimerFunction, 1);

    //--- �ݹ� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMouseFunc(Mouse); // ���콺 �Է�
    glutMainLoop();
}

// ���� ���� �Լ�
GLvoid InitBuffers(Shape& shape) {
    if (shape.VAO != 0) {
        glDeleteVertexArrays(1, &shape.VAO);
        glDeleteBuffers(1, &shape.VBO);
    }

    glGenVertexArrays(1, &shape.VAO);           // ���ؽ� �迭 ��üid ����
    glGenBuffers(1, &shape.VBO);                // ����id ����

    glBindVertexArray(shape.VAO);               // ���ؽ� �迭 ��ü ���ε�
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);   // ���۰�ü�� ������ ������ Ÿ�� ����

    // ���ε� �Ŀ� ȣ���ϴ� ��� ���۴� ���ε� �� ���۸� ����Ѵ�.
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    // ����ڰ� ������ �����͸� ���� ���ε��� ���ۿ� �����Ѵ�.

    // ���ؽ� �Ӽ� ������ ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 0�� �Ӽ�, 3���� ��, float��, ����ȭ ����, �� ������ 3���� float, ������ġ 0

    glEnableVertexAttribArray(0);               // ���ؽ� �Ӽ� �迭 ���
}

/*void createConnectedSpiral(float x, float y, int a) {
	//R = dis(gen), G = dis(gen), B = dis(gen); // ���� ���� ����
    shape[a].vertices.clear();
    //shape[a].centerX = x;
    //shape[a].centerY = y;

    const int pointsPerSpiral = 60;  // �� ������ �� ����
    const float maxRadius = 0.2f;    // �ִ� ������
    const float spiralTurns = 3.0f;  // 3����

    // ù ��° ����: Ŭ���� ������ �ݽð�������� �ٱ����� 3����
    float firstSpiralEndX, firstSpiralEndY;
    
    for (int i = 0; i < pointsPerSpiral; i++) {
        float t = (float)i / (pointsPerSpiral - 1);  // 0.0 ~ 1.0
        float angle = t * spiralTurns * 2.0f * 3.14159f;  // �ݽð���� ���� (���)
        float radius = maxRadius * t;  // �ȿ��� �ٱ����� (������ ����)

        float dx = x + radius * cos(angle);
        float dy = y + radius * sin(angle);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);

        // ������ ���� ��ǥ�� ���� (ù ��° ������ ����)
        if (i == pointsPerSpiral - 1) {
            firstSpiralEndX = dx;
            firstSpiralEndY = dy;
        }
    }

    // �� ��° �����̷��� �߽��� ��� (ù ��° �����̷��� �����ʿ� ��ġ)
    float distance = maxRadius * 2.0f;  // �� �������� ��
    float secondCenterX = x + distance;  // ù ��° �߽��� �����ʿ� ��ġ
    float secondCenterY = y;             // ���� ����

    // �� ��° �����̷�: ���ο� �߽������� 180������ �����Ͽ� �ð�������� ������ 3����
    for (int i = 0; i < pointsPerSpiral; i++) {
        float t = (float)i / (pointsPerSpiral - 1);  // 0.0 ~ 1.0
        
        // 180��(�� ����)���� �����Ͽ� �ð�������� ȸ�� (���� ����)
        float startAngle = 3.14159f;  // 180��
        float angle = startAngle - t * spiralTurns * 2.0f * 3.14159f;  // �ð���� ȸ��
        float radius = maxRadius * (1.0f - t);  // �ٱ����� ������ (������ ����)

        // �� ��° �߽����� �������� �� ��° ���� ����
        float dx = secondCenterX + radius * cos(angle);
        float dy = secondCenterY + radius * sin(angle);

        shape[a].vertices.push_back(dx);
        shape[a].vertices.push_back(dy);
        shape[a].vertices.push_back(0.0f);
    }

    shape[a].totalPoints = pointsPerSpiral * 2;  // ��ü ���� ���� (�� ����)
    shape[a].currentDrawCount = 0;
    //shape[a].isActive = true;
    shape[a].isAnimating = true;

    InitBuffers(shape[a]);
}*/

// ������ ���� �Լ�
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
	std::cout << shape[a].totalPoints << "�� ������" << std::endl;
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
    std::cout << shape[a].totalPoints << "�� ������" << std::endl;

    shape[a].isAnimating = true;
    InitBuffers(shape[a]);
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        if (!clicked) selectes = 1;
        std::cout << selectes << "���� ����" << std::endl;
        break;
    case '2':
        if (!clicked) selectes = 2;
        std::cout << selectes << "���� ����" << std::endl;
        break;
    case '3':
        if (!clicked) selectes = 3;
        std::cout << selectes << "���� ����" << std::endl;
        break;
    case '4':
        if (!clicked) selectes = 4;
        std::cout << selectes << "���� ����" << std::endl;
        break;
    case '5':
        if (!clicked) selectes = 5;
        std::cout << selectes << "���� ����" << std::endl;
        break;
    case 'p':
        isPoint = true;
        break;
    case 'l':
        isPoint = false;
        break;
    case 'c':
        // ��� �����̷� �ʱ�ȭ
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

// ���콺 �ݹ� �Լ�
GLvoid Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!clicked) {
            R = dis(gen), G = dis(gen), B = dis(gen); // ���� ���� ����
            for (int i = 1; i <= selectes; i++) {
                std::cout << "���콺 Ŭ���� - ����� ���� ���� ����" << std::endl;
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

//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
    //--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
    vertexSource = filetobuf("8_vertex.glsl");
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
    fragmentSource = filetobuf("8_fragment.glsl"); // �����׼��̴� �о����
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

    glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}

//--- Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    for (int i = 1; i <= selectes; i++) {
        if (shape[i].isAnimating) {
            if (shape[i].currentDrawCount < shape[i].totalPoints) shape[i].currentDrawCount++;
            else shape[i].isAnimating = false;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);  // ��� Ÿ�̸� ����
}