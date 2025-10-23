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

// Ÿ�̸� ���� ���¸� �߰�
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

void createLine(Shape& line, float x, float y);
void createTriangle(Shape& triangle, float x, float y);
void createRectangle(Shape& rectangle, float x, float y);
void createPentagon(Shape& pentagon, float x, float y);

void LinetoTriangle(Shape& line);
void TriangletoRectangle(Shape& triangle);
void RectangletoPentagon(Shape& rectangle);
void PentagontoLine(Shape& pentagon);

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
    glutCreateWindow("Example12");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
	firstcreateShape();
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
}

// ���� ���� �Լ�
GLvoid InitBuffers(Shape& shape) {
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

// ���� ���� �Լ�
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
// �� ���� �Լ�
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
// �ﰢ�� ���� �Լ�
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
// �簢�� ���� �Լ�
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
// ������ ���� �Լ�
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

// ������ �ﰢ�� �׸���
void LinetoTriangle(Shape& line) {
    line.type = 1;
    std::cout << "Line center: " << line.centerX << " " << line.centerY << std::endl;
    // steps ���͸� ������ ũ��� �ʱ�ȭ
    line.steps.clear();
    line.steps.resize(line.vertices.size(), 0.0f);
    
    // ��ǥ �ﰢ�� ��ǥ ���
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

    // �� ���� ���� �̵� ���� ���
    for (int i = 0; i < line.vertices.size(); i += 3) {
        if (i + 2 < line.vertices.size() && i + 2 < goalVertices.size() && i + 2 < line.steps.size()) {
            line.steps[i] = (goalVertices[i] - line.vertices[i]) / 10.0f;
            line.steps[i + 1] = (goalVertices[i + 1] - line.vertices[i + 1]) / 10.0f;
            std::cout << line.steps[i] << line.steps[i + 1] << std::endl;
        }
    }
    
    line.cnt = 0;  // �ִϸ��̼� ī���� �ʱ�ȭ
    
    // Ÿ�̸Ӱ� ���� ���� �ƴ� ���� Ÿ�̸� ����
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// �ﰢ������ �簢�� �׸���
void TriangletoRectangle(Shape& triangle) {
    triangle.type = 2;
    // steps ���͸� ������ ũ��� �ʱ�ȭ
    triangle.steps.clear();
    triangle.steps.resize(triangle.vertices.size(), 0.0f);
    // ��ǥ �簢�� ��ǥ ���
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
    // �� ���� ���� �̵� ���� ���
    for (int i = 0; i < triangle.vertices.size(); i += 3) {
        if (i + 2 < triangle.vertices.size() && i + 2 < goalVertices.size() && i + 2 < triangle.steps.size()) {
            std::cout << "Point " << i / 3 << " - Current: (" << triangle.vertices[i] << ", " << triangle.vertices[i + 1] << ")" << std::endl;
            std::cout << "Point " << i / 3 << " - Goal: (" << goalVertices[i] << ", " << goalVertices[i + 1] << ")" << std::endl;
            triangle.steps[i] = (goalVertices[i] - triangle.vertices[i]) / 10.0f;
            triangle.steps[i + 1] = (goalVertices[i + 1] - triangle.vertices[i + 1]) / 10.0f;
            //triangle.steps[i + 2] = 0.0f;
        }
    }
    triangle.cnt = 0;  // �ִϸ��̼� ī���� �ʱ�ȭ
    
    // Ÿ�̸Ӱ� ���� ���� �ƴ� ���� Ÿ�̸� ����
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// �簢������ ������ �׸���
void RectangletoPentagon(Shape& rectangle) {
    rectangle.type = 3;
    const float pi = 3.141592;
	// steps ���͸� ������ ũ��� �ʱ�ȭ
	rectangle.steps.clear();
	rectangle.steps.resize(rectangle.vertices.size(), 0.0f);
	// ��ǥ ������ ��ǥ ���
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
	// �� ���� ���� �̵� ���� ���
    for (int i = 0; i < rectangle.vertices.size(); i += 3) {
        if (i + 2 < rectangle.vertices.size() && i + 2 < goalVertices.size() && i + 2 < rectangle.steps.size()) {
            std::cout << "Point " << i / 3 << " - Current: (" << rectangle.vertices[i] << ", " << rectangle.vertices[i + 1] << ")" << std::endl;
            std::cout << "Point " << i / 3 << " - Goal: (" << goalVertices[i] << ", " << goalVertices[i + 1] << ")" << std::endl;
            rectangle.steps[i] = (goalVertices[i] - rectangle.vertices[i]) / 10.0f;
            rectangle.steps[i + 1] = (goalVertices[i + 1] - rectangle.vertices[i + 1]) / 10.0f;
        }
    }
	rectangle.cnt = 0;  // �ִϸ��̼� ī���� �ʱ�ȭ
    
    // Ÿ�̸Ӱ� ���� ���� �ƴ� ���� Ÿ�̸� ����
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}
// ���������� ������ �׸���
void PentagontoLine(Shape& pentagon) {
	std::cout << "Pentagon center: " << pentagon.centerX << " " << pentagon.centerY << std::endl;
	// steps ���͸� ������ ũ��� �ʱ�ȭ
	pentagon.steps.clear();
	pentagon.steps.resize(pentagon.vertices.size(), 0.0f);
	// ��ǥ �� ��ǥ ���
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
	// �� ���� ���� �̵� ���� ���
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
    
    // Ÿ�̸Ӱ� ���� ���� �ƴ� ���� Ÿ�̸� ����
    if (!timerRunning) {
        timerRunning = true;
        glutTimerFunc(100, TimerFunction, 1);
    }
}

// Ű���� �ݹ� �Լ�
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
    // ����� ������� ����
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

        // ��� ���� �׸���
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

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

//--- Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    bool animationActive = false;
    if (!extense) {
        for (int i = 0; i < 4; i++) {
            if (shape[i].type == 1 && shape[i].cnt < 10) {  // �ﰢ���̰� �ִϸ��̼��� ���� �Ϸ���� �ʾ�����
                animationActive = true;
                // �� ���� ��ǥ ��ġ�� �̵�
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X ��ǥ ������Ʈ
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y ��ǥ ������Ʈ
                    }
                }
                shape[i].cnt++;  // �ִϸ��̼� ���� �ܰ� ����
                InitBuffers(shape[i]);  // ���� ������Ʈ
            }
            if (shape[i].type == 1 && shape[i].cnt >= 10) {
				createTriangle(shape[i], shape[i].centerX, shape[i].centerY);
                TriangletoRectangle(shape[i]);
            }
            if (shape[i].type == 2 && shape[i].cnt < 10) {  // �簢���̰� �ִϸ��̼��� ���� �Ϸ���� �ʾ�����
                animationActive = true;
                // �� ���� ��ǥ ��ġ�� �̵�
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X ��ǥ ������Ʈ
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y ��ǥ ������Ʈ
                    }
                }
                shape[i].cnt++;  // �ִϸ��̼� ���� �ܰ� ����
                InitBuffers(shape[i]);  // ���� ������Ʈ
            }
            if (shape[i].type == 2 && shape[i].cnt >= 10) {
				createRectangle(shape[i], shape[i].centerX, shape[i].centerY);
                RectangletoPentagon(shape[i]);
            }
            if (shape[i].type == 3 && shape[i].cnt < 10) {  // �������̰� �ִϸ��̼��� ���� �Ϸ���� �ʾ�����
                animationActive = true;
                // �� ���� ��ǥ ��ġ�� �̵�
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X ��ǥ ������Ʈ
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y ��ǥ ������Ʈ
                    }
                }
                shape[i].cnt++;  // �ִϸ��̼� ���� �ܰ� ����
                InitBuffers(shape[i]);  // ���� ������Ʈ
            }
            if (shape[i].type == 3 && shape[i].cnt >= 10) {
				createPentagon(shape[i], shape[i].centerX, shape[i].centerY);
                PentagontoLine(shape[i]);
            }
            if (shape[i].type == 0 && shape[i].cnt < 10) {  // ���̰� �ִϸ��̼��� ���� �Ϸ���� �ʾ�����
                animationActive = true;
                // �� ���� ��ǥ ��ġ�� �̵�
                for (int j = 0; j < shape[i].vertices.size() && j < shape[i].steps.size(); j += 3) {
                    if (j + 2 < shape[i].vertices.size() && j + 2 < shape[i].steps.size()) {
                        shape[i].vertices[j] += shape[i].steps[j];       // X ��ǥ ������Ʈ
                        shape[i].vertices[j + 1] += shape[i].steps[j + 1]; // Y ��ǥ ������Ʈ
                    }
                }
                shape[i].cnt++;  // �ִϸ��̼� ���� �ܰ� ����
                InitBuffers(shape[i]);  // ���� ������Ʈ
            }
            if (shape[i].type == 0 && shape[i].cnt >= 10 && shape[i].ptol) {
                shape[i].ptol = false;
				createLine(shape[i], shape[i].centerX, shape[i].centerY);
                LinetoTriangle(shape[i]);
            }
        }
    }
    
    // �ִϸ��̼��� ���� ���̸� Ÿ�̸� ��� ����
    else {
        if (Bigshape.cnt < 10) {  // �ﰢ���̰� �ִϸ��̼��� ���� �Ϸ���� �ʾ�����
            animationActive = true;
            // �� ���� ��ǥ ��ġ�� �̵�
            for (int j = 0; j < Bigshape.vertices.size() && j < Bigshape.steps.size(); j += 3) {
                if (j + 2 < Bigshape.vertices.size() && j + 2 < Bigshape.steps.size()) {
                    Bigshape.vertices[j] += Bigshape.steps[j];       // X ��ǥ ������Ʈ
                    Bigshape.vertices[j + 1] += Bigshape.steps[j + 1]; // Y ��ǥ ������Ʈ
                }
            }
            Bigshape.cnt++;  // �ִϸ��̼� ���� �ܰ� ����
            InitBuffers(Bigshape);  // ���� ������Ʈ
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
        // ��� �ִϸ��̼��� �Ϸ�Ǹ� Ÿ�̸� �÷��׸� false�� ����
        timerRunning = false;
    }
}