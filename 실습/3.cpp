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
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
float MouseX(int x) {
	return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
}
float MouseY(int y) {
	return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT);
}
bool isOverlap(float ax1, float ay1, float ax2, float ay2, float bx1, float by1, float bx2, float by2) {
	return !(ax2 < bx1 || bx2 < ax1 || ay2 < by1 || by2 < ay1);
}
//--- 전역 변수 선언
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);
std::uniform_real_distribution<float> mis(-1.0f, 0.9f);
std::uniform_real_distribution<float> bis(0.1f, 0.2f);

struct BigBox {
	float rx1 = -2, ry1 = -2;
	float rx2 = -2, ry2 = -2;
	float r = dis(gen), g = dis(gen), b = dis(gen);
	int c = -1, x = -1;
};

BigBox bigbox[100];
int cnt = -1, move = -1, total = 0;
bool left_button = false;
float bigbox_offset_x = 0.0f, bigbox_offset_y = 0.0f;



//--- 윈도우 출력하고 콜백함수 설정
void main(int argc, char** argv) {
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example3"); // 윈도우 생성 (윈도우 이름)
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
	glutMotionFunc(Motion); // 마우스 이동
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 콜백 함수: 출력 콜백 함수
GLvoid drawScene() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.
	for (int i = 0; i < 100; i++) {
		//glColor3f(box[i].r, box[i].g, box[i].b);
		//glRectf(box[i].rx, box[i].ry, box[i].rx + 0.1, box[i].ry + 0.1);
		glColor3f(bigbox[i].r, bigbox[i].g, bigbox[i].b);
		glRectf(bigbox[i].rx1, bigbox[i].ry1, bigbox[i].rx2, bigbox[i].ry2);
	}
	
	glutSwapBuffers(); // 화면에 출력하기
}

//--- 콜백 함수: 다시 그리기 콜백 함수
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'a':
			if (total >= 30) break;
			cnt++;
			bigbox[cnt].rx1 = mis(gen);
			bigbox[cnt].rx2 = bigbox[cnt].rx1 + 0.1f;
			bigbox[cnt].ry1 = mis(gen);
			bigbox[cnt].ry2 = bigbox[cnt].ry1 + 0.1f;
			total++;
			break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < cnt + 1; i++) {
			if (((MouseX(x) >= bigbox[i].rx1) && (MouseX(x) <= bigbox[i].rx2)) && ((MouseY(y) >= bigbox[i].ry1) && (MouseY(y) <= bigbox[i].ry2))) {
				left_button = true;
				move = i;
				float cx = (bigbox[i].rx1 + bigbox[i].rx2) / 2.0f;
				float cy = (bigbox[i].ry1 + bigbox[i].ry2) / 2.0f;
				bigbox_offset_x = MouseX(x) - cx;
				bigbox_offset_y = MouseY(y) - cy;
				std::cout << "box " << i << " is selected\n";
			}
		}
		
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		for (int i = 0; i < cnt + 1; i++) {
			if (i == move) continue;
			if (isOverlap(bigbox[move].rx1,bigbox[move].ry1, bigbox[move].rx2, bigbox[move].ry2,
				bigbox[i].rx1, bigbox[i].ry1, bigbox[i].rx2, bigbox[i].ry2)) {
				cnt++;
				bigbox[cnt].rx1 = std::min(bigbox[move].rx1, bigbox[i].rx1);
				bigbox[cnt].rx2 = std::max(bigbox[move].rx2, bigbox[i].rx2);
				bigbox[cnt].ry1 = std::min(bigbox[move].ry1, bigbox[i].ry1);
				bigbox[cnt].ry2 = std::max(bigbox[move].ry2, bigbox[i].ry2);
				bigbox[cnt].r = dis(gen);
				bigbox[cnt].g = dis(gen);
				bigbox[cnt].b = dis(gen);
				bigbox[cnt].c = move;
				bigbox[cnt].x = i;
				bigbox[move].rx1 = -2, bigbox[move].ry1 = -2;
				bigbox[move].rx2 = -2, bigbox[move].ry2 = -2;
				bigbox[i].rx1 = -2, bigbox[i].ry1 = -2;
				bigbox[i].rx2 = -2, bigbox[i].ry2 = -2;
				total--;
				break;
			}
		}
		move = -1;
		left_button = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (total >= 30) return;
		for (int i = 0; i < cnt + 1; i++) {
			if (((MouseX(x) >= bigbox[i].rx1) && (MouseX(x) <= bigbox[i].rx2)) && ((MouseY(y) >= bigbox[i].ry1) && (MouseY(y) <= bigbox[i].ry2))) {
				if (bigbox[i].c == -1) break;
				bigbox[bigbox[i].c].rx1 = bigbox[i].rx1;
				bigbox[bigbox[i].c].rx2 = bigbox[i].rx1 + bis(gen);
				bigbox[bigbox[i].c].ry1 = bigbox[i].ry1;
				bigbox[bigbox[i].c].ry2 = bigbox[i].ry1 + bis(gen);
				bigbox[bigbox[i].x].rx1 = bigbox[bigbox[i].c].rx2 + 0.01f;
				bigbox[bigbox[i].x].rx2 = bigbox[bigbox[i].x].rx1 + bis(gen);
				bigbox[bigbox[i].x].ry1 = bigbox[i].ry1;
				bigbox[bigbox[i].x].ry2 = bigbox[i].ry1 + bis(gen);
				bigbox[bigbox[i].c].r = dis(gen), bigbox[bigbox[i].c].g = dis(gen), bigbox[bigbox[i].c].b = dis(gen);
				bigbox[bigbox[i].x].r = dis(gen), bigbox[bigbox[i].x].g = dis(gen), bigbox[bigbox[i].x].b = dis(gen);
				bigbox[i].rx1 = -2, bigbox[i].ry1 = -2;
				bigbox[i].rx2 = -2, bigbox[i].ry2 = -2;
				bigbox[i].c = -1, bigbox[i].x = -1;
				total++;
				break;
			}
		}
	}
	glutPostRedisplay();
}

void Motion(int x, int y) {
	if (left_button) {
		float cx = MouseX(x) - bigbox_offset_x;
		float cy = MouseY(y) - bigbox_offset_y;
		float half_w = (bigbox[move].rx2 - bigbox[move].rx1) / 2.0f;
		float half_h = (bigbox[move].ry2 - bigbox[move].ry1) / 2.0f;
		bigbox[move].rx1 = cx - half_w;
		bigbox[move].rx2 = cx + half_w;
		bigbox[move].ry1 = cy - half_h;
		bigbox[move].ry2 = cy + half_h;
	}
	glutPostRedisplay();
}