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
    float color[3];
    int type;
    GLuint VAO, VBO;
};

Shape shapes[10];
int selected = -1, cnt = 0;

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
GLvoid Mouse(int button, int state, int x, int y);
float MouseX(int x) {
    return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
    return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
void InitBuffers(Shape& shape);

void createPoint(float x, float y);
void createLine(float x, float y);
void createTriangle(float x, float y);
void createRectangle(float x, float y);

bool isPointInTriangle(float px, float py, const std::vector<float>& vertices);
bool isPointInRectangle(float px, float py, const std::vector<float>& vertices);
bool isPointNearLine(float px, float py, const std::vector<float>& vertices);
bool isPointNearPoint(float px, float py, const std::vector<float>& vertices);

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
    glutCreateWindow("Example8");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();    
    //--- �ݹ� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMouseFunc(Mouse); // ���콺 �Է�
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

    //glBindVertexArray(0);                       // ���ؽ� �迭 ��ü ���ε� ����
}

// �� ���� �Լ�
void createPoint(float x, float y) {
    Shape point;
    point.type = 0;
    point.vertices = {
        x, y, 0.0f
    };
    
    point.color[0] = dis(gen);
    point.color[1] = dis(gen);
    point.color[2] = dis(gen);

    InitBuffers(point);
    shapes[cnt] = point;
    cnt++;
}
// �� ���� �Լ�
void createLine(float x, float y) {
    Shape line;
    float size = 0.15f;
    line.type = 1;
    line.vertices = {
        x - size, y, 0.0f,
        x + size, y, 0.0f
    };
    
    line.color[0] = dis(gen);
    line.color[1] = dis(gen);
    line.color[2] = dis(gen);
    
    InitBuffers(line);
    shapes[cnt] = line;
	cnt++;
}
// �ﰢ�� ���� �Լ�
void createTriangle(float x, float y) {
    Shape triangle;
    float size = 0.1f;
    triangle.type = 2;
    triangle.vertices = {
        x, y + size, 0.0f,
        x - size, y - size, 0.0f,
        x + size, y - size, 0.0f
    };
    
    triangle.color[0] = dis(gen);
    triangle.color[1] = dis(gen);
    triangle.color[2] = dis(gen);
    
    InitBuffers(triangle);
    shapes[cnt] = triangle;
	cnt++;
}
// �簢�� ���� �Լ�
void createRectangle(float x, float y) {
    Shape rectangle;
    float size = 0.1f;
    rectangle.type = 3;
    rectangle.vertices = {
        x - size, y + size, 0.0f,
        x - size, y - size, 0.0f,
        x + size, y - size, 0.0f,
        
        x - size, y + size, 0.0f,
        x + size, y - size, 0.0f,
        x + size, y + size, 0.0f
    };
    
    rectangle.color[0] = dis(gen);
    rectangle.color[1] = dis(gen);
    rectangle.color[2] = dis(gen);
    
    InitBuffers(rectangle);
    shapes[cnt] = rectangle;
	cnt++;
}

// ���� �� ��ó�� �ִ��� Ȯ��
bool isPointNearPoint(float px, float py, const std::vector<float>& vertices) {    
    float dx = px - vertices[0];
    float dy = py - vertices[1];
    float distance = sqrt(dx * dx + dy * dy);
    
    return distance < 0.05f;
}
// ���� �� ��ó�� �ִ��� Ȯ��
bool isPointNearLine(float px, float py, const std::vector<float>& vertices) {
    float x1 = vertices[0], y1 = vertices[1];
    float x2 = vertices[3], y2 = vertices[4];

    // ���а� �� ������ �Ÿ� ���
    float A = sqrt(pow(x1 - px, 2) + pow(y1 - py, 2));
    float B = sqrt(pow(x2 - px, 2) + pow(y2 - py, 2));
    float line = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

    return abs(line - A - B) < 0.05f;
}
// ���� �ﰢ�� ���ο� �ִ��� Ȯ��
bool isPointInTriangle(float px, float py, const std::vector<float>& vertices) {
    float x1 = vertices[0], y1 = vertices[1];
    float x2 = vertices[3], y2 = vertices[4];
    float x3 = vertices[6], y3 = vertices[7];
    
	float Area = 0.5f * abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
	float a = 0.5f * abs(x1 * (y2 - py) + x2 * (py - y1) + px * (y1 - y2));
    float b = 0.5f * abs(x2 * (y3 - py) + x3 * (py - y2) + px * (y2 - y3));
	float c = 0.5f * abs(x3 * (y2 - y3) + x2 * (y3 - py) + x3 * (py - y2));
    if (abs(Area - (a + b + c)) <= 0.05f) return true;
	else return false;
}
// ���� �簢�� ���ο� �ִ��� Ȯ��
bool isPointInRectangle(float px, float py, const std::vector<float>& vertices) {
    // �簢���� ��� �ڽ� ���
    float minX = vertices[0], maxX = vertices[0];
    float minY = vertices[1], maxY = vertices[1];
    
    for (int i = 0; i < vertices.size(); i += 3) {
        if (vertices[i] < minX) minX = vertices[i];
        if (vertices[i] > maxX) maxX = vertices[i];
        if (vertices[i + 1] < minY) minY = vertices[i + 1];
        if (vertices[i + 1] > maxY) maxY = vertices[i + 1];
    }
    
    return (px >= minX && px <= maxX && py >= minY && py <= maxY);
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
		if (cnt > 9) break;
        createPoint(MouseX(x), MouseY(y));
        break;
    case 'm':
		if (cnt > 9) break;
        createLine(MouseX(x), MouseY(y));
        break;
    case 't':
		if (cnt > 9) break;
        createTriangle(MouseX(x), MouseY(y));
        break;
    case 'r':
		if (cnt > 9) break;
        createRectangle(MouseX(x), MouseY(y));
        break;
    case 'w':
        for (int i = 1; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'a':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 's':
        for (int i = 1; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'd':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.05f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'i':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.02f;
            shapes[selected].vertices[i + 1] += 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'j':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.02f;
            shapes[selected].vertices[i + 1] -= 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'k':
        for(int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] -= 0.02f;
            shapes[selected].vertices[i + 1] += 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'l':
        for (int i = 0; i < shapes[selected].vertices.size(); i += 3) {
            shapes[selected].vertices[i] += 0.02f;
            shapes[selected].vertices[i + 1] -= 0.02f;
        }
        InitBuffers(shapes[selected]);
        break;
    case 'c':
        for (int i = 0; i < cnt; i++) {
            glDeleteVertexArrays(1, &shapes[i].VAO);
            glDeleteBuffers(1, &shapes[i].VBO);
            shapes[i].vertices.clear();
            shapes[i].type = -1;
        }
        selected = -1;
        cnt = 0;
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
        for (int i = cnt - 1; i >= 0; i--) {
            bool clicked = false;
            if (shapes[i].type == 0) { // ��
				clicked = isPointNearPoint(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 1) { // ��
				clicked = isPointNearLine(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 2) { // �ﰢ��
				clicked = isPointInTriangle(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            else if (shapes[i].type == 3) { // �簢��
				clicked = isPointInRectangle(MouseX(x), MouseY(y), shapes[i].vertices);
            }
            if (clicked) {
                selected = i; // ���� ����
				std::cout << "Selected shape index: " << selected << std::endl;
                glutPostRedisplay();
                break;
            }
        }
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
    if(!result) {
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
    
    // ��� ���� �׸���
    for (int i = 0; i < cnt; i++) {
        const auto& shape = shapes[i];
        
        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, shape.color[0], shape.color[1], shape.color[2], 1.0f);
        
        glBindVertexArray(shape.VAO);
        if (shape.type == 0) {
            glPointSize(10.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (shape.type == 1) {
            glLineWidth(3.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_LINES, 0, 2);
        }
        else if (shape.type == 2) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (shape.type == 3) {
            glLineWidth(1.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glBindVertexArray(0);
    }
    
    glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}