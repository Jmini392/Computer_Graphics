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
    int move = 0;
    float size = 0.1f;
    float centerX, centerY;
    GLuint VAO, VBO;
};
Shape shape[15];
Shape moveshape[10];
bool clicked = false;
bool timerRunning = false; // Ÿ�̸� ���� ���¸� �����ϴ� ����
bool stop = false;
int selected = -1, cnt = 0;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> bis(-0.8f, 0.8f);

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
void Motion(int x, int y);
float MouseX(int x) {
    return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
    return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
void TimerFunction(int value);
void InitBuffers(Shape& shape);

void firstcreateShape();
void startTimer();

void createPoint(Shape& point, float x, float y);
void createLine(Shape& line, float x, float y);
void createTriangle(Shape& triangle, float x, float y);
void createRectangle(Shape& rectangle, float x, float y);
void createPentagon(Shape& pentagon, float x, float y);

bool isPointNearPoint(float px, float py, const std::vector<float>& vertices);
bool isPointNearLine(float px, float py, const std::vector<float>& vertices);
bool isPointInTriangle(float px, float py, const std::vector<float>& vertices);
bool isPointInRectangle(float px, float py, const std::vector<float>& vertices);
bool isPointInPentagon(float px, float py, const std::vector<float>& vertices);


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
    glutInitWindowSize(700, 700);
    glutCreateWindow("Example13");
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
	glutMouseFunc(Mouse); // ���콺 �Է�
	glutMotionFunc(Motion); // ���콺 ������
	startTimer(); // Ÿ�̸� ����
    glutMainLoop();
}

// Ÿ�̸� ���� �Լ�
void startTimer() {
    if (!timerRunning) {
        timerRunning = true;
        stop = false;
        glutTimerFunc(50, TimerFunction, 1);
        std::cout << "Timer started!" << std::endl;
    }
}

// ���� ���� �Լ�
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // ���ؽ� �迭 ��üid ����
    glBindVertexArray(shape.VAO);               // ���ؽ� �迭 ��ü ���ε�

    glGenBuffers(1, &shape.VBO);                // ����id ����

    
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);   // ���۰�ü�� ������ ������ Ÿ�� ����

    // ���ε� �Ŀ� ȣ���ϴ� ��� ���۴� ���ε� �� ���۸� ����Ѵ�.
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    // ����ڰ� ������ �����͸� ���� ���ε��� ���ۿ� �����Ѵ�.

    // ���ؽ� �Ӽ� ������ ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 0�� �Ӽ�, 3���� ��, float��, ����ȭ ����, �� ������ 3���� float, ������ġ 0

    glEnableVertexAttribArray(0);               // ���ؽ� �Ӽ� �迭 ���
}

// ó�� ���� ����
void firstcreateShape() {
    createPoint(shape[0], bis(gen), bis(gen));
    createPoint(shape[1], bis(gen), bis(gen));
    createPoint(shape[2], bis(gen), bis(gen));
    createLine(shape[3], bis(gen), bis(gen));
    createLine(shape[4], bis(gen), bis(gen));
    createLine(shape[5], bis(gen), bis(gen));
    createTriangle(shape[6], bis(gen), bis(gen));
    createTriangle(shape[7], bis(gen), bis(gen));
    createTriangle(shape[8], bis(gen), bis(gen));
    createRectangle(shape[9], bis(gen), bis(gen));
    createRectangle(shape[10], bis(gen), bis(gen));
    createRectangle(shape[11], bis(gen), bis(gen));
    createPentagon(shape[12], bis(gen), bis(gen));
    createPentagon(shape[13], bis(gen), bis(gen));
    createPentagon(shape[14], bis(gen), bis(gen));
}

// �� ���� �Լ�
void createPoint(Shape& point, float x, float y) {
    point.type = 0;
    point.centerX = x;
    point.centerY = y;
    point.vertices = {
        x, y, 0.0f
    };
    point.color[0] = dis(gen);
    point.color[1] = dis(gen);
    point.color[2] = dis(gen);
    InitBuffers(point);
}
// �� ���� �Լ�
void createLine(Shape& line, float x, float y) {
    line.type = 1;
    line.centerX = x;
    line.centerY = y;
    line.vertices = {
        x + line.size, y + line.size, 0.0f,
        x - line.size, y - line.size, 0.0f
    };

    line.color[0] = dis(gen);
    line.color[1] = dis(gen);
    line.color[2] = dis(gen);

    InitBuffers(line);
}
// �ﰢ�� ���� �Լ�
void createTriangle(Shape& triangle, float x, float y) {
    triangle.type = 2;
    triangle.centerX = x;
    triangle.centerY = y;
    triangle.vertices = {
        x, y + triangle.size, 0.0f,
        x - (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f,
        x + (triangle.size / 2) * (float)sqrt(3), y - triangle.size / 2, 0.0f
    };
    triangle.color[0] = dis(gen);
    triangle.color[1] = dis(gen);
    triangle.color[2] = dis(gen);

    InitBuffers(triangle);
}
// �簢�� ���� �Լ�
void createRectangle(Shape& rectangle, float x, float y) {
    rectangle.type = 3;
    rectangle.centerX = x;
    rectangle.centerY = y;
    rectangle.vertices = {
        x - rectangle.size, y + rectangle.size, 0.0f,
        x - rectangle.size, y - rectangle.size, 0.0f,
        x + rectangle.size, y - rectangle.size, 0.0f,

        x - rectangle.size, y + rectangle.size, 0.0f,
        x + rectangle.size, y - rectangle.size, 0.0f,
        x + rectangle.size, y + rectangle.size, 0.0f
    };

    rectangle.color[0] = dis(gen);
    rectangle.color[1] = dis(gen);
    rectangle.color[2] = dis(gen);

    InitBuffers(rectangle);
}
// ������ ���� �Լ�
void createPentagon(Shape& pentagon, float x, float y) {
    const float pi = 3.141592;
    pentagon.type = 4;
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
// ���� ������ ���ο� �ִ��� Ȯ��
bool isPointInPentagon(float px, float py, const std::vector<float>& vertices) {
    int n = vertices.size() / 3; // �������� ������ ����
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = vertices[i * 3], yi = vertices[i * 3 + 1];
        float xj = vertices[j * 3], yj = vertices[j * 3 + 1];
        bool intersect = ((yi > py) != (yj > py)) &&
            (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect)
            inside = !inside;
    }
    return inside;
}
// ���� �ռ� �Լ�
void createMoveShape(float x, float y, int a) {
    int type;
    if ((shape[selected].type + 1) + (shape[a].type + 1) <= 5) {
        type = shape[selected].type + shape[a].type + 1;
    }
	else type = (shape[selected].type + shape[a].type) - 4;
    if (type == 0) createPoint(moveshape[cnt], x, y);
    else if (type == 1) createLine(moveshape[cnt], x, y);
    else if (type == 2) createTriangle(moveshape[cnt], x, y);
    else if (type == 3) createRectangle(moveshape[cnt], x, y);
    else if (type == 4) createPentagon(moveshape[cnt], x, y);
    
    shape[a].vertices.clear();
    shape[selected].vertices.clear();
    shape[a].type = -1;
    shape[selected].type = -1;

    cnt++;
    selected = -1;
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c':
        // ���� �������� �ٽ� �ʱ�ȭ
        for (int i = 0; i < 15; i++) {
            shape[i].type = -1;
            shape[i].vertices.clear();
        }
        // ������ �����鵵 �ʱ�ȭ
        for (int i = 0; i < cnt; i++) {
            moveshape[i].type = -1;
            moveshape[i].vertices.clear();
        }
		firstcreateShape();
		cnt = 0;
		selected = -1;
		clicked = false;
        break;
    case 's':
        if (stop) {
            stop = false;
            std::cout << "Animation resumed" << std::endl;
        }
        else {
            stop = true;
            std::cout << "Animation stopped" << std::endl;
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

    // ��� ���� �׸���
    for (int i = 0; i < 15; i++) {
        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, shape[i].color[0], shape[i].color[1], shape[i].color[2], 1.0f);
        glBindVertexArray(shape[i].VAO);
		if (shape[i].type == -1) continue;
        if (shape[i].type == 0) {
            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (shape[i].type == 1) {
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, 2);
        }
        else if (shape[i].type == 2) {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (shape[i].type == 3) {
            glLineWidth(1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (shape[i].type == 4) {
            glDrawArrays(GL_TRIANGLES, 0, 9);
        }

        glBindVertexArray(0);
    }
    for (int i = 0; i < cnt; i++) {
        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, moveshape[i].color[0], moveshape[i].color[1], moveshape[i].color[2], 1.0f);
        glBindVertexArray(moveshape[i].VAO);
        if (moveshape[i].type == 0) {
            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (moveshape[i].type == 1) {
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, 2);
        }
        else if (moveshape[i].type == 2) {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        else if (moveshape[i].type == 3) {
            glLineWidth(1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (moveshape[i].type == 4) {
            glDrawArrays(GL_TRIANGLES, 0, 9);
        }

        glBindVertexArray(0);
    }

    glutSwapBuffers();
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (int i = 14; i >= 0; i--) {
            if (shape[i].type == 0) { // ��
                clicked = isPointNearPoint(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 1) { // ��
                clicked = isPointNearLine(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 2) { // �ﰢ��
                clicked = isPointInTriangle(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 3) { // �簢��
                clicked = isPointInRectangle(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 4) { // ������
                clicked = isPointInPentagon(MouseX(x), MouseY(y), shape[i].vertices);
			}
            if (clicked) {
                selected = i; // ���� ����
                std::cout << "Selected shape index: " << selected << std::endl;
                glutPostRedisplay();
                break;
            }
        }
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		clicked = false;
        for (int i = 14; i >= 0; i--) {
            bool isHit = false;
			if (i == selected) continue;
            if (shape[i].type == 0) {
                isHit = isPointNearPoint(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 1) {
                isHit = isPointNearLine(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 2) {
                isHit = isPointInTriangle(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 3) {
                isHit = isPointInRectangle(MouseX(x), MouseY(y), shape[i].vertices);
            }
            else if (shape[i].type == 4) {
                isHit = isPointInPentagon(MouseX(x), MouseY(y), shape[i].vertices);
            }

            if (isHit) {
                std::cout << "Merging with shape: " << i << " (type: " << shape[i].type << ")" << std::endl;
                createMoveShape(MouseX(x), MouseY(y), i);
                break;
            }
        }
    }
    glutPostRedisplay();
}

void Motion(int x, int y) {
    if (clicked && selected != -1) {
        float dx = MouseX(x) - shape[selected].centerX;
        float dy = MouseY(y) - shape[selected].centerY;
        shape[selected].centerX = MouseX(x);
        shape[selected].centerY = MouseY(y);
        for (int i = 0; i < shape[selected].vertices.size(); i += 3) {
            shape[selected].vertices[i] += dx;
            shape[selected].vertices[i + 1] += dy;
        }
        InitBuffers(shape[selected]);
    }
    glutPostRedisplay();
}

//--- Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    if (stop) {
        glutTimerFunc(50, TimerFunction, 1);
        return;
    }
    for (int i = 0; i < cnt; i++) {
        if (moveshape[i].type == -1 || moveshape[i].vertices.empty()) continue;

        // ������ ��� �ڽ� ��� (��� ������ �����ϰ� �˻�)
        float minX = moveshape[i].vertices[0], maxX = moveshape[i].vertices[0];
        float minY = moveshape[i].vertices[1], maxY = moveshape[i].vertices[1];

        for (int j = 0; j < moveshape[i].vertices.size(); j += 3) {
            if (moveshape[i].vertices[j] < minX) minX = moveshape[i].vertices[j];
            if (moveshape[i].vertices[j] > maxX) maxX = moveshape[i].vertices[j];
            if (moveshape[i].vertices[j + 1] < minY) minY = moveshape[i].vertices[j + 1];
            if (moveshape[i].vertices[j + 1] > maxY) maxY = moveshape[i].vertices[j + 1];
        }

        // ��� �浹 �˻� �� ���� ����
        if (maxY >= 1.0f && (moveshape[i].move == 0 || moveshape[i].move == 3)) {
            moveshape[i].move = (moveshape[i].move == 0) ? 1 : 2; // �Ʒ�������
        }
        if (maxX >= 1.0f && (moveshape[i].move == 0 || moveshape[i].move == 1)) {
            moveshape[i].move = (moveshape[i].move == 0) ? 3 : 2; // ��������
        }
        if (minY <= -1.0f && (moveshape[i].move == 1 || moveshape[i].move == 2)) {
            moveshape[i].move = (moveshape[i].move == 1) ? 0 : 3; // ��������
        }
        if (minX <= -1.0f && (moveshape[i].move == 2 || moveshape[i].move == 3)) {
            moveshape[i].move = (moveshape[i].move == 2) ? 1 : 0; // ����������
        }

        // �̵� ���� (��� ������ �����ϰ� ó��)
        float moveSpeed = 0.02f;
        for (int j = 0; j < moveshape[i].vertices.size(); j += 3) {
            switch (moveshape[i].move) {
            case 0: // �����
                moveshape[i].vertices[j] += moveSpeed;
                moveshape[i].vertices[j + 1] += moveSpeed;
                break;
            case 1: // ������
                moveshape[i].vertices[j] += moveSpeed;
                moveshape[i].vertices[j + 1] -= moveSpeed;
                break;
            case 2: // ������
                moveshape[i].vertices[j] -= moveSpeed;
                moveshape[i].vertices[j + 1] -= moveSpeed;
                break;
            case 3: // �»���
                moveshape[i].vertices[j] -= moveSpeed;
                moveshape[i].vertices[j + 1] += moveSpeed;
                break;
            }
        }

        // �߽��� ������Ʈ (���� Ÿ�Կ� �������)
        float centerX = 0, centerY = 0;
        int numVertices = moveshape[i].vertices.size() / 3;
        for (int j = 0; j < moveshape[i].vertices.size(); j += 3) {
            centerX += moveshape[i].vertices[j];
            centerY += moveshape[i].vertices[j + 1];
        }
        moveshape[i].centerX = centerX / numVertices;
        moveshape[i].centerY = centerY / numVertices;

        InitBuffers(moveshape[i]);
    }
    glutPostRedisplay();
    // ��� Ÿ�̸Ӹ� ����
    glutTimerFunc(50, TimerFunction, 1);
}