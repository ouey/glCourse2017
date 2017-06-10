#version 330

// Attribute変数
layout(location = 0) in vec3 in_position;//位置
layout(location = 1) in vec2 in_texture;//テクスチャ

layout(location = 2) in float in_surfaceId;//面番号

// Varying変数
out vec2 f_fragTexture;
out float f_surfaceId;

// Uniform変数
uniform mat4 u_mvpMat;


void main() {
    // gl_Positionは頂点シェーダの組み込み変数
    // 指定を忘れるとエラーになるので注意
    gl_Position = u_mvpMat * vec4(in_position, 1.0);
    
    f_fragTexture = in_texture; //テクスチャをフラグメントシェーダに転送
    
    f_surfaceId = in_surfaceId;//面番号をフラグメントシェーダに転送
    
}
