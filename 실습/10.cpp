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
    int type, move = 0, state = 0;
    int count = 0;
    float max = 1.0, min = -1.0;
	float radian, alpha = 0;
    GLuint VAO, VBO;
};

Shape shapes[16];
bool filled = true, start = true;
bool one = false, two = false, three = false, four = false;
bool timerRunning = false; // Ÿ�̸� ���� ���¸� �����ϴ� ����
int total = 4;
int cnt[4] = { 0, };

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> mis(0.1f, 0.2f);

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
void createTriangle(float x, float y);

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
    glutCreateWindow("Example10");
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
}

// �ﰢ�� ���� �Լ�
void createTriangle(float x, float y, int a) {
    Shape triangle;
    float size = 0.1f;
    float tsize = 0.2f;
    if (a < 4) tsize = mis(gen), size = mis(gen);
    triangle.vertices = {
        x, y + tsize, 0.0f,
        x - size, y - size, 0.0f,
        x + size, y - size, 0.0f
    };
    if (x > 0 && y > 0) triangle.type = 0;
    else if (x < 0 && y > 0) triangle.type = 1;
    else if (x < 0 && y < 0) triangle.type = 2;
    else if (x > 0 && y < 0) triangle.type = 3;
    triangle.color[0] = dis(gen);
    triangle.color[1] = dis(gen);
    triangle.color[2] = dis(gen);

    InitBuffers(triangle);
    shapes[a] = triangle;
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
		if (one) one = false;
        else {
            one = true;
			two = three = four = false;
            if (!timerRunning) {
                timerRunning = true;
                glutTimerFunc(50, TimerFunction, 1);
            }
        }
		break;
	case '2':
        if (two) two = false;
        else {
            two = true;
			one = three = four = false;
            if (!timerRunning) {
                timerRunning = true;
                glutTimerFunc(50, TimerFunction, 1);
            }
		}
        break;
	case '3':
        if (three) three = false;
        else {
            three = true;
			one = two = four = false;
            if (!timerRunning) {
                timerRunning = true;
                glutTimerFunc(50, TimerFunction, 1);
			}
        }
        break;
	case '4':
        if (four) four = false;
        else {
			four = true;
            one = two = three = false;
            for (int i = 0; i < 4; i++) shapes[i].radian = atan2(shapes[i].vertices[1], shapes[i].vertices[0]);
            if (!timerRunning) {
                timerRunning = true;
                glutTimerFunc(50, TimerFunction, 1);
            }
        }
		break;
    case 'a':
        filled = true;
        break;
    case 'b':
        filled = false;
        break;
    case 'c':
        start = true;
        for (int i = 4; i < total; i++) {
            glDeleteVertexArrays(1, &shapes[i].VAO);
            glDeleteBuffers(1, &shapes[i].VBO);
            shapes[i].vertices.clear();
        }
        for (int i = 0; i < 12; i++) {
			shapes[i].count = 0;
			shapes[i].move = 0;
			shapes[i].state = 0;
			shapes[i].max = 1.0;
			shapes[i].min = -1.0;
		}
        cnt[0] = 0; cnt[1] = 0; cnt[2] = 0; cnt[3] = 0;
        total = 4;
		one = two = three = four = false;
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
        if (MouseX(x) > 0 && MouseY(y) > 0) {
            glDeleteVertexArrays(1, &shapes[0].VAO);
            glDeleteBuffers(1, &shapes[0].VBO);
            shapes[0].vertices.clear();
            createTriangle(MouseX(x), MouseY(y), 0);
            shapes[0].radian = atan2(shapes[0].vertices[1], shapes[0].vertices[0]);
        }
        if (MouseX(x) < 0 && MouseY(y) > 0) {
            glDeleteVertexArrays(1, &shapes[1].VAO);
            glDeleteBuffers(1, &shapes[1].VBO);
            shapes[1].vertices.clear();
            createTriangle(MouseX(x), MouseY(y), 1);
            shapes[1].radian = atan2(shapes[1].vertices[1], shapes[1].vertices[0]);
        }
        if (MouseX(x) < 0 && MouseY(y) < 0) {
            glDeleteVertexArrays(1, &shapes[2].VAO);
            glDeleteBuffers(1, &shapes[2].VBO);
            shapes[2].vertices.clear();
            createTriangle(MouseX(x), MouseY(y), 2);
            shapes[2].radian = atan2(shapes[2].vertices[1], shapes[2].vertices[0]);
        }
        if (MouseX(x) > 0 && MouseY(y) < 0) {
            glDeleteVertexArrays(1, &shapes[3].VAO);
            glDeleteBuffers(1, &shapes[3].VBO);
            shapes[3].vertices.clear();
            createTriangle(MouseX(x), MouseY(y), 3);
            shapes[3].radian = atan2(shapes[3].vertices[1], shapes[3].vertices[0]);
        }
        glutPostRedisplay();
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (total >= 16) return;
        if ((MouseX(x) > 0 && MouseY(y) > 0) && cnt[0] >= 3) return;
        if ((MouseX(x) < 0 && MouseY(y) > 0) && cnt[1] >= 3) return;
        if ((MouseX(x) < 0 && MouseY(y) < 0) && cnt[2] >= 3) return;
        if ((MouseX(x) > 0 && MouseY(y) < 0) && cnt[3] >= 3) return;
        createTriangle(MouseX(x), MouseY(y), total);
        shapes[total].radian = atan2(shapes[total].vertices[1], shapes[total].vertices[0]);
        std::cout << cnt[0] << cnt[1] << cnt[2] << cnt[3] << std::endl;
        cnt[shapes[total].type]++;
        total++;
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
    // ����� ������� ����
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);

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

    if (start) {
        createTriangle(0.5f, 0.5f, 0);
        createTriangle(-0.5f, 0.5f, 1);
        createTriangle(-0.5f, -0.5f, 2);
        createTriangle(0.5f, -0.5f, 3);
    }
    start = false;
    GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
    glUniform4f(colorLocation, line.color[0], line.color[1], line.color[2], 1.0f);
    glBindVertexArray(line.VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, 4);

    for (int i = 0; i < total; i++) {
        const auto& shape = shapes[i];

        GLint colorLocation = glGetUniformLocation(shaderProgramID, "shapeColor");
        glUniform4f(colorLocation, shape.color[0], shape.color[1], shape.color[2], 1.0f);

        glBindVertexArray(shape.VAO);
        if (!filled) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);
    }

    glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
    glViewport(0, 0, w, h);
}

void TimerFunction(int value) {
    // ��� �ִϸ��̼��� ��Ȱ��ȭ�Ǿ��� �� Ÿ�̸� ����
    if (!one && !two && !three && !four) {
        timerRunning = false;
        return;
    }
    if (one) {
        for (int i = 0; i < total; i++) {
            if (shapes[i].vertices[1] >= 1.0f) {
                shapes[i].move = 1;
            }
            if (shapes[i].vertices[6] >= 1.0f) {
                shapes[i].move = 2;
            }
            if (shapes[i].vertices[4] <= -1.0f) {
                shapes[i].move = 3;
            }
            if (shapes[i].vertices[3] <= -1.0f) {
                shapes[i].move = 0;
            }
            for (int j = 0; j < shapes[i].vertices.size(); j += 3) {
                if (shapes[i].move == 0) {
                    shapes[i].vertices[j] += 0.05f;
                    shapes[i].vertices[j + 1] += 0.05f;
                }
                if (shapes[i].move == 1) {
                    shapes[i].vertices[j] += 0.05f;
                    shapes[i].vertices[j + 1] -= 0.05f;
                }
                if (shapes[i].move == 2) {
                    shapes[i].vertices[j] -= 0.05f;
                    shapes[i].vertices[j + 1] -= 0.05f;
                }
                if (shapes[i].move == 3) {
                    shapes[i].vertices[j] -= 0.05f;
                    shapes[i].vertices[j + 1] += 0.05f;
                }
            }
            InitBuffers(shapes[i]);
        }
    }
    if (two) {
		for (int i = 0; i < total; i++) {
            if (shapes[i].move == 0 && shapes[i].vertices[6] >= 1.0f) {
				shapes[i].move = 2;
                if (shapes[i].vertices[7] <= -1.0f) shapes[i].state = 1;
                if (shapes[i].vertices[1] >= 1.0f) shapes[i].state = 0;
            }
            if (shapes[i].move == 1 && shapes[i].vertices[3] <= -1.0f) {
				shapes[i].move = 3;
                if (shapes[i].vertices[7] <= -1.0f) shapes[i].state = 1;
                if (shapes[i].vertices[1] >= 1.0f) shapes[i].state = 0;
            }
            if (shapes[i].count >= 12) {
                if (shapes[i].move == 2) shapes[i].move = 1;
				if (shapes[i].move == 3) shapes[i].move = 0;
				shapes[i].count = 0;
			}
            for (int j = 0; j < shapes[i].vertices.size(); j += 3) {
                if (shapes[i].move == 0) {
                    shapes[i].vertices[j] += 0.05f;
				}
                if (shapes[i].move == 1) {
                    shapes[i].vertices[j] -= 0.05f;
                }
                if (shapes[i].move == 2 || shapes[i].move == 3) {
                    if (shapes[i].state == 0) shapes[i].vertices[j + 1] -= 0.05f;
                    if (shapes[i].state == 1) shapes[i].vertices[j + 1] += 0.05f;
                    shapes[i].count++;
				}
            }
            InitBuffers(shapes[i]);
		}
    }
    if (three) {
        for (int i = 0; i < total; i++) {
			if (shapes[i].max <= 0.1f || shapes[i].min >= -0.1f) shapes[i].state = 1;
			if (shapes[i].max >= 1.0f&& shapes[i].min <= -1.0f) shapes[i].state = 0, shapes[i].max = 1.0, shapes[i].min = -1.0;
            if (shapes[i].state == 0) {
                if (shapes[i].move == 0 && shapes[i].vertices[3] <= shapes[i].min) shapes[i].move = 1, shapes[i].min += 0.1f;
                if (shapes[i].move == 1 && shapes[i].vertices[4] <= shapes[i].min) shapes[i].move = 2, shapes[i].min += 0.1f;
                if (shapes[i].move == 2 && shapes[i].vertices[6] >= shapes[i].max) shapes[i].move = 3, shapes[i].max -= 0.1f;
                if (shapes[i].move == 3 && shapes[i].vertices[1] >= shapes[i].max) shapes[i].move = 0, shapes[i].max -= 0.1f;
            }
            if (shapes[i].state == 1) {
                if (shapes[i].move == 2 && shapes[i].vertices[6] >= shapes[i].max) shapes[i].move = 1, shapes[i].max += 0.1f;
                if (shapes[i].move == 1 && shapes[i].vertices[4] <= shapes[i].min) shapes[i].move = 0, shapes[i].min -= 0.1f;
                if (shapes[i].move == 0 && shapes[i].vertices[3] <= shapes[i].min) shapes[i].move = 3, shapes[i].min -= 0.1f;
                if (shapes[i].move == 3 && shapes[i].vertices[1] >= shapes[i].max) shapes[i].move = 2, shapes[i].max += 0.1f;
			}
            
            for (int j = 0; j < shapes[i].vertices.size(); j += 3) {
                if (shapes[i].move == 0) shapes[i].vertices[j] -= 0.05f;
                if (shapes[i].move == 1) shapes[i].vertices[j + 1] -= 0.05f;
				if (shapes[i].move == 2) shapes[i].vertices[j] += 0.05f;
				if (shapes[i].move == 3) shapes[i].vertices[j + 1] += 0.05f;
            }
            InitBuffers(shapes[i]);
        }
    }
    if (four) {
        for (int i = 0; i < total; i++) {
            // ���� �ﰢ���� �߽��� ���
            float cenX = (shapes[i].vertices[0] + shapes[i].vertices[3] + shapes[i].vertices[6]) / 3.0f;
            float cenY = (shapes[i].vertices[1] + shapes[i].vertices[4] + shapes[i].vertices[7]) / 3.0f;
            float disnt = sqrt(cenX * cenX + cenY * cenY);

            // ���� ��ȯ ����
            if (disnt <= 0.01f) shapes[i].state = 1, disnt = 0.02f, shapes[i].alpha = 0.0f; // �ٱ�����
            if (disnt >= 1.1f) shapes[i].state = 0, disnt = 1.09f, shapes[i].alpha = 0.0f; // ������

            // radian�� ������, alpha�� ������ �������� ���
            // ���ο� �߽��� ��� (������)
            float newradian = disnt + shapes[i].alpha;
            float newCenX = newradian * cos(shapes[i].radian);
            float newCenY = newradian * sin(shapes[i].radian);

            // �߽��� �̵��� ���
            float deltaX = newCenX - cenX;
            float deltaY = newCenY - cenY;

            // ��� ������ �����ϰ� �̵� (�ﰢ�� ���� ����)
            for (int j = 0; j < shapes[i].vertices.size(); j += 3) {
                shapes[i].vertices[j] += deltaX;     // X ��ǥ
                shapes[i].vertices[j + 1] += deltaY; // Y ��ǥ
            }

            // ���¿� ���� �� ������Ʈ
            if (shapes[i].state == 0) { // ������ ����
                shapes[i].radian += 0.1;    // ���� ���� (���� ȸ��)
                shapes[i].alpha -= 0.0001f; // ������ ����
            }
            if (shapes[i].state == 1) { // �ٱ����� ����
                shapes[i].radian -= 0.1;    // ���� ���� (�ݴ� ȸ��)
                shapes[i].alpha += 0.0001f; // ������ ����
            }
            std::cout << shapes[i].radian << " " << shapes[i].alpha << std::endl;
            std::cout << disnt << std::endl;
            std::cout << shapes[i].state << std::endl;
            InitBuffers(shapes[i]);
        }
}
    glutPostRedisplay();
    // ��� Ÿ�̸Ӹ� ����
    glutTimerFunc(50, TimerFunction, 1);
}