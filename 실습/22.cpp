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
	glm::vec3 position;
    GLuint VAO, VBO[2], EBO;
};
Shape robot[7]; // 0 머리, 1 몸통, 2 왼팔, 3 오른팔, 4 왼다리, 5 오른다리, 6 코
Shape box; Shape obstacles[3];
glm::mat4 lanimation = glm::mat4(1.0f);
glm::mat4 ranimation = glm::mat4(1.0f);
glm::mat4 movement = glm::mat4(1.0f);
glm::mat4 front = glm::mat4(1.0f);
float robot_speed = 0.05f; float wall_size = 3.0f;
bool movemotion = false; bool open = false;

// 카메라
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.0f, 10.0f),
             glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(0.0f, 1.0f, 0.0f) };
float angle; float cos_angle; float sin_angle;
float new_eye_x; float new_eye_z;

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
GLvoid KeyboardUp(unsigned char key, int x, int y);
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void CreateCube(Shape& cube, float x, float y, float z);
bool AABB(glm::vec3 pos1, float size1, glm::vec3 pos2, float size2) { // 충돌 검사 함수
    return (pos1.x - size1 <= pos2.x + size2 && pos1.x + size1 >= pos2.x - size2 &&
            pos1.z - size1 <= pos2.z + size2 && pos1.z + size1 >= pos2.z - size2);
}
bool wall_collision(glm::vec3 pos1, float size1, float size2) { // 벽 충돌 검사 함수
    return (pos1.x - size1 <= -size2 || pos1.x + size1 >= size2 ||
            pos1.z - size1 <= -size2 || pos1.z + size1 >= size2);
}
void location();
void robot_movement();
void robot_fall();
void robot_jump();
void menu();

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
    glutCreateWindow("Example20");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    menu();
	// 상자 생성
	CreateCube(box, wall_size, wall_size, wall_size);
	// 장애물 생성
    for (int i = 0; i < 3; i++) {
        CreateCube(obstacles[i], 0.5f, 0.5f, 0.5f);
	}
	// 로봇 생성
	CreateCube(robot[0], 0.1f, 0.1f, 0.1f); // 머리
	CreateCube(robot[1], 0.2f, 0.4f, 0.1f); // 몸통
	CreateCube(robot[2], 0.05f, 0.3f, 0.05f); // 왼팔
	CreateCube(robot[3], 0.05f, 0.3f, 0.05f); // 오른팔
	CreateCube(robot[4], 0.05f, 0.3f, 0.05f); // 왼다리
	CreateCube(robot[5], 0.05f, 0.3f, 0.05f); // 오른다리
	CreateCube(robot[6], 0.02f, 0.05f, 0.02f); // 코
	location(); // 초기 위치 설정 함수
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
	glutKeyboardUpFunc(KeyboardUp); // 키보드 업 입력
    glutMainLoop();
}

// 메뉴
void menu() {
    std::cout << "o: 앞면이 열렸다 닫혔다" << std::endl;
    std::cout << "w/a/s/d: 로봇 이동" << std::endl;
    std::cout << "+/-: 로봇 속도 조절" << std::endl;
    std::cout << "j: 로봇 점프" << std::endl;
    std::cout << "i: 초기화" << std::endl;
    std::cout << "z: 카메라 z축 이동" << std::endl;
    std::cout << "x: 카메라 x축 이동" << std::endl;
    std::cout << "y: 카메라 y축 공전" << std::endl;
    std::cout << "q: 종료" << std::endl;
}

// 큐브 생성 함수
void CreateCube(Shape& cube, float x, float y, float z) {
    cube.vertices = {
        // 앞면
       -x, y, z,
       -x, -y, z,
       x, -y, z,
       x, y, z,

       // 뒷면
       -x, -y, -z,
       -x, y, -z,
       x, y, -z,
       x, -y, -z,

       // 윗면
       -x, y, -z,
       -x, y, z,
       x, y, z,
       x, y, -z,

       // 아래면 
       -x, -y, z,
       -x, -y, -z,
       x, -y, -z,
       x, -y, z,

       // 왼면 
       -x, y, -z,
       -x, -y, -z,
       -x, -y, z,
       -x, y, z,

       // 오른면
       x, y, z,
       x, -y, z,
       x, -y, -z,
       x, y, -z
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

    cube.colors = {
        // 앞면 - 연한회색
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,

        // 뒷면 - 연한 회색
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,
        0.8f, 0.8f, 0.8f,

        // 아랫면 - 진한 회색
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,

        // 윗면 - 진한 회색
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,
        0.3f, 0.3f, 0.3f,

        // 오른면 - 회색
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        // 왼면 - 파란색
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f
    };

    InitBuffers(cube);
}

// 큐브 앞면 열기
float open_angle = 0.0f;
void CubeFrontOpen() {
    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, -3.0f));
    glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 3.0f));
    front = translateBack * rotMat * translateToOrigin * front;
    if (open_angle >= 90.0f) open = false;
    open_angle += 2.0f;
}

// 초기 위치 설정 함수
void location() {
    robot[0].position = glm::vec3(0.0f, 1.35f, 0.0f);    // 머리
    robot[1].position = glm::vec3(0.0f, 0.85f, 0.0f);    // 몸통
    robot[2].position = glm::vec3(-0.25f, 0.95f, 0.0f);  // 왼팔
    robot[3].position = glm::vec3(0.25f, 0.95f, 0.0f);   // 오른팔
    robot[4].position = glm::vec3(-0.1f, 0.3f, 0.0f); // 왼다리
    robot[5].position = glm::vec3(0.1f, 0.3f, 0.0f);  // 오른다리
    robot[6].position = glm::vec3(0.0f, 1.3f, 0.1f);   // 코

    for (int i = 0; i < 7; i++) {
		robot[i].colors.clear();
	}
    for (size_t j = 0; j < robot[0].vertices.size() / 3; j++) {
        // 머리 - 빨간색
        robot[0].colors.push_back(1.0f);
        robot[0].colors.push_back(0.0f);
        robot[0].colors.push_back(0.0f);
        // 몸통 - 초록색
        robot[1].colors.push_back(0.0f);
        robot[1].colors.push_back(1.0f);
        robot[1].colors.push_back(0.0f);
        // 왼팔 - 파란색
        robot[2].colors.push_back(0.0f);
        robot[2].colors.push_back(0.0f);
        robot[2].colors.push_back(1.0f);
		// 오른팔 - 시안색
        robot[3].colors.push_back(0.0f);
        robot[3].colors.push_back(1.0f);
		robot[3].colors.push_back(1.0f);
		// 왼다리 - 노란색
        robot[4].colors.push_back(1.0f);
        robot[4].colors.push_back(1.0f);
		robot[4].colors.push_back(0.0f);
        // 오른다리 - 마젠타색
        robot[5].colors.push_back(1.0f);
		robot[5].colors.push_back(0.0f);
		robot[5].colors.push_back(1.0f);
		// 코 - 검정색
		robot[6].colors.push_back(0.0f);
        robot[6].colors.push_back(0.0f);
		robot[6].colors.push_back(0.0f);
    }
    for (int i = 0; i < 7; i++) {
        InitBuffers(robot[i]);
	}

    obstacles[0].position = glm::vec3(-2.0f, -2.5f, 1.0f);
    obstacles[1].position = glm::vec3(-1.0f, -2.5f, 2.0f);
    obstacles[2].position = glm::vec3(1.5f, -2.5f, -1.0f);
}

// 로봇 팔 다리 움직임 함수
void robot_movement() {
    static float robot_angle = 0.0f;
    static bool increasing = true;
    if (increasing) {
        robot_angle += 2.0f;
        if (robot_angle >= 20.f) {
            robot_angle = 20.0f;
            increasing = false;
        }
    } else {
        robot_angle -= 2.0f;
        if (robot_angle <= -20.0f) {
            robot_angle = -20.f;
            increasing = true;
        }
    }
        
    lanimation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f)) *
                 glm::rotate(glm::mat4(1.0f), glm::radians(robot_angle), glm::vec3(1.0f, 0.0f, 0.0f)) *
                 glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
	ranimation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f)) * 
                 glm::rotate(glm::mat4(1.0f), glm::radians(-robot_angle), glm::vec3(1.0f, 0.0f, 0.0f)) *
		         glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
}

// 로봇 점프 함수
bool jumping = false;
void robot_jump() {
    static float jump_height = 0.0f;
    const float jump_speed = 0.1f;
    const float max_jump_height = 2.0f;
    jump_height += jump_speed;
    if (jump_height >= max_jump_height) {
        jump_height = 0.0f;
        jumping = false; // 최고점 도달 후 점프 종료
    }
    for (int i = 0; i < 7; i++) {
        robot[i].position.y += jump_speed;
    }
}

// 로봇 자유 낙하 함수
void robot_fall() {
    // 로봇 다리 위치 계산
    glm::vec3 robot_pos = glm::vec3(movement[3][0], movement[3][1], movement[3][2]);
    float y = robot[4].position.y;
    // 로봇 다리 크기
    float size = 0.3f; // 다리 높이의 절반

	// 장애물과의 충돌 감지
    for (int i = 0; i < 3; i++) {
        if (AABB(robot_pos, 0.2f, obstacles[i].position, 0.5f)) {
            if (y - size > obstacles[i].position.y + 0.6f) {
                for (int j = 0; j < 7; j++) {
                    robot[j].position.y += -0.1f; // 낙하 속도
                }
            }
            else return; // 장애물에 닿았으면 낙하 멈춤
		}
	}

	// 바닥과의 충돌 감지
    if (y - size >= -wall_size) {
        for (int i = 0; i < 7; i++) {
			robot[i].position.y += -0.1f; // 낙하 속도
        }
	}
	else return; // 바닥에 닿았으면 낙하 멈춤
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

    // 투영 행렬 설정
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    // 뷰 행렬 설정
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    // 메인 상자
    glBindVertexArray(box.VAO);
    if (open) {
        for (int i = 0; i < 6; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            if (i == 0) model = model * front;
            else model = glm::mat4(1.0f);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
        }
    }
    else {
        if (open_angle >= 90.0f) {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CW);
        }
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, box.index.size(), GL_UNSIGNED_INT, 0);
    }

    
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

	// 로봇 그리기
    for (int i = 0; i < 7; i++) {
        glm::mat4 model = glm::mat4(1.0f);
		model = movement * model;
		model = glm::translate(model, robot[i].position);
        if (i == 2 || i == 5) model = model * lanimation;
        else if (i == 3 || i == 4) model = model * ranimation;
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(robot[i].VAO);
        glDrawElements(GL_TRIANGLES, robot[i].index.size(), GL_UNSIGNED_INT, 0);
    }

	// 장애물 그리기
    for (int i = 0; i < 3; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obstacles[i].position);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(obstacles[i].VAO);
        glDrawElements(GL_TRIANGLES, obstacles[i].index.size(), GL_UNSIGNED_INT, 0);
	}

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

// 키보드 콜백 함수
float new_x;
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'o': // 앞면 열기
        open = true;
        break;
    case 'z': // 카메라 z축 이동
        camera.eye.z -= 0.1f;
        break;
    case 'Z':
        camera.eye.z += 0.1f;
        break;
    case 'x': // 카메라 x축 이동
        camera.eye.x += 0.1f;
        break;
    case 'X':
        camera.eye.x -= 0.1f;
        break;
    case 'y': // 카메라 y축 공전
        angle = glm::radians(5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
        new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

        camera.eye.x = new_eye_x;
        camera.eye.z = new_eye_z;
        break;
    case 'Y':
        angle = glm::radians(-5.0f);
        cos_angle = cos(angle);
        sin_angle = sin(angle);

        new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
        new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;

        camera.eye.x = new_eye_x;
        camera.eye.z = new_eye_z;
        break;
    case 'w': // 로봇 이동
        glm::vec3 robot_pos = glm::vec3(movement[3][0], movement[3][1], movement[3][2]);
        glm::vec3 next_pos = robot_pos + glm::vec3(glm::mat3(movement) * glm::vec3(0.0f, 0.0f, robot_speed));
        if (wall_collision(next_pos, 0.2f, wall_size)) return;
        for (int i = 0; i < 3; i++) {
            bool isAboveObstacle = (robot[4].position.y - 0.3f) >= (obstacles[i].position.y + 0.5f);
            if (!isAboveObstacle && AABB(next_pos, 0.2f, obstacles[i].position, 0.5f)) return; 
        }
        movement = glm::translate(movement, glm::vec3(0.0f, 0.0f, robot_speed));
        movemotion = true;
        break;
	case 's':
        movement = glm::rotate(movement, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case 'a':
        movement = glm::rotate(movement, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case 'd':
        movement = glm::rotate(movement, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case '=': // 로봇 속도 조절
	case '+':
		robot_speed += 0.01f;
		std::cout << "Robot speed: " << robot_speed << std::endl;
        break;
    case '-':
		if (robot_speed > 0.01f) robot_speed -= 0.01f;
		std::cout << "Robot speed: " << robot_speed << std::endl;
        break;
    case 'j': // 로봇 점프
        jumping = true;
		break;
	case 'i': // 초기화
        location();
        movement = glm::mat4(1.0f);
		lanimation = glm::mat4(1.0f);
        ranimation = glm::mat4(1.0f);
		front = glm::mat4(1.0f);
		movemotion = false;
		open = false;
		open_angle = 0.0f;
		break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 키보드 업 콜백 함수
GLvoid KeyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': // 로봇 이동
    	lanimation = glm::mat4(1.0f);
        ranimation = glm::mat4(1.0f);
        movemotion = false;
        break;
    default:
        break;
    }
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    if(movemotion) robot_movement();
	if (open) CubeFrontOpen();
    if(!jumping) robot_fall();
	else robot_jump();
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}