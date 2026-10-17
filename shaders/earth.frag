#version 330 core
in vec3 vWorldNormal,vWorldPosition;
in vec2 vUV;
out vec4 fragmentColor;
uniform sampler2D uEarthTexture;
uniform bool uHasTexture;
uniform vec3 uSunWorld,uCameraWorld;
void main(){
 vec3 N=normalize(vWorldNormal);
 float cosine=dot(N,normalize(uSunWorld));
 // Visualization approximation, NOT radiative transfer.
 float daylight=smoothstep(-0.035,0.08,cosine);
 float illumination=0.035+daylight*(0.17+0.795*max(cosine,0.0));
 vec3 observed=uHasTexture?texture(uEarthTexture,vUV).rgb:vec3(0.22,0.29,0.35);
 vec3 result=observed*illumination;
 float limb=pow(1.0-max(0.0,dot(N,normalize(uCameraWorld-vWorldPosition))),5.0);
 result+=vec3(0.01,0.04,0.075)*limb*daylight;
 fragmentColor=vec4(result,1.0);
}
