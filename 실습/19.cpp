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

// ī�޶�
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.1f, 1.0f),
             glm::vec3(0.0f, 0.0f, 0.0f),
			 glm::vec3(0.0f, 1.0f, 0.0f) };

GLUquadric* sphere;
GLUquadric* spheres[3];
GLUquadric* s_spheres[3];
Shape orbits[3];
Shape s_orbits[3];
glm::mat4 Matrix[3];
glm::mat4 s_Matrix[3];
glm::mat4 smat[3];
glm::mat4 big_Matrix;
glm::mat4 scalemat(1.0f);
glm::mat4 zmat(1.0f);
glm::mat4 movemat(1.0f);
bool solid = true, angle = false, z_rotate = false;
float scale = 1.0f, zangle = 1.0f;


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
void CreateCorrdinate(Shape& line);
void CreateCube(Shape& cube);
void CreateCorn(Shape& corn);
void CreateOrbit(Shape& orbit, float radius, int segments);
void CreateMatrix();
void menu();

//--- �ʿ��� ���� ����
GLint width = 800, height = 600;
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
    glutInitWindowSize(width, height);
    glutCreateWindow("Example19");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    menu();
    CreateMatrix();
    for (int i = 0; i < 3; i++) {
        CreateOrbit(orbits[i], 0.5f, 100);
		CreateOrbit(s_orbits[i], 0.2f, 100);
    }
    glutTimerFunc(50, TimerFunction, 1); // Ÿ�̸� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutMainLoop();
}
// �޴�
void menu() {
    std::cout << "p: �������� / ��������" << std::endl;
    std::cout << "m: �ָ��� / ���̾�" << std::endl;
    std::cout << "w: �� �̵�" << std::endl;
    std::cout << "a: �� �̵�" << std::endl;
    std::cout << "s: �� �̵�" << std::endl;
    std::cout << "d: �� �̵�" << std::endl;
    std::cout << "+: ī�޶� z�̵�" << std::endl;
    std::cout << "y: �˵��� ������ ũ������" << std::endl;
    std::cout << "z: �ֺ����� z������ ȸ��" << std::endl;
    std::cout << "q: ����" << std::endl;
}

void CreateMatrix() {
    big_Matrix = glm::mat4(1.0f);
	glm::mat4 transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
	glm::mat4 s_transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.0f, 0.0f));
	smat[0] = glm::mat4(1.0f);
    smat[1] = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    smat[2] = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    for (int i = 0; i < 3; i++) {
        Matrix[i] = transmat;
		s_Matrix[i] = s_transmat;
    }
}

void scaling(float s) {
    scalemat = glm::scale(glm::mat4(1.0f), glm::vec3(s, s, s));
}

// �˵� ���� �Լ�
void CreateOrbit(Shape& orbit, float radius, int segments) {
    orbit.vertices.clear();
    orbit.index.clear();
    orbit.colors.clear();

    // �� ������ ���� ����
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159265f * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        orbit.vertices.push_back(x);
        orbit.vertices.push_back(0.0f);
        orbit.vertices.push_back(z);

        // �˵� ���� (ȸ��)
        orbit.colors.push_back(0.5f);
        orbit.colors.push_back(0.5f);
        orbit.colors.push_back(0.5f);

        orbit.index.push_back(i);
    }

    InitBuffers(orbit);
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

//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
    //--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
    vertexSource = filetobuf("19_vertex.glsl");
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
    fragmentSource = filetobuf("19_fragment.glsl"); // �����׼��̴� �о����
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
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);
    
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");
    unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	// ���� ��� ����
    glm::mat4 projection = glm::mat4(1.0f);
    if (angle) projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 10.0f);
    else projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    
    // �� ��� ����
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    // �˵� �׸���
    glUniform3f(colorLocation, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; i++) {
        glm::mat4 orbitMatrix = movemat * zmat * scalemat * smat[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(orbitMatrix));
        glBindVertexArray(orbits[i].VAO);
        glDrawElements(GL_LINE_LOOP, orbits[i].index.size(), GL_UNSIGNED_INT, 0);

        // ���� �� �˵� �׸��� (spheres ����)
        glm::mat4 s_orbitMatrix = movemat * zmat * scalemat * smat[i] * Matrix[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(s_orbitMatrix));
        glBindVertexArray(s_orbits[i].VAO);
		glDrawElements(GL_LINE_LOOP, s_orbits[i].index.size(), GL_UNSIGNED_INT, 0);
    }

    // ���� �׸���
    sphere = gluNewQuadric();
    for (int i = 0; i < 3; i++) {
		spheres[i] = gluNewQuadric();
		s_spheres[i] = gluNewQuadric();
    }
    if (solid) {
        gluQuadricDrawStyle(sphere, GLU_FILL);
        for (int i = 0; i < 3; i++) {
			gluQuadricDrawStyle(spheres[i], GLU_FILL);
			gluQuadricDrawStyle(s_spheres[i], GLU_FILL);
        }
    }
    else {
        gluQuadricDrawStyle(sphere, GLU_LINE);
        for (int i = 0; i < 3; i++) {
            gluQuadricDrawStyle(spheres[i], GLU_LINE);
			gluQuadricDrawStyle(s_spheres[i], GLU_LINE);
        }
    }
	
    // �߽� �� (���� ū ��) - ������
	glm::mat4 trans = glm::mat4(1.0f);
	trans = movemat * big_Matrix;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // ������
    gluSphere(sphere, 0.2, 20, 20);
    
    for (int i = 0; i < 3; i++) {
        // �߰� ũ�� �� - �ʷϻ�
        glm::mat4 translate = glm::mat4(1.0f);
		translate = movemat * zmat * scalemat * smat[i] * Matrix[i];
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(translate));
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f); // �ʷϻ�
        gluSphere(spheres[i], 0.05, 10, 10);

        // ���� �� - �Ķ���
		glm::mat4 s_translate = glm::mat4(1.0f);
		s_translate = movemat * zmat * scalemat * smat[i] * Matrix[i] *  s_Matrix[i];
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(s_translate));
		glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f); // �Ķ���
		gluSphere(s_spheres[i], 0.02, 10, 10);

		gluDeleteQuadric(s_spheres[i]);
        gluDeleteQuadric(spheres[i]);
	}
    gluDeleteQuadric(sphere);
    
    glutSwapBuffers();
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
	case 'p': // ���� ����
        angle = true;
        break;
	case 'P': // ���� ����
		angle = false;
		break;
    case 'm': // �ָ���
        solid = true;
        break;
    case 'M': // ���̾�
        solid = false;
		break;
    case '+':
        camera.eye.z -= 0.1f;
        break;
    case '-':
        camera.eye.z += 0.1f;
        break;
    case 'w':
		movemat = glm::translate(movemat, glm::vec3(0.0f, 0.1f, 0.0f));
        break;
    case 'a':
        movemat = glm::translate(movemat, glm::vec3(-0.1f, 0.0f, 0.0f));
        break;
    case 's':
		movemat = glm::translate(movemat, glm::vec3(0.0f, -0.1f, 0.0f));
		break;
    case 'd':
		movemat = glm::translate(movemat, glm::vec3(0.1f, 0.0f, 0.0f));
        break;
	case 'y': // �˵��� ������ ũ������
		scale += 0.1f;
        scaling(scale);
		break;
	case 'Y':
		scale -= 0.1f;
        scaling(scale);
		break;
	case 'z': // �ֺ����� z������ ȸ��
		z_rotate = true;
        zangle = 2.0f;
		break;
	case 'Z':
		z_rotate = true;
        zangle = -2.0f;
		break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    glm::mat4 a = glm::rotate(glm::mat4(1.0f), glm::radians(1.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 b = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 c = glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    Matrix[0] = a * Matrix[0];
    Matrix[1] = b * Matrix[1];
    Matrix[2] = c * Matrix[2];

    glm::mat4 s_a = glm::rotate(glm::mat4(1.0f), glm::radians(2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 s_b = glm::rotate(glm::mat4(1.0f), glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 s_c = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    s_Matrix[0] = s_a * s_Matrix[0];
    s_Matrix[1] = s_b * s_Matrix[1];
    s_Matrix[2] = s_c * s_Matrix[2];

	if (z_rotate) zmat = glm::rotate(glm::mat4(1.0f), glm::radians(zangle), glm::vec3(0.0f, 0.0f, 1.0f)) * zmat;

    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}