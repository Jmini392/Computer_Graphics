#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// 객체 구조체
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
float trans = 0.0f; int trans_cnt = 0; int cnt = 0; int flag = 1;
int flagcnt = 0;


// 카메라
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.5f, 3.0f),
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

//--- 사용자 정의 함수 선언
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
void drawTank(unsigned int transformLocation, unsigned int colorLocation);

//--- 필요한 변수 선언
GLint width = 800, height = 600;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example23");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    menu();
    CreateCube(sub_body, 1.0f, 0.4f, 0.3f);
    CreateCube(mid_body, 0.5f, 0.3f, 0.15f);
    for (int i = 0; i < 2; i++) {
        CreateCube(top_body[i], 0.3f, 0.4f, 0.2f);
        CreateCube(cannon[i], 0.05f, 0.4f, 0.05f);
        CreateCube(flagpole[i], 0.05f, 0.05f, 0.4f);
    }
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutSpecialFunc(SpecailKeyboard); // 특수키 입력
    glutMainLoop();
}
// 메뉴
void menu() {
    std::cout << "방향키: 물체 이동" << std::endl;
    std::cout << "t: 중앙몸체 y축 회전" << std::endl;
    std::cout << "l: 상부 몸체 서로 위치 이동" << std::endl;
    std::cout << "g: 상부 포신 y축 회전 서로 반대방향" << std::endl;
    std::cout << "p: 상부 깃대 x축 회전 서로 반대방향" << std::endl;
    std::cout << "z: 카메라 z축 이동" << std::endl;
    std::cout << "x: 카메라 x축 이동" << std::endl;
    std::cout << "y: 카메라 y축 자전" << std::endl;
    std::cout << "r: 카메라 y축 공전" << std::endl;
    std::cout << "a: 카메라 공전 애니메이션" << std::endl;
    std::cout << "o: 움직임 멈추기" << std::endl;
    std::cout << "c: 리셋" << std::endl;
    std::cout << "q: 종료" << std::endl;
}

// 큐브 생성 함수
void CreateCube(Shape& cube, float x, float height, float y) {
    cube.vertices = {
        // 앞면
        -x, y, -height,
        -x, -y, -height,
        x, -y, -height,
        x, y, -height,

        // 뒷면
        x, y, height,
        x, -y, height,
        -x, -y, height,
        -x, y, height,

        // 아래면
        -x, -y, -height,
        -x, -y, height,
        x, -y, height,
        x, -y, -height,

        // 윗면
        -x, y, height,
        -x, y, -height,
        x, y, -height,
        x, y, height,

        // 오른면
        x, y, -height,
        x, -y, -height,
        x, -y, height,
        x, y, height,

        // 왼면
        -x, y, height,
        -x, -y, height,
        -x, -y, -height,
        -x, y, -height
    };
    cube.index = {
        // 앞면
        0, 1, 2, 0, 2, 3,
        // 뒷면
        4, 5, 6, 4, 6, 7,
        // 아래면
        8, 9, 10, 8, 10, 11,
        // 위면
        12, 13, 14, 12, 14, 15,
        // 오른면
        16, 17, 18, 16, 18, 19,
        // 왼면
        20, 21, 22, 20, 22, 23
    };
    InitBuffers(cube);
}

// 카메라 회전 애니메이션
void camera_rotation_animation() {
    angle = glm::radians(5.0f);
    cos_angle = cos(angle);
    sin_angle = sin(angle);

    new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
    new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

    camera.eye.x = new_eye_x;
    camera.eye.z = new_eye_z;
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩

    glGenBuffers(2, shape.VBO);                 // 버퍼id 생성

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // 좌표
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &shape.EBO);                 // 인덱스 버퍼id 생성
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // 색상
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("19_vertex.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
    GLchar* fragmentSource;
    //--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
    fragmentSource = filetobuf("19_fragment.glsl"); // 프래그세이더 읽어오기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
    GLuint shaderID;
    GLint result;
    GLchar errorLog[512];
    shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return false;
    }
    glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
    //--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
    //--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
    //--- 사용하기 직전에 호출할 수 있다.
    return shaderID;
}

//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
    // 배경을 흰색으로 설정
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);

    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "model");
    unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

    // 투영 행렬 설정
    glm::mat4 projection = glm::mat4(1.0f);

    // ========== 원근 투영 ==========
    glViewport(0, 0, width / 2 , height /2);
    projection = glm::perspective(glm::radians(90.0f), (float)(width / 2) / (float)(height / 2), 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::mat4 view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    drawTank(transformLocation, colorLocation);

    // ========== 위에서 본 직교 투영 ==========
    glViewport(width / 2, height / 2, width / 2, height / 2);
    projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 10.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    
    view = glm::lookAt(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    drawTank(transformLocation, colorLocation);

    // ========== 앞에서 본 직교 투영 ==========
    glViewport(width / 2, 0, width / 2, height / 2);
    projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 10.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    drawTank(transformLocation, colorLocation);

    glutSwapBuffers();
}

// 탱크 그리기를 별도 함수로 분리
void drawTank(unsigned int transformLocation, unsigned int colorLocation) {
    // 아래 몸체
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, tankPosition);
    model = glm::translate(model, glm::vec3(0.0f, -0.45f, 0.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(sub_body.VAO);
    glUniform3f(colorLocation, 0.5f, 0.5f, 0.5f);
    glDrawElements(GL_TRIANGLES, sub_body.index.size(), GL_UNSIGNED_INT, 0);

    // 중간 몸체
    model = glm::mat4(1.0f);
    model = glm::translate(model, tankPosition);
    model = middle_rotation_matrix * model;
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(mid_body.VAO);
    glUniform3f(colorLocation, 0.8f, 0.8f, 0.8f);
    glDrawElements(GL_TRIANGLES, mid_body.index.size(), GL_UNSIGNED_INT, 0);

    for (int i = 0; i < 2; i++) {
        // 왼쪽, 오른쪽 배치
        float x;
        if (i == 0) {
            if (flag == 1) x = -0.5f + trans;
            else x = 0.5f - trans;
        }
        else {
            if (flag == 1) x = 0.5f - trans;
            else x = -0.5f + trans;
        }

        // 상부 몸체
        model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition);
        model = middle_rotation_matrix * model;
        model = glm::translate(model, glm::vec3(x, 0.35f, 0.0f));
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(top_body[i].VAO);
        glUniform3f(colorLocation, 0.0f, 0.8f, 0.0f);
        glDrawElements(GL_TRIANGLES, top_body[i].index.size(), GL_UNSIGNED_INT, 0);

        // 포신
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

        // 깃대
        model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition);
        model = glm::translate(model, glm::vec3(x, 0.65f, 0.0f));
        model = middle_rotation_matrix * model;

        glm::mat4 individual_flagpole_rotation = glm::mat4(1.0f);
        if (i == 0) individual_flagpole_rotation = flagpole_rotation_matrix;
        else individual_flagpole_rotation = glm::inverse(flagpole_rotation_matrix);
        glm::mat4 translate_down = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.2f, 0.0f));
        model = inverse(translate_down) * model * individual_flagpole_rotation * translate_down;

        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(flagpole[i].VAO);
        glUniform3f(colorLocation, 1.0f, 0.7f, 0.0f);
        glDrawElements(GL_TRIANGLES, flagpole[i].index.size(), GL_UNSIGNED_INT, 0);
    }
}


//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// 키보드 콜백 함수
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

// 특수키 콜백 함수
GLvoid SpecailKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        tankPosition.z -= 0.1f;  // 앞으로 이동
        break;
    case GLUT_KEY_DOWN:
        tankPosition.z += 0.1f;  // 뒤로 이동
        break;
    case GLUT_KEY_LEFT:
        tankPosition.x -= 0.1f;  // 왼쪽으로 이동
        break;
    case GLUT_KEY_RIGHT:
        tankPosition.x += 0.1f;  // 오른쪽으로 이동
    }
    glutPostRedisplay();
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    if (middle_rotate) middle_rotation_matrix = glm::rotate(middle_rotation_matrix, glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if (top_translate) {
        trans_cnt++;

        trans += 1.0f / 30.0f;
        if (trans_cnt >= 30) {
            flag = -flag; // 방향 전환
            trans = 0.0f; // 정확히 1.0f로 설정
            top_translate = false; // 애니메이션 중지
            trans_cnt = 0; // 카운터 초기화
        }
    }
    if (cannon_rotate) {
        cnt++;
        if (cnt < 60) cannon_rotation_matrix = glm::rotate(cannon_rotation_matrix, glm::radians(-2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        else if (cnt < 120) cannon_rotation_matrix = glm::rotate(cannon_rotation_matrix, glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        else cnt = 0;
    }
    if (flagpole_rotate) {
        flagcnt++;
        if (flagcnt < 40)flagpole_rotation_matrix = glm::rotate(flagpole_rotation_matrix, glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        else if (flagcnt < 80)flagpole_rotation_matrix = glm::rotate(flagpole_rotation_matrix, glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        else flagcnt = 0;
    }
    if (camera_rotate) camera_rotation_animation();
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}