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
std::uniform_real_distribution<float> vel_dis(-0.05f, 0.05f);

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    std::vector<unsigned int> index;
    std::vector<float> colors;
    GLuint VAO, VBO[2], EBO;
};
Shape box; Shape boxs[3];
GLUquadric* sphere[5];
glm::vec3 boxs_axis[3];
glm::vec3 spheres_axis[5];
glm::vec3 spheres_vaxis[5];
float cube_rotate_angle;
float boxsize = 1.0f;
float ballsize = 0.1f;
float opensize = 0.0f;
int ballCount = 0;
int bottom_face_index = 2; // 현재 바닥면의 인덱스


// 카메라
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.3f, 1.5f),
             glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(0.0f, 1.0f, 0.0f) };
float angle; float cos_angle; float sin_angle;
float new_eye_x; float new_eye_z;

bool isclicked = false;
bool open = false; bool show = true;


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
void CreateCube(Shape& cube, float x, float height, float y);
void menu();
void UpdateCubePositions();
void BallsMove();

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
    glutCreateWindow("Example21");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    menu();
	CreateCube(box, boxsize, boxsize, boxsize);
    for (int i = 0; i < 3; i++) {
        CreateCube(boxs[i], 0.1f * (i + 1), 0.1f * (i + 1), 0.1f * (i + 1));
        // 초기 위치 설정
        boxs_axis[i].x = 0.0f;
        boxs_axis[i].y = -1.0f + (0.1f * (i + 1));
        boxs_axis[i].z = -0.1f * ((i + 1) * (i + 1) - 1) + 0.3f;
	}
    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMouseFunc(Mouse); // 마우스 입력
    glutMotionFunc(Motion); // 마우스 움직임
    glutMainLoop();
}

// 메뉴
void menu() {
    std::cout << "z: 카메라 z축 이동" << std::endl;
    std::cout << "y: 카메라 y축 공전" << std::endl;
    std::cout << "a: 바닥면 열림" << std::endl;
    std::cout << "b: 공추가 (최대 5개)" << std::endl;
    std::cout << "q: 종료" << std::endl;
    std::cout << "마우스 드래그: 큐브 회전 및 작은 큐브 이동" << std::endl;
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

// 공 업데이트 함수
void BallsMove() {
    for (int i = 0; i < ballCount; i++) {
        // 위치 업데이트
        spheres_axis[i] += spheres_vaxis[i];

        // X축 충돌 체크 및 튕김
        if (spheres_axis[i].x - ballsize < -boxsize) {
            spheres_axis[i].x = -boxsize + ballsize;
            spheres_vaxis[i].x = -spheres_vaxis[i].x;
        } else if (spheres_axis[i].x + ballsize > boxsize) {
            spheres_axis[i].x = boxsize - ballsize;
            spheres_vaxis[i].x = -spheres_vaxis[i].x;
        }
        
        // Y축 충돌 체크 및 튕김
        if (spheres_axis[i].y - ballsize < -boxsize) {
            spheres_axis[i].y = -boxsize + ballsize;
            spheres_vaxis[i].y = -spheres_vaxis[i].y;
        } else if (spheres_axis[i].y + ballsize > boxsize) {
            spheres_axis[i].y = boxsize - ballsize;
            spheres_vaxis[i].y = -spheres_vaxis[i].y;
        }
        
        // Z축 충돌 체크 및 튕김
        if (spheres_axis[i].z - ballsize < -boxsize) {
            spheres_axis[i].z = -boxsize + ballsize;
            spheres_vaxis[i].z = -spheres_vaxis[i].z;
        } else if (spheres_axis[i].z + ballsize > boxsize) {
            spheres_axis[i].z = boxsize - ballsize;
            spheres_vaxis[i].z = -spheres_vaxis[i].z;
        }
    }
}

// 큐브 위치 업데이트 함수
void UpdateCubePositions() {
    for (int i = 0; i < 3; i++) {
        float half_size = 0.1 * (i + 1);
        
        // 새로운 위치 계산
        float new_x = boxs_axis[i].x - sin(glm::radians(cube_rotate_angle)) * 0.02f;
        float new_y = boxs_axis[i].y - cos(glm::radians(cube_rotate_angle)) * 0.02f;
        
        // X축 충돌 체크
        if (new_x - half_size < -boxsize - opensize) new_x = -boxsize - opensize + half_size;
        else if (new_x + half_size > boxsize + opensize) new_x = boxsize + opensize - half_size;
        
        // Y축 충돌 체크
        if (new_y - half_size < -boxsize - opensize) new_y = -boxsize - opensize + half_size;
        else if (new_y + half_size > boxsize + opensize) new_y = boxsize + opensize - half_size;
        
        // 위치 업데이트
        boxs_axis[i].x = new_x;
        boxs_axis[i].y = new_y;
    }
}

// 바닥면을 계산하는 함수 추가
int CalculateBottomFace() {
    // 각 면의 법선 벡터 (로컬 좌표)
    glm::vec3 face_normals[6] = {
        glm::vec3(0.0f, 0.0f, -1.0f), // 앞면 (인덱스 0)
        glm::vec3(0.0f, 0.0f, 1.0f),  // 뒷면 (인덱스 1)
        glm::vec3(0.0f, -1.0f, 0.0f), // 아래면 (인덱스 2)
        glm::vec3(0.0f, 1.0f, 0.0f),  // 위면 (인덱스 3)
        glm::vec3(1.0f, 0.0f, 0.0f),  // 오른면 (인덱스 4)
        glm::vec3(-1.0f, 0.0f, 0.0f)  // 왼면 (인덱스 5)
    };

    // 회전 변환 행렬
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(cube_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));

    int bottom_index = 2; // 기본값
    float min_y = 1.0f; // 가장 아래쪽을 찾기 위한 초기값

    for (int i = 0; i < 6; i++) {
        // 법선 벡터를 회전 변환
        glm::vec4 rotated_normal = rotation_matrix * glm::vec4(face_normals[i], 0.0f);

        // y 성분이 가장 작은(음수 방향으로 가장 큰) 면이 바닥면
        if (rotated_normal.y < min_y) {
            min_y = rotated_normal.y;
            bottom_index = i;
        }
    }

    return bottom_index;
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
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    // 뷰 행렬 설정
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(camera.eye, camera.at, camera.up);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    // 메인 상자
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(cube_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(box.VAO);
    if (open) {
        for (int i = 0; i < 6; i++) {
			if (i == bottom_face_index) continue;
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
        }
    }
	else glDrawElements(GL_TRIANGLES, box.index.size(), GL_UNSIGNED_INT, 0);
    
    
    // 작은 상자들
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    if (show) {
        for (int i = 0; i < 3; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(cube_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, boxs_axis[i]);
            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
            glBindVertexArray(boxs[i].VAO);
            glDrawElements(GL_TRIANGLES, boxs[i].index.size(), GL_UNSIGNED_INT, 0);
        }
    }
    
    // 공 - 활성화된 공만 렌더링
    for (int i = 0; i < ballCount; i++) {
        sphere[i] = gluNewQuadric();
        gluQuadricDrawStyle(sphere[i], GLU_FILL);
        
        glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(cube_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, spheres_axis[i]);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
        gluSphere(sphere[i], ballsize, 20, 20);
        gluDeleteQuadric(sphere[i]);
    }

    glutSwapBuffers();
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
	case 'z':
		camera.eye.z -= 0.1f;
		break;
	case 'Z':
		camera.eye.z += 0.1f;
		break;
    case 'y':
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
    case 'b': // 공 하나씩 생성
        if (ballCount >= 5) return;
        spheres_axis[ballCount] = glm::vec3(0.0f);

        do {
            spheres_vaxis[ballCount].x = vel_dis(gen);
            spheres_vaxis[ballCount].y = vel_dis(gen);
            spheres_vaxis[ballCount].z = vel_dis(gen);
        } while (abs(spheres_vaxis[ballCount].x) < 0.01f &&
            abs(spheres_vaxis[ballCount].y) < 0.01f &&
            abs(spheres_vaxis[ballCount].z) < 0.01f);

        ballCount++;
        break;
    case 'a':
		opensize = 10.0f;
		open = true;
        bottom_face_index = CalculateBottomFace();
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 마우스 콜백 함수
void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isclicked = true;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isclicked = false;
    }
    glutPostRedisplay();
}

// 마우스 움직임 콜백 함수
void Motion(int x, int y) {
    if (isclicked) {
        if (MouseX(x) < 0.0f) cube_rotate_angle += 0.2f;
        else cube_rotate_angle -= 0.2f;
    }    
    glutPostRedisplay();
}

// 타이머 콜백 함수
void TimerFunction(int value) {
    UpdateCubePositions();
    BallsMove();
    if (boxs_axis[0].y <= -2.0f || boxs_axis[0].x <= -2.0f || boxs_axis[0].x >= 2.0f || boxs_axis[0].y >= 2.0f) {
        open = false;
        show = false;
    }
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}