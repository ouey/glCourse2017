#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_GLU
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


// ディレクトリの設定ファイル
#include "common.h.in"



static int WIN_WIDTH   = 500;                       // ウィンドウの幅
static int WIN_HEIGHT  = 500;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル

static const double PI = 4.0 * std::atan(1.0);

// シェーダファイル
static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "shader/objectSelectionDice.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "shader/objectSelectionDice.frag";

//テクスチャファイル
static const std::string TEX_FILE = std::string(DATA_DIRECTORY) + "dice.png";
GLuint textureId = 0u;

// 頂点オブジェクト
struct Vertex {
    Vertex(const glm::vec3 &position_, const glm::vec2 &texture_, const float &surfaceId_)
    : position(position_)
    , texture(texture_)
    , surfaceId(surfaceId_){
    }
    
    glm::vec3 position;
    glm::vec2 texture;
    float surfaceId;
};

static const glm::vec3 positions[8] = {
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f)
};

static const glm::vec2 textures[14] = {
    
    glm::vec2( 0.25, 0.0), //0
    glm::vec2( 0.5, 0.0), //1
    
    glm::vec2 ( 0.0, 0.33333), //2
    glm::vec2( 0.25, 0.3333), //3
    glm::vec2( 0.5, 0.3333), //4
    glm::vec2( 0.75, 0.3333),//5
    glm::vec2( 1.0, 0.33333), //6
    
    glm::vec2( 0.0, 0.666), //7
    glm::vec2( 0.25, 0.666), //8
    glm::vec2( 0.5, 0.666), //9
    glm::vec2( 0.75, 0.666), //10
    glm::vec2( 1.0, 0.666), //11
    
    glm::vec2( 0.25, 1.0), //12
    glm::vec2( 0.5 , 1.0), //13
    
};


static const unsigned int faces[12][3] = {
    { 1, 6, 7 }, { 1, 7, 4 },
    { 2, 5, 7 }, { 2, 7, 4 },
    { 3, 5, 7 }, { 3, 7, 6 },
    { 0, 1, 4 }, { 0, 4, 2 },
    { 0, 1, 6 }, { 0, 6, 3 },
    { 0, 2, 5 }, { 0, 5, 3 }
};

static const unsigned int texID[12][3] = {
    { 9, 13, 12 }, { 9, 12, 8 },  //6.
    { 5, 10, 9 }, {  5, 9, 4 },  //5.
    { 4, 9, 8 }, { 4, 8, 3 },  //3.
    { 6, 11, 10 }, { 6, 10, 5 }, //4.
    { 3, 8, 7 }, { 3, 7, 2 },//2
    { 1, 4, 3 }, { 1, 3, 0 }  //1.
};



// バッファを参照する番号
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

// シェーダを参照する番号
GLuint programId;

int ClickedId = -1;

// 立方体の回転角度
static float theta = 0.0f;


static size_t indexBufferSize = 48;


static const glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);


// オブジェクトを選択するためのID
bool selectMode = false;

// VAOの初期化
void initVAO() {
    
    
    // Vertex配列の作成
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    int idx = 0;
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 3; i++) {
            vertices.push_back(Vertex(positions[faces[face * 2 + 0][i]], textures[texID[face * 2 + 0][i]], float(face+1)));
            indices.push_back(idx++);
        }
        
        for (int i = 0; i < 3; i++) {
            vertices.push_back(Vertex(positions[faces[face * 2 + 1][i]], textures[texID[face * 2 + 1][i]], float(face+1)));
            indices.push_back(idx++);
        }
    }
    
    
    // VAOの作成
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    
    // 頂点バッファの作成
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    
    // 頂点バッファの有効化
    glEnableVertexAttribArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, surfaceId));
    
    
    // 頂点番号バッファの作成
    glGenBuffers(1, &indexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    
    // VAOをOFFにしておく
    glBindVertexArray(0);

}

GLuint compileShader(const std::string &filename, GLuint type) {
    // シェーダの作成
    GLuint shaderId = glCreateShader(type);
    
    // ファイルの読み込み
    std::ifstream reader;
    size_t codeSize;
    std::string code;
    
    // ファイルを開く
    reader.open(filename.c_str(), std::ios::in);
    if (!reader.is_open()) {
        // ファイルを開けなかったらエラーを出して終了
        fprintf(stderr, "Failed to load a shader: %s\n", VERT_SHADER_FILE.c_str());
        exit(1);
    }
    
    // ファイルをすべて読んで変数に格納 (やや難)
    reader.seekg(0, std::ios::end);             // ファイル読み取り位置を終端に移動
    codeSize = reader.tellg();                  // 現在の箇所(=終端)の位置がファイルサイズ
    code.resize(codeSize);                      // コードを格納する変数の大きさを設定
    reader.seekg(0);                            // ファイルの読み取り位置を先頭に移動
    reader.read(&code[0], codeSize);            // 先頭からファイルサイズ分を読んでコードの変数に格納
    
    // ファイルを閉じる
    reader.close();
    
    // コードのコンパイル
    const char *codeChars = code.c_str();
    glShaderSource(shaderId, 1, &codeChars, NULL);
    glCompileShader(shaderId);
    
    // コンパイルの成否を判定する
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        // コンパイルが失敗したらエラーメッセージとソースコードを表示して終了
        fprintf(stderr, "Failed to compile a shader!\n");
        
        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetShaderInfoLog(shaderId, logLength, &length, &errMsg[0]);
            
            // エラーメッセージとソースコードの出力
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
            fprintf(stderr, "%s\n", code.c_str());
        }
        exit(1);
    }
    
    return shaderId;
}

GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile) {
    // シェーダの作成
    GLuint vertShaderId = compileShader(vShaderFile, GL_VERTEX_SHADER);
    GLuint fragShaderId = compileShader(fShaderFile, GL_FRAGMENT_SHADER);
    
    // シェーダプログラムのリンク
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertShaderId);
    glAttachShader(programId, fragShaderId);
    glLinkProgram(programId);
    
    // リンクの成否を判定する
    GLint linkState;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
    if (linkState == GL_FALSE) {
        // リンクに失敗したらエラーメッセージを表示して終了
        fprintf(stderr, "Failed to link shaders!\n");
        
        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetProgramInfoLog(programId, logLength, &length, &errMsg[0]);
            
            // エラーメッセージを出力する
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
        }
        exit(1);
    }
    
    // シェーダを無効化した後にIDを返す
    glUseProgram(0);
    return programId;
}

// シェーダの初期化
void initShaders() {
    programId = buildShaderProgram(VERT_SHADER_FILE, FRAG_SHADER_FILE);
}

//テクスチャの初期化
void initTexture(){
    
    int texWidth, texHeight, channels;
    unsigned char *bytes = stbi_load(TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
    if (!bytes) {
        fprintf(stderr, "Failed to load image file: %s\n", TEX_FILE.c_str());
        exit(1);
    }
    
    // テクスチャの生成と有効化
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // 単純なテクスチャの転送
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    
    // テクスチャの画素値参照方法の設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // テクスチャ境界の折り返し設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // テクスチャの無効化
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // ロードした画素情報の破棄
    stbi_image_free(bytes);
    
    
}


// OpenGLの初期化関数
void initializeGL() {
    // 深度テストの有効化
    glEnable(GL_DEPTH_TEST);
    
    // 背景色の設定 (黒)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // VAOの初期化
    initVAO();
    
    initTexture();

    
    // シェーダの用意
    initShaders();
    
}

// OpenGLの描画関数
void paintGL() {
    // 背景色の描画
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // シェーダの有効化
    glUseProgram(programId);
    
    // テクスチャの有効化
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // VAOの有効化
    glBindVertexArray(vaoId);
    
    // 1つ目の立方体を描画
    // 座標の変換
        glm::mat4 projMat = glm::perspective(45.0f,(float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
    
        glm::mat4 viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // 視点の位置
                                        glm::vec3(0.0f, 0.0f, 0.0f),   // 見ている先
                                        glm::vec3(0.0f, 1.0f, 0.0f));  // 視界の上方向
        glm::mat4 modelMat;
        modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));
        modelMat = glm::rotate(modelMat, theta, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 mvpMat = projMat * viewMat * modelMat;
        
        // Uniform変数の転送
        GLuint uid;
        uid = glGetUniformLocation(programId, "u_mvpMat");
        glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
        uid = glGetUniformLocation(programId, "u_selectID");
        glUniform1i(uid, selectMode ? 1 : -1);
        
        uid = glGetUniformLocation(programId, "u_texture");
        glUniform1i(uid, 0);
        
        glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
    
    // クリックされた面のIdを転送
    uid = glGetUniformLocation(programId, "u_clickedId");
    glUniform1i(uid, ClickedId);
    
    
    // VAOの無効化
    glBindVertexArray(0);
    
    // シェーダの無効化
    glUseProgram(0);
    
    // テクスチャの無効化
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

}


void resizeGL(GLFWwindow *window, int width, int height) {
    // ユーザ管理のウィンドウサイズを変更
    WIN_WIDTH = width;
    WIN_HEIGHT = height;
    
    // GLFW管理のウィンドウサイズを変更
    glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);
    
    // 実際のウィンドウサイズ (ピクセル数) を取得
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
    
    // ビューポート変換の更新
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);
}

void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        // クリックされた位置を取得
        double px, py;
        glfwGetCursorPos(window, &px, &py);
        const int cx = (int)px;
        const int cy = (int)py;
        
        // 選択モードでの描画
        selectMode = true;
        paintGL();
        selectMode = false;
        
        // ピクセルの大きさの計算 (Macの場合には必要)
        int renderBufferWidth, renderBufferHeight;
        glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
        int pixelSize = std::max(renderBufferWidth / WIN_WIDTH, renderBufferHeight / WIN_HEIGHT);
        
        // より適切なやり方
        unsigned char byte[4];
        glReadPixels(cx * pixelSize, (WIN_HEIGHT - cy - 1) * pixelSize, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &byte);
        
        ClickedId = (int)byte[0];
        
        printf("Mouse position: %d %d\n", cx, cy);
        printf("Select object %d\n", (int)byte[0]);
    }
}

// アニメーションのためのアップデート
void animate() {
    theta += 2.0f * PI / 360.0f;  // 10分の1回転
}

int main(int argc, char **argv) {
    // OpenGLを初期化する
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }
    
    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Windowの作成
    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed!");
        glfwTerminate();
        return 1;
    }
    
    // OpenGLの描画対象にWindowを追加
    glfwMakeContextCurrent(window);
    
    // マウスのイベントを処理する関数を登録
    glfwSetMouseButtonCallback(window, mouseEvent);
    
    // GLEWを初期化する (glfwMakeContextCurrentの後でないといけない)
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed!\n");
        return 1;
    }
    
    // ウィンドウのリサイズを扱う関数の登録
    glfwSetWindowSizeCallback(window, resizeGL);
    
    // OpenGLを初期化
    initializeGL();
    
    // メインループ
    while (glfwWindowShouldClose(window) == GL_FALSE) {
        // 描画
        paintGL();
        
        // アニメーション
        animate();
        
        // 描画用バッファの切り替え
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
}
