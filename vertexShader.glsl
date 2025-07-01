#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTexCoord;
//vec2 aTexCoord = vec2(0.,0.);

out VS_OUT {
    vec2 TexCoord;
    int vertexIndex;
    vec4 pos3d;
} vs_out;

#define MAX_VERTEX  1000
uniform float time;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 dynamicPos[MAX_VERTEX];
void main()
{
    
    vs_out.TexCoord = aTexCoord;
    int id = gl_VertexID;
    vs_out.vertexIndex = id;

    vec3 pos2 = pos;
    pos2 = dynamicPos[id];

    

    vec4 pos3d = modelMatrix*vec4(pos2.x, pos2.y, pos2.z, 1.0);
    vs_out.pos3d = pos3d;
    gl_Position = projMatrix*viewMatrix*pos3d;
}