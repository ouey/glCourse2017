#version 330

// Varying変数
in vec2 f_fragTexture;
in float f_surfaceId;
//in float darkness;


// ディスプレイへの出力変数
out vec4 out_color;

// 選択を判定するためのID
uniform int u_selectID;

uniform sampler2D u_texture;

//クリックされた面を判別するID
uniform int u_clickedId;

float brightness = 0.5; //暗くする係数

void main() {
    if (u_selectID > 0) {
        // 選択のIDが0より大きければIDで描画する
        float c = f_surfaceId / 255.0;
        out_color = vec4(c, c, c, 1.0);
    } else {
        // 描画色を代入
        //クリックされた面番号のみ明るく描画する
        if(int(u_clickedId) == f_surfaceId)  out_color = vec4(texture(u_texture, f_fragTexture).rgb, 1.0);
        else out_color = vec4(texture(u_texture, f_fragTexture).rgb * brightness , 1.0);
        
    }
}
