#include <iostream>
#include <random>
//--- �ʿ��� ������� include
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

//--- ������ ����ϰ� �ݹ��Լ� ����
void main(int argc, char** argv) { 
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ���� (������ �̸�)
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
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

//--- �ݹ� �Լ�: ��� �ݹ� �Լ�
GLvoid drawScene() {
	glClearColor(a, b, c, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
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
			glutTimerFunc(1000, TimerFunction, 1); // Ÿ�̸�
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