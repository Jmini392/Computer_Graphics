#include <iostream>
#include <random>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void TimerFunction(int value);

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
float a = 1.0, b = 1.0, c = 1.0;
bool v = true;

//--- 윈도우 출력하고 콜백함수 설정
void main(int argc, char** argv) { 
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성 (윈도우 이름)
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE; // glew 초기화
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 콜백 함수: 출력 콜백 함수
GLvoid drawScene() {
	glClearColor(a, b, c, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.
	glutSwapBuffers(); // 화면에 출력하기
}

//--- 콜백 함수: 다시 그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'c':
			a = 0.0f; b = 1.0f; c = 1.0f;
			glutPostRedisplay();
			break;
		case 'm':
			a = 1.0f; b = 0.0f; c = 1.0f;
			glutPostRedisplay();
			break;
		case 'y':
			a = 1.0f; b = 1.0f; c = 0.0f;
			glutPostRedisplay();
			break;
		case 'a':
			a = dis(gen); b = dis(gen); c = dis(gen);
			glutPostRedisplay();
			break;
		case 'w':
			a = 1.0f; b = 1.0f; c = 1.0f;
			glutPostRedisplay();
			break;
		case 'k':
			a = 0.0f; b = 0.0f; c = 0.0f;
			glutPostRedisplay();
			break;
		case 't':
			v = true;
			glutTimerFunc(1000, TimerFunction, 1); // 타이머
			glutPostRedisplay();
			break;
		case 's':
			v = false;
			glutPostRedisplay();
			break;
		case 'q':
			exit(0);
			break;
	}
}

void TimerFunction(int value) {
	if (!v) return;
	std::cout << "TimerFunction called!" << std::endl;
	a = dis(gen); b = dis(gen); c = dis(gen);
	glutPostRedisplay();
	glutTimerFunc(1000, TimerFunction, 1);
}