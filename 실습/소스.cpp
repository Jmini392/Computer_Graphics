#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// OpenGL 라이브러리 포함
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

// STB 이미지 로더 포함
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --- 자료 구조 정의 ---
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 color;    // 색상 추가
};

// --- 전역 변수 ---
GLint width = 800, height = 600;
GLuint shaderProgramID;
GLuint VAO, VBO, EBO;
GLuint textureID = 0;  // 텍스처 ID 추가
std::vector<unsigned int> indices;
float rotationAngle = 0.0f;
bool useTexture = true;  // 텍스처 사용 여부

// --- 텍스처 로딩 함수 ---
GLuint loadTexture(const char* path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // 텍스처 파라미터 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 이미지 로드
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // OpenGL은 Y축이 뒤집어져 있음
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        std::cout << "Texture loaded successfully: " << path << " (" << width << "x" << height << ", channels: " << nrChannels << ")" << std::endl;
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    
    stbi_image_free(data);
    return texture;
}

// --- 삼각분할 함수 (색상 인자 추가) ---
void triangulateFace(const std::vector<std::vector<unsigned int>>& face_indices,
                     const std::vector<glm::vec3>& temp_positions,
                     const std::vector<glm::vec2>& temp_texCoords,
                     const std::vector<glm::vec3>& temp_normals,
                     const glm::vec3& face_color,  // 면의 색상 추가
                     std::vector<Vertex>& out_vertices,
                     std::vector<unsigned int>& out_indices) {
    
    // 면이 삼각형이 아닌 경우 삼각분할
    if (face_indices.size() >= 3) {
        for (size_t i = 1; i < face_indices.size() - 1; ++i) {
            // 삼각형의 세 정점에 대해 정점 생성
            std::vector<std::vector<unsigned int>> triangle = {
                face_indices[0], 
                face_indices[i], 
                face_indices[i + 1]
            };
            
            for (const auto& vertex_indices : triangle) {
                Vertex vertex;
                
                unsigned int pos_idx = vertex_indices[0];
                unsigned int tex_idx = vertex_indices[1];
                unsigned int norm_idx = vertex_indices[2];
                
                // 위치 설정
                if (pos_idx < temp_positions.size()) {
                    vertex.position = temp_positions[pos_idx];
                } else {
                    vertex.position = glm::vec3(0.0f);
                }
                
                // 텍스처 좌표 설정
                if (tex_idx < temp_texCoords.size()) {
                    vertex.texCoord = temp_texCoords[tex_idx];
                } else {
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                }
                
                // 법선 설정 - 법선이 없으면 기본값 사용
                if (!temp_normals.empty() && norm_idx < temp_normals.size()) {
                    vertex.normal = temp_normals[norm_idx];
                } else {
                    // 법선이 없으면 기본 법선 사용 (위쪽 방향)
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                
                // 색상 설정
                vertex.color = face_color;
                
                // 정점 추가 (중복 검사 없이)
                out_vertices.push_back(vertex);
                out_indices.push_back(out_vertices.size() - 1);
            }
        }
    }
}

// --- OBJ 로더 함수 ---
bool loadOBJ(const char* path, std::vector<Vertex>& out_vertices, std::vector<unsigned int>& out_indices) {
    std::cout << "Loading OBJ file " << path << "...\n";

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return false;
    }

    // 다양한 색상 팔레트 정의
    std::vector<glm::vec3> color_palette = {
        glm::vec3(0.8f, 0.6f, 0.4f),   // 베이지
        glm::vec3(0.6f, 0.4f, 0.2f),   // 갈색
        glm::vec3(0.7f, 0.3f, 0.3f),   // 빨강
        glm::vec3(0.3f, 0.7f, 0.3f),   // 초록
        glm::vec3(0.3f, 0.3f, 0.7f),   // 파랑
        glm::vec3(0.9f, 0.9f, 0.2f),   // 노랑
        glm::vec3(0.7f, 0.2f, 0.7f),   // 보라
        glm::vec3(0.2f, 0.7f, 0.7f),   // 청록
    };
    
    int face_count = 0;  // 면의 개수를 세기 위한 카운터

    std::string line;
    while (std::getline(file, line)) {
        // 빈 줄이나 주석 무시
        if (line.empty() || line[0] == '#') continue;
        
        std::stringstream ss(line);
        std::string line_header;
        ss >> line_header;

        if (line_header == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_positions.push_back(vertex);
        }
        else if (line_header == "vt") {
            glm::vec2 texcoord;
            ss >> texcoord.x >> texcoord.y;
            temp_texCoords.push_back(texcoord);
        }
        else if (line_header == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (line_header == "f") {
            std::string vertex_str;
            std::vector<std::vector<unsigned int>> face_indices; // [정점번호][v, vt, vn]
            
            while (ss >> vertex_str) {
                std::vector<unsigned int> vertex_indices = {0, 0, 0}; // v, vt, vn
                
                // v/vt/vn 형식 파싱
                size_t pos1 = vertex_str.find('/');
                if (pos1 != std::string::npos) {
                    vertex_indices[0] = std::stoul(vertex_str.substr(0, pos1));
                    
                    size_t pos2 = vertex_str.find('/', pos1 + 1);
                    if (pos2 != std::string::npos) {
                        // v/vt/vn 또는 v//vn 형식
                        if (pos2 - pos1 > 1) {
                            vertex_indices[1] = std::stoul(vertex_str.substr(pos1 + 1, pos2 - pos1 - 1));
                        }
                        vertex_indices[2] = std::stoul(vertex_str.substr(pos2 + 1));
                    } else {
                        // v/vt 형식
                        vertex_indices[1] = std::stoul(vertex_str.substr(pos1 + 1));
                    }
                } else {
                    // v 형식
                    vertex_indices[0] = std::stoul(vertex_str);
                }
                
                // OBJ 인덱스는 1부터 시작하므로 0 기반으로 변환
                if (vertex_indices[0] > 0) vertex_indices[0] -= 1;
                if (vertex_indices[1] > 0) vertex_indices[1] -= 1;
                if (vertex_indices[2] > 0) vertex_indices[2] -= 1;
                
                face_indices.push_back(vertex_indices);
            }
            
            // 각 면마다 다른 색상 할당 (팔레트에서 순환)
            glm::vec3 current_color = color_palette[face_count % color_palette.size()];
            
            // 삼각분할 함수 호출 (색상 전달)
            triangulateFace(face_indices, temp_positions, temp_texCoords, temp_normals, 
                          current_color, out_vertices, out_indices);
            
            face_count++;  // 면 카운터 증가
        }
    }

    std::cout << "OBJ loaded successfully. Vertices: " << out_vertices.size()
        << ", Indices: " << out_indices.size() << ", Faces: " << face_count << std::endl;
    return true;
}

// --- 셰이더 코드 (텍스처 지원 추가) ---
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;   // 색상 속성 추가

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 VertexColor;   // 프래그먼트 셰이더로 전달할 색상

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoord;
    VertexColor = aColor;   // 색상 전달
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;   // 정점에서 받은 색상

uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;  

    // 텍스처 또는 정점 색상 선택
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(texture1, TexCoords).rgb;
    } else {
        baseColor = VertexColor;
    }

    // 최종 색상 계산
    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
)";

// --- 셰이더 컴파일 함수 ---
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// --- 셰이더 프로그램 생성 함수 ---
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// --- 디스플레이 콜백 함수 ---
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 정점이 없으면 그리지 않음
    if (indices.empty()) {
        glutSwapBuffers();
        return;
    }
    
    glUseProgram(shaderProgramID);

    // 텍스처 바인딩
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgramID, "texture1"), 0);
    }
    
    // 텍스처 사용 여부 설정
    glUniform1i(glGetUniformLocation(shaderProgramID, "useTexture"), useTexture ? 1 : 0);

    // Model 행렬
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // View 행렬
    glm::mat4 view = glm::lookAt(
        glm::vec3(5.0f, 5.0f, 8.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Projection 행렬
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), 
        (float)width / (float)height, 
        0.1f, 
        100.0f
    );

    // 유니폼 설정
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f(glGetUniformLocation(shaderProgramID, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shaderProgramID, "lightPos"), 10.0f, 10.0f, 10.0f);
    glUniform3f(glGetUniformLocation(shaderProgramID, "viewPos"), 5.0f, 5.0f, 8.0f);

    // 모델 그리기
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glutSwapBuffers();
}

// --- 타이머 콜백 함수 ---
void timerFunc(int value) {
    rotationAngle += 0.5f;
    if (rotationAngle >= 360.0f) {
        rotationAngle = 0.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
}

// --- 키보드 콜백 함수 ---
void keyBoard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC 키
        exit(0);
        break;
    case 'w': // 와이어프레임 모드
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case 's': // 솔리드 모드
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case 't': // 텍스처 토글
        useTexture = !useTexture;
        std::cout << "Texture " << (useTexture ? "enabled" : "disabled") << std::endl;
        break;
    }
    glutPostRedisplay();
}

// --- 초기화 함수 ---
void initGL() {
    // GLEW 초기화
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }

    // OpenGL 설정
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // 셰이더 프로그램 생성
    shaderProgramID = createShaderProgram();

    // 텍스처 로드
    textureID = loadTexture("Farming_Color_Palette_01.png");

    // OBJ 파일 로드
    std::vector<Vertex> vertices;
    if (!loadOBJ("farm.obj", vertices, indices)) {
        std::cerr << "Failed to load OBJ file. No fallback model will be created." << std::endl;
        return;
    }

    // 정점이 없으면 종료
    if (vertices.empty() || indices.empty()) {
        std::cerr << "No valid geometry loaded." << std::endl;
        return;
    }

    // VAO, VBO, EBO 설정
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 정점 속성 설정
    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture Coords (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    
    // Normal (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    
    // Color (location = 3) - 새로 추가
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    std::cout << "Model loaded successfully with " << vertices.size() << " vertices and " 
              << indices.size() / 3 << " triangles." << std::endl;
}

// --- 메인 함수 ---
int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Textured OBJ Model Viewer");

    // OpenGL 초기화
    initGL();

    // 콜백 함수 설정
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyBoard);
    glutTimerFunc(0, timerFunc, 0);

    std::cout << "Controls:" << std::endl;
    std::cout << "W - Wireframe mode" << std::endl;
    std::cout << "S - Solid mode" << std::endl;
    std::cout << "T - Toggle texture/color" << std::endl;
    std::cout << "ESC - Exit" << std::endl;

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}