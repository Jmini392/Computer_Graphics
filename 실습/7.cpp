#include <iostream>
#include <random>
#include <algorithm>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void TimerFunction(int value);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
float MouseX(int x) {
	return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
	return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
bool isRectangle(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2) {
	return !(ax2 < bx1 || bx2 < ax1 || ay2 < by1 || by2 < ay1);
}

//--- 전역 변수 선언
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> xis(0.0f, 0.8f);
std::uniform_real_distribution<float> yis(-0.8f, 0.8f);

struct Box {
	float x = xis(gen), y = yis(gen);
	float width, height;
	float r = dis(gen), g = dis(gen), b = dis(gen);
	bool fit = false;
};
Box box[10];
Box Sbox[10];
bool click = false;
int move = -1, cnt = 0;
bool isclick(float mx, float my, Box b) {
	return (b.x <= mx && mx <= b.x + b.width && b.y <= my && my <= b.y + b.height);
}

//--- 윈도우 출력하고 콜백함수 설정
void main(int argc, char** argv) {
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example7"); // 윈도우 생성 (윈도우 이름)
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
	glutMouseFunc(Mouse); // 마우스 입력
	glutMotionFunc(Motion); // 마우스 움직임
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 콜백 함수: 출력 콜백 함수
GLvoid drawScene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	box[0].width = 0.5f; box[0].height = 0.2f; // 두꺼운 작대기
	box[1].width = 0.5f; box[1].height = 0.1f; // 얇은 작대기
	box[2].width = 0.2f; box[2].height = 0.5f; // 기둥 1
	box[3].width = 0.2f; box[3].height = 0.5f; // 기둥 2
	box[4].width = 0.2f; box[4].height = 0.2f; // 큰 사각형 1
	box[5].width = 0.2f; box[5].height = 0.2f; // 큰 사각형 2
	box[6].width = 0.1f; box[6].height = 0.1f; // 작은 사각형 1
	box[7].width = 0.1f; box[7].height = 0.1f; // 작은 사각형 2
	box[8].width = 0.1f; box[8].height = 0.6f;
	box[9].width = 0.6f; box[9].height = 0.1f;

	Sbox[0].x = -0.7f; Sbox[0].y = -0.5f; Sbox[0].width = box[0].width; Sbox[0].height = box[0].height;
	Sbox[1].x = -0.7f; Sbox[1].y = 0.2f; Sbox[1].width = box[1].width; Sbox[1].height = box[1].height;
	Sbox[2].x = -0.4f; Sbox[2].y = -0.3f; Sbox[2].width = box[2].width; Sbox[2].height = box[2].height;
	Sbox[3].x = -0.7f; Sbox[3].y = -0.3f; Sbox[3].width = box[3].width; Sbox[3].height = box[3].height;
	Sbox[4].x = -0.2f; Sbox[4].y = -0.5f; Sbox[4].width = box[4].width; Sbox[4].height = box[4].height;
	Sbox[5].x = -0.9f; Sbox[5].y = -0.5f; Sbox[5].width = box[5].width; Sbox[5].height = box[5].height;
	Sbox[6].x = -0.5f; Sbox[6].y = 0.3f; Sbox[6].width = box[6].width; Sbox[6].height = box[6].height;
	Sbox[7].x = -0.5f; Sbox[7].y = 0.4f; Sbox[7].width = box[7].width; Sbox[7].height = box[7].height;
	Sbox[8].x = -0.7f; Sbox[8].y = 0.3f; Sbox[8].width = box[8].width; Sbox[8].height = box[8].height;
	Sbox[9].x = -0.4f; Sbox[9].y = 0.3f; Sbox[9].width = box[9].width; Sbox[9].height = box[9].height;

	for (int i = 0; i < 10; i++) {
		Sbox[i].r = box[i].r; Sbox[i].g = box[i].g; Sbox[i].b = box[i].b;
		glPolygonMode(GL_FRONT, GL_LINE);
		glColor3f(Sbox[i].r, Sbox[i].g, Sbox[i].b);
		glRectf(Sbox[i].x, Sbox[i].y, Sbox[i].x + Sbox[i].width, Sbox[i].y + Sbox[i].height);
	}
	for (int i = 0; i < 10; i++) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glColor3f(box[i].r, box[i].g, box[i].b);
		glRectf(box[i].x, box[i].y, box[i].x + box[i].width, box[i].y + box[i].height);
	}
	
	glutSwapBuffers(); // 화면에 출력하기
}

//--- 콜백 함수: 다시 그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		std::cout << "key r" << std::endl;
		for (int i = 0; i < 10; i++) {
			box[i].fit = false;
			box[i].x = xis(gen);
			box[i].y = yis(gen);
		}
		move = -1;
		cnt = 0;
		click = false;
		break;
	case 'b':
		std::cout << "key b" << std::endl;
		if (box[cnt].fit) cnt++;
		glutTimerFunc(10, TimerFunction, 1);
		move = -1;
		click = false;
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << "mouse click" << std::endl;
		for (int i = 0; i < 10; i++) {
			if (isclick(MouseX(x), MouseY(y), box[i])) {
				if (box[i].fit) break;
				click = true;
				move = i;
			}
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (click) {
			if (isclick(MouseX(x), MouseY(y), Sbox[move])) {
				box[move].fit = true;
				box[move].x = Sbox[move].x;
				box[move].y = Sbox[move].y;
				std::cout << "box " << move << " is fit\n";
			}
			else {
				box[move].fit = false;
				box[move].x = xis(gen);
				box[move].y = yis(gen);
				move = -1;
			}
		}
		click = false;
		for (int i = 0; i < 10; i++) {
			if (!box[i].fit) break;
			if (i == 7) std::cout << "All box is fit!\n";
		}
	}
	glutPostRedisplay();
}

void Motion(int x, int y) {
	if (click) {
		box[move].x = MouseX(x) - box[move].width / 2;
		box[move].y = MouseY(y) - box[move].height / 2;
	}
	glutPostRedisplay();
}

void TimerFunction(int value) {
	if (Sbox[cnt].y < box[cnt].y) box[cnt].y -= 0.01f;
	if (Sbox[cnt].y > box[cnt].y) box[cnt].y += 0.01f;
	box[cnt].x -= 0.01f;
	if (box[cnt].x <= Sbox[cnt].x && box[cnt].y >= Sbox[cnt].y) {
		box[cnt].x = Sbox[cnt].x;
		box[cnt].y = Sbox[cnt].y;
		box[cnt].fit = true;
		cnt++;
		glutPostRedisplay();
		return;
	}
	glutTimerFunc(10, TimerFunction, 1);
	glutPostRedisplay();
}