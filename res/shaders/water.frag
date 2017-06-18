#version 140

uniform sampler2D waterGradientMap;

varying vec2 waterCoord;
varying float reflectionAmount;

// Values passed in from the vertex shader
varying vec3 vNormal;
varying vec3 vPosition;
varying vec2 vTextureCoord0;

void main() {
    vec3 viewDirection = (gl_ModelViewMatrix * vec4(0, 0, -1, 1)).xyz;
    vec2 gradient = texture2D(waterGradientMap, waterCoord).rg;

    vec3 normal = gl_NormalMatrix * normalize(vec3(gradient.x, 1, gradient.y));

    float incident = 1 - dot(normal, normalize(-vPosition));

    gl_FragColor = vec4(1, 1, 1, incident*incident);
    //gl_FragColor = vec4(vPosition/15, 1);
}