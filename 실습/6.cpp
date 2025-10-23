#include <iostream>
#include <random>
#include <algorithm>
//--- �ʿ��� ������� include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
float MouseX(int x) {
	return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
	return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
bool isRectangle(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2) {
	return !(ax2 < bx1 || bx2 < ax1 || ay2 < by1 || by2 < ay1);
}
//--- ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> mis(-0.8f, 0.8f);
std::uniform_real_distribution<float> bis(1, 5);

struct Box {
	float x = mis(gen), y = mis(gen);
	float width = 0.2f, height = 0.2f;
	float r = dis(gen), g = dis(gen), b = dis(gen);
	int move = bis(gen);
};
Box bigbox[8];
Box smallbox[30];
int ct = -1 ;
bool click = true, end = false;


//--- ������ ����ϰ� �ݹ��Լ� ����
void main(int argc, char** argv) {
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example6"); // ������ ���� (������ �̸�)
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE; // glew �ʱ�ȭ
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMouseFunc(Mouse); // ���콺 �Է�
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

//--- �ݹ� �Լ�: ��� �ݹ� �Լ�
GLvoid drawScene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.
	for (int i = 0; i < 8; i++) {
		glColor3f(bigbox[i].r, bigbox[i].g, bigbox[i].b);
		glRectf(bigbox[i].x, bigbox[i].y, bigbox[i].x + bigbox[i].width, bigbox[i].y + bigbox[i].height);
	}
	for (int i = 0; i <= ct; i++) {
		glColor3f(smallbox[i].r, smallbox[i].g, smallbox[i].b);
		glRectf(smallbox[i].x, smallbox[i].y, smallbox[i].x + smallbox[i].width, smallbox[i].y + smallbox[i].height);
	}

	
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (!click) return;
		for (int i = 0; i < 8; i++) {
			if (isRectangle(bigbox[i].x, bigbox[i].y, bigbox[i].x + bigbox[i].width, bigbox[i].y + bigbox[i].height,
				MouseX(x), MouseY(y), MouseX(x) + 0.1f, MouseY(y) + 0.1f)) {
				click = false;
				end = false;
				std::cout << bigbox[i].move << std::endl;
				if (bigbox[i].move == 4) {
					for (int j = 0; j < 8; ++j) {
						ct++;
						smallbox[ct].x = bigbox[i].x + bigbox[i].width / 4;
						smallbox[ct].y = bigbox[i].y + bigbox[i].height / 4;
						smallbox[ct].width = bigbox[i].width / 2;
						smallbox[ct].height = bigbox[i].height / 2;
						smallbox[ct].r = bigbox[i].r;
						smallbox[ct].g = bigbox[i].g;
						smallbox[ct].b = bigbox[i].b;
						smallbox[ct].move = j;
					}
				}
				else {
					for (int j = 0; j < 4; ++j) {
						ct++;
						if (j == 0) smallbox[ct].x = bigbox[i].x, smallbox[ct].y = bigbox[i].y;
						if (j == 1) smallbox[ct].x = bigbox[i].x + bigbox[i].width / 2, smallbox[ct].y = bigbox[i].y;
						if (j == 2) smallbox[ct].x = bigbox[i].x, smallbox[ct].y = bigbox[i].y + bigbox[i].height / 2;
						if (j == 3) smallbox[ct].x = bigbox[i].x + bigbox[i].width / 2, smallbox[ct].y = bigbox[i].y + bigbox[i].height / 2;
						smallbox[ct].width = bigbox[i].width / 2;
						smallbox[ct].height = bigbox[i].height / 2;
						smallbox[ct].r = bigbox[i].r;
						smallbox[ct].g = bigbox[i].g;
						smallbox[ct].b = bigbox[i].b;
						smallbox[ct].move = j;
					}
				}
				bigbox[i].x = -2;
				bigbox[i].y = -2;
				glutTimerFunc(100, TimerFunction, bigbox[i].move);
				break;
			}
		}
		std::cout << "mouse click" << std::endl;
	}
	glutPostRedisplay();
}

void TimerFunction(int value) {
	if (value == 0) {
		std::cout << "end time" << std::endl;
		return;
	}
	if (value == 1) {
		std::cout << "time1" << std::endl;
		for (int i = ct - 3; i <= ct; i++) {
			if (smallbox[i].width <= 0 || smallbox[i].height <= 0) {
				smallbox[i].x = -2;
				smallbox[i].y = -2;
				end = true;
				if (i == ct) click = true;
				glutTimerFunc(100, TimerFunction, 0);
			}
			if (smallbox[i].move == 0) smallbox[i].x -= 0.05f;
			if (smallbox[i].move == 1) smallbox[i].x += 0.05f;
			if (smallbox[i].move == 2) smallbox[i].y -= 0.05f;
			if (smallbox[i].move == 3) smallbox[i].y += 0.05f;
			smallbox[i].width -= 0.01f;
			smallbox[i].height -= 0.01f;
		}
		if (!end) glutTimerFunc(100, TimerFunction, 1);
	}
	if (value == 2) {
		std::cout << "time2" << std::endl;
		for (int i = ct - 3; i <= ct; i++) {
			if (smallbox[i].width <= 0 || smallbox[i].height <= 0) {
				smallbox[i].x = -2;
				smallbox[i].y = -2;
				end = true;
				if (i == ct) click = true;
				glutTimerFunc(100, TimerFunction, 0);
			}
			if (smallbox[i].move == 0) {
				smallbox[i].x -= 0.05f;
				smallbox[i].y -= 0.05f;
			}
			if (smallbox[i].move == 1) {
				smallbox[i].x += 0.05f;
				smallbox[i].y -= 0.05f;
			}
			if (smallbox[i].move == 2) {
				smallbox[i].x -= 0.05f;
				smallbox[i].y += 0.05f;
			}
			if (smallbox[i].move == 3) {
				smallbox[i].x += 0.05f;
				smallbox[i].y += 0.05f;
			}
			smallbox[i].width -= 0.01f;
			smallbox[i].height -= 0.01f;
		}
		if (!end) glutTimerFunc(100, TimerFunction, 2);
	}
	if (value == 3) {
		std::cout << "time3" << std::endl;
		for (int i = ct - 3; i <= ct; i++) {
			if (smallbox[i].width <= 0 || smallbox[i].height <= 0) {
				smallbox[i].x = -2;
				smallbox[i].y = -2;
				end = true;
				if (i == ct) click = true;
				glutTimerFunc(100, TimerFunction, 0);
			}
			smallbox[i].x += 0.05f;
			smallbox[i].width -= 0.01f;
			smallbox[i].height -= 0.01f;
		}
		if (!end) glutTimerFunc(100, TimerFunction, 3);
	}
	if (value == 4) {
		std::cout << "time4" << std::endl;
		for (int i = ct - 7; i <= ct; i++) {
			if (smallbox[i].width <= 0 || smallbox[i].height <= 0) {
				smallbox[i].x = -2;
				smallbox[i].y = -2;
				end = true;
				if (i == ct) click = true;
				glutTimerFunc(100, TimerFunction, 0);
			}
			if (smallbox[i].move == 0) smallbox[i].x -= 0.05f;
			if (smallbox[i].move == 1) smallbox[i].x += 0.05f;
			if (smallbox[i].move == 2) smallbox[i].y -= 0.05f;
			if (smallbox[i].move == 3) smallbox[i].y += 0.05f;
			if (smallbox[i].move == 4) {
				smallbox[i].x -= 0.05f;
				smallbox[i].y -= 0.05f;
			}
			if (smallbox[i].move == 5) {
				smallbox[i].x += 0.05f;
				smallbox[i].y -= 0.05f;
			}
			if (smallbox[i].move == 6) {
				smallbox[i].x -= 0.05f;
				smallbox[i].y += 0.05f;
			}
			if (smallbox[i].move == 7) {
				smallbox[i].x += 0.05f;
				smallbox[i].y += 0.05f;
			}
			smallbox[i].width -= 0.01f;
			smallbox[i].height -= 0.01f;
		}
		if (!end)glutTimerFunc(100, TimerFunction, 4);
	}
	glutPostRedisplay();
}