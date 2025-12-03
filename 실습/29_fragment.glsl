#version 330 core

in vec3 outPos;
in vec3 outColor;
in vec3 outNormal;
in vec2 outTexture;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D utexture;

void main() {
    // 텍스처 색상
    vec3 textureColor = texture(utexture, outTexture).rgb;

    // 앰비언트빛 세기
    float ambientLight = 0.3;
    // 앰비언트
    vec3 ambient = ambientLight * lightColor;

    // 법선 벡터
    vec3 normalVector = normalize(outNormal); // 단위 벡터로 변환
    // 빛의 방향
    vec3 lightDir = normalize(lightPos - outPos); // 빛에서 물체로 향하는
    // 디퓨즈
    vec3 diffuse = max(dot(normalVector, lightDir), 0.0) * lightColor;

    // 반짝임 계수
    int shininess = 128;
    // 시선 방향
    vec3 viewDir = normalize(viewPos - outPos); // 카메라에서 물체로 향하는
    // 반사 방향
    vec3 reflectDir = reflect(-lightDir, normalVector); // 물체에서 빛으로 향하는 벡터를 법선 벡터 기준으로 반사
    // 스페큘러
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess) * lightColor;

    // 최종 색상 계산
    vec3 result = (ambient + diffuse + specular) * textureColor;
    color = vec4(result, 1.0);
}