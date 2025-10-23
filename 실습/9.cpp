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

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    float color[3];
	int type;
    GLuint VAO, VBO;
};

Shape shapes[16];
bool filled = true, start = true;
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

//--- 사용자 정의 함수 선언
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
void createTriangle(float x, float y);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Example9");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutMouseFunc(Mouse); // 마우스 입력
    glutMainLoop();
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glGenBuffers(1, &shape.VBO);                // 버퍼id 생성

    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);   // 버퍼객체에 저장할 데이터 타입 지정

    // 바인드 후에 호출하는 모든 버퍼는 바인딩 된 버퍼를 사용한다.
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    // 사용자가 정의한 데이터를 현재 바인딩된 버퍼에 복사한다.

    // 버텍스 속성 포인터 설정
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 0번 속성, 3개의 값, float형, 정규화 안함, 한 정점당 3개의 float, 시작위치 0

    glEnableVertexAttribArray(0);               // 버텍스 속성 배열 사용

    //glBindVertexArray(0);                       // 버텍스 배열 객체 바인딩 해제
}

// 삼각형 생성 함수
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
    //total++;
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
        filled = true;
        break;
    case 'b':
		filled = false;
		break;
    case 'c':
		for (int i = 4; i < total; i++) {
            glDeleteVertexArrays(1, &shapes[i].VAO);
            glDeleteBuffers(1, &shapes[i].VBO);
            shapes[i].vertices.clear();
		}
		cnt[0] = 0; cnt[1] = 0; cnt[2] = 0; cnt[3] = 0;
        total = 4;
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 마우스 콜백 함수
GLvoid Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (MouseX(x) > 0 && MouseY(y) > 0) {
            glDeleteVertexArrays(1, &shapes[0].VAO);
            glDeleteBuffers(1, &shapes[0].VBO);
            shapes[0].vertices.clear();
            createTriangle(MouseX(x), MouseY(y),0);
        }
        if (MouseX(x) < 0 && MouseY(y) > 0) {
            glDeleteVertexArrays(1, &shapes[1].VAO);
            glDeleteBuffers(1, &shapes[1].VBO);
			shapes[1].vertices.clear();
            createTriangle(MouseX(x), MouseY(y),1);
        }
        if (MouseX(x) < 0 && MouseY(y) < 0) {
            glDeleteVertexArrays(1, &shapes[2].VAO);
			glDeleteBuffers(1, &shapes[2].VBO);
			shapes[2].vertices.clear();
            createTriangle(MouseX(x), MouseY(y),2);
        }
        if (MouseX(x) > 0 && MouseY(y) < 0) {
			glDeleteVertexArrays(1, &shapes[3].VAO);
			glDeleteBuffers(1, &shapes[3].VBO);
			shapes[3].vertices.clear();
            createTriangle(MouseX(x), MouseY(y),3);
        }
        glutPostRedisplay();
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if ((MouseX(x) > 0 && MouseY(y) > 0) && cnt[0] >= 3) return;
        if ((MouseX(x) < 0 && MouseY(y) > 0) && cnt[1] >= 3) return;
        if ((MouseX(x) < 0 && MouseY(y) < 0) && cnt[2] >= 3) return;
        if ((MouseX(x) > 0 && MouseY(y) < 0) && cnt[3] >= 3) return;
        createTriangle(MouseX(x), MouseY(y),total);
		std::cout << cnt[0] << cnt[1] << cnt[2] << cnt[3] << std::endl;
        cnt[shapes[total].type]++;
		total++;
        glutPostRedisplay();
	}
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("8_vertex.glsl");
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
    fragmentSource = filetobuf("8_fragment.glsl"); // 프래그세이더 읽어오기
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

    glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h);
}