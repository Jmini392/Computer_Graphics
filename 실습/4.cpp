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
void TimerFunction(int value);
void Mouse(int button, int state, int x, int y);
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
std::uniform_real_distribution<float> mis(1.0f, 4.0f);

struct Box {
	float x = -2, y = -2;
	float dx = -2, dy = -2;
	float ex = -2, ey = -2;
	float width = 0.1f, height = 0.1f;
	float r = dis(gen), g = dis(gen), b = dis(gen);
	int move = mis(gen), size = 0, zmove = 1;
};

Box box[5];
int cnt = 0, click = -1;
bool one = false, two = false, three = false, four = false, five = false;
bool timerRunning = false; // Ÿ�̸� ���� ���¸� �����ϴ� ����

//--- ������ ����ϰ� �ݹ��Լ� ����
void main(int argc, char** argv) {
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example4"); // ������ ���� (������ �̸�)
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE; // glew �ʱ�ȭ
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutKeyboardFunc(Keyboard); // Ű���� �Է�
	glutMouseFunc(Mouse); // ���콺 �Է�
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

//--- �ݹ� �Լ�: ��� �ݹ� �Լ�
GLvoid drawScene() {
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.
	
	for (int i = 0; i < cnt; i++) {
		glColor3f(box[i].r, box[i].g, box[i].b);
		glRectf(box[i].x, box[i].y, box[i].x + box[i].width, box[i].y + box[i].height);
	}
	
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		std::cout << "key 1" << std::endl;
		if (one) {
			one = false;
		}
		else {
			one = true;
			two = false;
			// Ÿ�̸Ӱ� �̹� ���� ���� �ƴ� ���� �� Ÿ�̸� ����
			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(100, TimerFunction, 1);
			}
		}
		break;
	case '2':
		std::cout << "key 2" << std::endl;
		if (two) {
			two = false;
		}
		else {
			two = true;
			one = false;
			// Ÿ�̸Ӱ� �̹� ���� ���� �ƴ� ���� �� Ÿ�̸� ����
			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(100, TimerFunction, 1);
			}
		}
		break;
	case '3':
		std::cout << "key 3" << std::endl;
		if (three) {
			three = false;
		}
		else {
			three = true;
			// Ÿ�̸Ӱ� �̹� ���� ���� �ƴ� ���� �� Ÿ�̸� ����
			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(100, TimerFunction, 1);
			}
		}
		break;
	case '4':
		std::cout << "key 4" << std::endl;
		if (four) {
			four = false;
		}
		else {
			four = true;
			// Ÿ�̸Ӱ� �̹� ���� ���� �ƴ� ���� �� Ÿ�̸� ����
			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(100, TimerFunction, 1);
			}
		}
		break;
	case '5':
		std::cout << "key 5" << std::endl;
		if (click == -1)break;
		if (five) {
			five = false;
		}
		else {
			five = true;
			// Ÿ�̸Ӱ� �̹� ���� ���� �ƴ� ���� �� Ÿ�̸� ����
			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(100, TimerFunction, 1);
			}
		}
		break;
	case 's':
		std::cout << "key s" << std::endl;
		one = two = three = four = five = false;
		break;
	case 'm':
		std::cout << "key m" << std::endl;
		for (int i = 0; i < cnt; i++) {
			box[i].x = box[i].dx;
			box[i].y = box[i].dy;
		}
		break;
	case 'r':
		std::cout << "key r" << std::endl;
		for (int i = 0; i < 5; i++) {
			box[i].x = box[i].dx = box[i].y = box[i].dy = -2;
			box[i].move = 0;
		}
		cnt = 0;
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < cnt; i++) {
			if (isRectangle(box[i].x, box[i].y, box[i].x + box[i].width, box[i].y + box[i].height,
				MouseX(x), MouseY(y), MouseX(x) + 0.1f, MouseY(y) + 0.1f)) {
				click = i;
				std::cout << "click" << click << std::endl;
				break;
			}
			else click = -1;
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (cnt >= 5) return;
		box[cnt].x = MouseX(x) - 0.05f;
		box[cnt].dx = box[cnt].x;
		box[cnt].y = MouseY(y) - 0.05f;
		box[cnt].dy = box[cnt].y;
		std::cout << "create" << cnt << std::endl;
		cnt++;
	}
	glutPostRedisplay();
}

void TimerFunction(int value) {
	// ��� �ִϸ��̼��� ��Ȱ��ȭ�Ǿ��� �� Ÿ�̸� ����
	if (!one && !two && !three && !four && !five) {
		timerRunning = false;
		return;
	}

	if (one) {
		for (int i = 0; i < cnt; i++) {
			box[i].ex = box[i].x;
			box[i].ey = box[i].y;
			if (box[i].x <= -1.0f) box[i].move = 1;
			if (box[i].y <= -1.0f) box[i].move = 2;
			if (box[i].x + box[i].width >= 1.0f) box[i].move = 3;
			if (box[i].y + box[i].height >= 1.0f) box[i].move = 4;

			if (box[i].move == 1) {
				box[i].x += 0.1f; box[i].y -= 0.1f;
			}
			if (box[i].move == 2) {
				box[i].x += 0.1f; box[i].y += 0.1f;
			}
			if (box[i].move == 3) {
				box[i].x -= 0.1f; box[i].y += 0.1f;
			}
			if (box[i].move == 4) {
				box[i].x -= 0.1f; box[i].y -= 0.1f;
			}
			std::cout << i << "  " << box[i].move << std::endl;
		}
	}
	if (two) {
		for (int i = 0; i < cnt; i++) {
			box[i].ex = box[i].x;
			box[i].ey = box[i].y;
			if (box[i].x <= -1.0f) box[i].move = 1;
			if (box[i].x + box[i].width >= 1.0f) box[i].move = 2;

			if (box[i].y <= -1.0f) box[i].move = 3;
			if (box[i].y + box[i].height >= 1.0f) box[i].move = 4;

			if (box[i].move == 1) {
				if (box[i].zmove == 2) {
					box[i].x += 0.05f;
					box[i].y -= 0.05f;
					box[i].zmove = 1;
				}
				else {
					box[i].x += 0.05f;
					box[i].y += 0.05f;
					box[i].zmove = 2;
				}

			}
			if (box[i].move == 2) {
				if (box[i].zmove == 2) {
					box[i].x -= 0.05f;
					box[i].y += 0.05f;
					box[i].zmove = 1;
				}
				else {
					box[i].x -= 0.05f;
					box[i].y -= 0.05f;
					box[i].zmove = 2;
				}
			}

			if (box[i].move == 3) {
				if (box[i].zmove == 2) {
					box[i].x += 0.05f;
					box[i].y += 0.05f;
					box[i].zmove = 1;
				}
				else {
					box[i].x -= 0.05f;
					box[i].y += 0.05f;
					box[i].zmove = 2;
				}
			}
			if (box[i].move == 4) {
				if (box[i].zmove == 2) {
					box[i].x -= 0.05f;
					box[i].y -= 0.05f;
					box[i].zmove = 1;
				}
				else {
					box[i].x += 0.05f;
					box[i].y -= 0.05f;
					box[i].zmove = 2;
				}
			}
			std::cout << i << "  " << box[i].move << std::endl;
		}
	}
	if (three) {
		for (int i = 0; i < cnt; i++) {
			if (box[i].width >= 0.5f) box[i].size = 1;
			if (box[i].height >= 0.5f) box[i].size = 2;
			if (box[i].size == 1) {
				box[i].width -= 0.05f;
				box[i].height += 0.05f;
			}
			if (box[i].size == 2) {
				box[i].width += 0.05f;
				box[i].height -= 0.05f;
			}
			if (box[i].size == 0) box[i].width += 0.05f;
		}
	}
	if (four) {
		for (int i = 0; i < cnt; i++) {
			box[i].r = dis(gen);
			box[i].g = dis(gen);
			box[i].b = dis(gen);
		}
	}
	if (five) {
		for (int i = 0; i < cnt; i++) {
			if (click != 0) {
				if (i == click);
			    else if (i - 1 == click) {
					box[i].x = box[click - 1].ex;
					box[i].y = box[click - 1].ey;
				}
				else if (i == 0) {
					box[i].x = box[click].ex;
					box[i].y = box[click].ey;
				}
				else {
					box[i].x = box[i - 1].ex;
					box[i].y = box[i - 1].ey;
				}
			}
			else {
				if (i == 0);
				else {
					box[i].x = box[i - 1].ex;
					box[i].y = box[i - 1].ey;
				}
			}
		}
	}
	glutPostRedisplay();
	// ��� Ÿ�̸Ӹ� ����
	glutTimerFunc(100, TimerFunction, 1);
}