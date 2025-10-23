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
std::uniform_real_distribution<float> mis(-1.0f, 1.0f);

struct Box {
	float x = mis(gen), y = mis(gen);
	float width = 0.1f, height = 0.1f;
	float r = dis(gen), g = dis(gen), b = dis(gen);
};
Box box[100];
int total = 30, cnt = 30;
bool click = false, eat = false;
float cx, cy, size = 0.1f;
float r = 0.0f, g = 0.0f, b = 0.0f;

//--- ������ ����ϰ� �ݹ��Լ� ����
void main(int argc, char** argv) {
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example5"); // ������ ���� (������ �̸�)
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
	glutMotionFunc(Motion); // ���콺 ������
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

//--- �ݹ� �Լ�: ��� �ݹ� �Լ�
GLvoid drawScene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.

	for (int i = 0; i < cnt; i++) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glColor3f(box[i].r, box[i].g, box[i].b);
		glRectf(box[i].x, box[i].y, box[i].x + box[i].width, box[i].y + box[i].height);
	}
	if (click) {
		if (eat) glPolygonMode(GL_FRONT, GL_FILL);
		else glPolygonMode(GL_FRONT, GL_LINE); // ������ �׸���
		glColor3f(r, g, b);
		glRectf(cx, cy, cx + size, cy + size);
	}
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		std::cout << "key r" << std::endl;
		total = cnt = 30;
		for (int i = 0; i < cnt; i++) {
			box[i].x = mis(gen); box[i].y = mis(gen);
			box[i].r = dis(gen); box[i].g = dis(gen); box[i].b = dis(gen);
		}
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
		click = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		click = false;
		eat = false;
		size = 0.1f;
		r = 0.0f, g = 0.0f, b = 0.0f;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (total >= 30) return;
		box[cnt].x = MouseX(x) - 0.05f;
		box[cnt].y = MouseY(y) - 0.05f;
		box[cnt].r = dis(gen); box[cnt].g = dis(gen); box[cnt].b = dis(gen);
		total++;
		cnt++;
		std::cout << "mouse right click" << std::endl;
	}
	glutPostRedisplay();
}

void Motion(int x, int y) {
	if (click) {
		cx = MouseX(x) - size / 2;
		cy = MouseY(y) - size / 2;
		for (int i = 0; i < cnt; i++) {
			if (isRectangle(cx, cy, cx + size, cy + size, box[i].x, box[i].y, box[i].x + box[i].width, box[i].y + box[i].height)) {
				eat = true;
				total--;
				r = box[i].r; g = box[i].g; b = box[i].b;
				size += 0.05f;
				box[i].x = -2; box[i].y = -2;
			}
		}
		std::cout << cx << ", " << cy << std::endl;
	}
	glutPostRedisplay();
}