#version 120

// Constant across both shaders
uniform sampler2D underwaterScene;

// Values passed in from the vertex shader
varying vec3 vNormal;
varying vec3 vPosition;
varying vec2 vTextureCoord0;

void main() {
	gl_FragColor = vec4(0, 0, 1, 0.5);
}