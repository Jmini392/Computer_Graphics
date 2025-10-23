#include <iostream>
#include <random>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
float MouseX(int x) {
	return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
	return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
float a = 1.0, b = 1.0, c = 1.0;
float rx1 = -1.0, ry1 = 1.0, rx2 = 0.0, ry2 = 0.0, rx3 = 0.0, ry3 = 1.0, rx4 = 1.0, ry4 = 0.0, rx5 = -1.0, ry5 = 0.0, rx6 = 0.0, ry6 = -1.0, rx7 = 0.0, ry7 = 0.0, rx8 = 1.0, ry8 = -1.0;
float ra1 = 1.0, ra2 = 0.0, ra3 = 1.0, ra4 = 0.0, rb1 = 0.0, rb2 = 1.0, rb3 = 0.0, rb4 = 1.0, rc1 = 1.0, rc2 = 0.0, rc3 = 0.0, rc4 = 1.0;

//--- 윈도우 출력하고 콜백함수 설정
void main(int argc, char** argv) {
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example2"); // 윈도우 생성 (윈도우 이름)
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE; // glew 초기화
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutMouseFunc(Mouse); // 마우스 입력
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 콜백 함수: 출력 콜백 함수
GLvoid drawScene() {
	glClearColor(a, b, c, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.
	glColor3f(ra1, rb1, rc1);
	glRectf(rx1, ry1, rx2, ry2);
	glColor3f(ra2, rb2, rc2);
	glRectf(rx3, ry3, rx4, ry4);
	glColor3f(ra3, rb3, rc3);
	glRectf(rx5, ry5, rx6, ry6);
	glColor3f(ra4, rb4, rc4);
	glRectf(rx7, ry7, rx8, ry8);

	glutSwapBuffers(); // 화면에 출력하기
}

//--- 콜백 함수: 다시 그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (((MouseX(x) > rx1) && (MouseX(x) < rx2)) && ((MouseY(y) < ry1 && MouseY(y) > ry2))) {
			ra1 = dis(gen), rb1 = dis(gen), rc1 = dis(gen);
		}
		else if (((MouseX(x) > rx3) && (MouseX(x) < rx4)) && ((MouseY(y) < ry3) && (MouseY(y) > ry4))) {
			ra2 = dis(gen), rb2 = dis(gen), rc2 = dis(gen);
		}
		else if (((MouseX(x) > rx5) && (MouseX(x) < rx6)) && ((MouseY(y) < ry5) && (MouseY(y) > ry6))) {
			ra3 = dis(gen), rb3 = dis(gen), rc3 = dis(gen);
		}
		else if (((MouseX(x) > rx7) && (MouseX(x) < rx8)) && ((MouseY(y) < ry7) && (MouseY(y) > ry8))) {
			ra4 = dis(gen), rb4 = dis(gen), rc4 = dis(gen);
		}
		else a = dis(gen), b = dis(gen), c = dis(gen);
		glutPostRedisplay();
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (((MouseX(x) > rx1) && (MouseX(x) < rx2)) && ((MouseY(y) < ry1 && MouseY(y) > ry2))) {
			if (rx1 >= -0.6) return;
			rx1 += 0.1; ry1 -= 0.1; rx2 -= 0.1; ry2 += 0.1;
		}
		else if (((MouseX(x) > rx3) && (MouseX(x) < rx4)) && ((MouseY(y) < ry3) && (MouseY(y) > ry4))) {
			if (rx3 >= 0.4) return;
			rx3 += 0.1; ry3 -= 0.1; rx4 -= 0.1; ry4 += 0.1;
		}
		else if (((MouseX(x) > rx5) && (MouseX(x) < rx6)) && ((MouseY(y) < ry5) && (MouseY(y) > ry6))) {
			if (rx5 >= -0.6) return;
			rx5 += 0.1; ry5 -= 0.1; rx6 -= 0.1; ry6 += 0.1;
		}
		else if (((MouseX(x) > rx7) && (MouseX(x) < rx8)) && ((MouseY(y) < ry7) && (MouseY(y) > ry8))) {
			if (rx7 >= 0.4) return;
			rx7 += 0.1; ry7 -= 0.1; rx8 -= 0.1; ry8 += 0.1;
		}
		else {
			if (rx1 > -1.0) {
				rx1 -= 0.1; ry1 += 0.1; rx2 += 0.1; ry2 -= 0.1;
			}
			if (rx3 > 0.0) {
				rx3 -= 0.1; ry3 += 0.1; rx4 += 0.1; ry4 -= 0.1;
				if ((rx3 < 0.1) && (rx3 > 0.0)) rx3 = 0.0;
			}
			if (rx5 > -1.0) {
				rx5 -= 0.1; ry5 += 0.1; rx6 += 0.1; ry6 -= 0.1;
			}
			if (rx7 > 0.0) {
				rx7 -= 0.1; ry7 += 0.1; rx8 += 0.1; ry8 -= 0.1;
				if ((rx7 < 0.1) && (rx7 > 0.0)) rx7 = 0.0;
			}
		}
		glutPostRedisplay();
	}
}