#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
uniform mat4 uModel,uView,uProjection;
out vec3 vWorldNormal,vWorldPosition;
out vec2 vUV;
void main(){
 vec4 p=uModel*vec4(aPosition,1.0);
 vWorldPosition=p.xyz;
 vWorldNormal=normalize(mat3(uModel)*aNormal);
 vUV=aUV;
 gl_Position=uProjection*uView*p;
}
