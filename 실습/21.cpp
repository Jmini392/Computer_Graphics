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
Shape sub_body; Shape mid_body;
Shape top_body[2]; Shape cannon[2]; Shape flagpole[2];
glm::vec3 tankPosition = glm::vec3(0.0f);
glm::mat4 middle_rotation_matrix = glm::mat4(1.0f);
glm::mat4 cannon_rotation_matrix = glm::mat4(1.0f);
glm::mat4 flagpole_rotation_matrix = glm::mat4(1.0f);
bool middle_rotate = false, top_translate = false;
bool cannon_rotate = false, flagpole_rotate = false;
float trans = 0.0f; int trans_cnt = 0; int cnt = 0;


// ī�޶�
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.0f, 3.0f),
             glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(0.0f, 1.0f, 0.0f) };
float angle; float cos_angle; float sin_angle;
float new_eye_x; float new_eye_z; glm::vec3 direction;
bool camera_rotate = false;


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
GLvoid SpecailKeyboard(int key, int x, int y);
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void CreateCube(Shape& cube, float x, float height, float y);
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
    glutCreateWindow("Example21");
    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();
    //--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
    make_vertexShaders(); //--- ���ؽ� ���̴� �����
    make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
    shaderProgramID = make_shaderProgram();
    //--- �ݹ� �Լ� ���
    menu();
    CreateCube(sub_body, 1.0f, 0.4f, 0.3f);
    CreateCube(mid_body, 0.5f, 0.3f, 0.15f);
    for (int i = 0; i < 2; i++) {
        CreateCube(top_body[i], 0.3f, 0.4f, 0.2f);
        CreateCube(cannon[i], 0.05f, 0.4f, 0.05f);
        CreateCube(flagpole[i], 0.05f, 0.05f, 0.2f);
    }
    glutTimerFunc(50, TimerFunction, 1); // Ÿ�̸� �Լ� ���
    glutDisplayFunc(drawScene); // ��� �Լ��� ����
    glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
    glutKeyboardFunc(Keyboard); // Ű���� �Է�
    glutSpecialFunc(SpecailKeyboard); // Ư��Ű �Է�
    glutMainLoop();
}
// �޴�
void menu() {
    std::cout << "����Ű: ��ü �̵�" << std::endl;
    std::cout << "t: �߾Ӹ�ü y�� ȸ��" << std::endl;
    std::cout << "l: ��� ��ü ���� ��ġ �̵�" << std::endl;
    std::cout << "g: ��� ���� y�� ȸ�� ���� �ݴ����" << std::endl;
    std::cout << "p: ��� ��� x�� ȸ�� ���� �ݴ����" << std::endl;
    std::cout << "z: ī�޶� z�� �̵�" << std::endl;
    std::cout << "x: ī�޶� x�� �̵�" << std::endl;
    std::cout << "y: ī�޶� y�� ����" << std::endl;
    std::cout << "r: ī�޶� y�� ����" << std::endl;
    std::cout << "a: ī�޶� ���� �ִϸ��̼�" << std::endl;
    std::cout << "o: ������ ���߱�" << std::endl;
    std::cout << "c: ����" << std::endl;
    std::cout << "q: ����" << std::endl;
}

// ť�� ���� �Լ�
void CreateCube(Shape& cube, float x, float height, float y) {
    cube.vertices = {
        // �ո�
        -x, y, -height,
        -x, -y, -height,
        x, -y, -height,
        x, y, -height,

        // �޸�
        x, y, height,
        x, -y, height,
        -x, -y, height,
        -x, y, height,

        // �Ʒ���
        -x, -y, -height,
        -x, -y, height,
        x, -y, height,
        x, -y, -height,

        // ���� 
        -x, y, height,
        -x, y, -height,
        x, y, -height,
        x, y, height,

        // ������
        x, y, -height,
        x, -y, -height,
        x, -y, height,
        x, y, height,

        // �޸� 
        -x, y, height,
        -x, -y, height,
        -x, -y, -height,
        -x, y, -height
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

// ī�޶� ȸ�� �ִϸ��̼�
void camera_rotation_animation() {
    angle = glm::radians(5.0f);
    cos_angle = cos(angle);
    sin_angle = sin(angle);

    new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
    new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

    camera.eye.x = new_eye_x;
    camera.eye.z = new_eye_z;
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
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    // �� ��� ����
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    // ��ũ �׸���
    // �Ʒ� ��ü
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, tankPosition);
    model = glm::translate(model, glm::vec3(0.0f, -0.45f, 0.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(sub_body.VAO);
    glUniform3f(colorLocation, 0.5f, 0.5f, 0.5f);
    glDrawElements(GL_TRIANGLES, sub_body.index.size(), GL_UNSIGNED_INT, 0);

    // �߰� ��ü
    model = glm::mat4(1.0f);
    model = glm::translate(model, tankPosition);
    model = middle_rotation_matrix * model;
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(mid_body.VAO);
    glUniform3f(colorLocation, 0.8f, 0.8f, 0.8f);
    glDrawElements(GL_TRIANGLES, mid_body.index.size(), GL_UNSIGNED_INT, 0);


    for (int i = 0; i < 2; i++) {
        // ����, ������ ��ġ
        float x;
        if (i == 0) x = -0.5f + trans;
        else x = 0.5f - trans;

        // ��� ��ü
        model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition);
        model = middle_rotation_matrix * model;
        model = glm::translate(model, glm::vec3(x, 0.35f, 0.0f));
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(top_body[i].VAO);
        glUniform3f(colorLocation, 0.0f, 0.8f, 0.0f);
        glDrawElements(GL_TRIANGLES, top_body[i].index.size(), GL_UNSIGNED_INT, 0);

        // ����
        model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition);
        model = glm::translate(model, glm::vec3(x, 0.35f, 0.0f));
        model = middle_rotation_matrix * model;

        glm::mat4 individual_cannon_rotation = glm::mat4(1.0f);
        if (x < 0) individual_cannon_rotation = cannon_rotation_matrix;
        else individual_cannon_rotation = glm::inverse(cannon_rotation_matrix);
        model = model * individual_cannon_rotation;

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.4f));

        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(cannon[i].VAO);
        glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, cannon[i].index.size(), GL_UNSIGNED_INT, 0);

        // ���
        model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition);
        model = glm::translate(model, glm::vec3(x, 0.75f, 0.0f));
        model = middle_rotation_matrix * model;

        glm::mat4 individual_flagpole_rotation = glm::mat4(1.0f);
        if (i == 0) individual_flagpole_rotation = flagpole_rotation_matrix;
        else individual_flagpole_rotation = glm::inverse(flagpole_rotation_matrix);
        model = model * individual_flagpole_rotation;

        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(flagpole[i].VAO);
        glUniform3f(colorLocation, 1.0f, 0.7f, 0.0f);
        glDrawElements(GL_TRIANGLES, flagpole[i].index.size(), GL_UNSIGNED_INT, 0);
    }

    glutSwapBuffers();
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// Ű���� �ݹ� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 't':
        middle_rotate = true;
        break;
    case 'l':
        top_translate = true;
        break;
    case 'g':
        cannon_rotate = true;
        break;
    case 'p':
        flagpole_rotate = true;
        break;
    case 'z':
        camera.eye.z += 0.1f;
        break;
    case 'Z':
        camera.eye.z -= 0.1f;
        break;
    case 'x':
        camera.eye.x += 0.1f;
        camera.at.x += 0.1f;
        break;
    case 'X':
        camera.eye.x -= 0.1f;
        camera.at.x -= 0.1f;
        break;
    case 'y':
        angle = glm::radians(5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        direction = camera.at - camera.eye;
        new_eye_x = direction.x * cos_angle - direction.z * sin_angle;
        new_eye_z = direction.x * sin_angle + direction.z * cos_angle;

        camera.at.x = camera.eye.x + new_eye_x;
        camera.at.z = camera.eye.z + new_eye_z;
        break;
    case 'Y':
        angle = glm::radians(-5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        direction = camera.at - camera.eye;
        new_eye_x = direction.x * cos_angle - direction.z * sin_angle;
        new_eye_z = direction.x * sin_angle + direction.z * cos_angle;

        camera.at.x = camera.eye.x + new_eye_x;
        camera.at.z = camera.eye.z + new_eye_z;
        break;
    case 'r':
        angle = glm::radians(5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
        new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

        camera.eye.x = new_eye_x;
        camera.eye.z = new_eye_z;
        break;
    case 'R':
        angle = glm::radians(-5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
        new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

        camera.eye.x = new_eye_x;
        camera.eye.z = new_eye_z;
        break;
    case 'a':
        camera_rotate = true;
        break;
    case 'o':
        middle_rotate = false;
        top_translate = false;
        cannon_rotate = false;
        flagpole_rotate = false;
        camera_rotate = false;
        break;
    case 'c':
        camera_rotate = false;
        middle_rotate = false;
        top_translate = false;
        cannon_rotate = false;
        flagpole_rotate = false;
        cnt = 0;
        trans_cnt = 0;
        trans = 0.0f;
        camera = { glm::vec3(0.0f, 0.0f, 3.0f),
                   glm::vec3(0.0f, 0.0f, 0.0f),
                   glm::vec3(0.0f, 1.0f, 0.0f) };
        CreateCube(sub_body, 1.0f, 0.4f, 0.3f);
        CreateCube(mid_body, 0.5f, 0.3f, 0.15f);
        for (int i = 0; i < 2; i++) {
            CreateCube(top_body[i], 0.3f, 0.4f, 0.2f);
            CreateCube(cannon[i], 0.05f, 0.2f, 0.05f);
            CreateCube(flagpole[i], 0.05f, 0.05f, 0.2f);
        }
        tankPosition = glm::vec3(0.0f);
        middle_rotation_matrix = glm::mat4(1.0f);
        cannon_rotation_matrix = glm::mat4(1.0f);
        flagpole_rotation_matrix = glm::mat4(1.0f);
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// Ư��Ű �ݹ� �Լ�
GLvoid SpecailKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        tankPosition.z -= 0.1f;  // ������ �̵�
        break;
    case GLUT_KEY_DOWN:
        tankPosition.z += 0.1f;  // �ڷ� �̵�
        break;
    case GLUT_KEY_LEFT:
        tankPosition.x -= 0.1f;  // �������� �̵�
        break;
    case GLUT_KEY_RIGHT:
        tankPosition.x += 0.1f;  // ���������� �̵�
    }
    glutPostRedisplay();
}

// Ÿ�̸� �ݹ� �Լ�
void TimerFunction(int value) {
    if (middle_rotate) middle_rotation_matrix = glm::rotate(middle_rotation_matrix, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if (top_translate) {
        trans_cnt++;

        trans += 1.0f / 30.0f;
        if (trans_cnt >= 30) {
            trans = 1.0f; // ��Ȯ�� 1.0f�� ����
            top_translate = false; // �ִϸ��̼� ����
            trans_cnt = 0; // ī���� �ʱ�ȭ
        }
    }
    if (cannon_rotate) {
        cnt++;
        if (cnt < 60) cannon_rotation_matrix = glm::rotate(cannon_rotation_matrix, glm::radians(-2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        else if (cnt < 120) cannon_rotation_matrix = glm::rotate(cannon_rotation_matrix, glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        else cnt = 0;
    }
    if (flagpole_rotate) flagpole_rotation_matrix = glm::rotate(flagpole_rotation_matrix, glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    if (camera_rotate) camera_rotation_animation();
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}