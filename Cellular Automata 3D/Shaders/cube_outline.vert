#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(aPos + vec3(0.001, 0.001, 0.001), 1.0);
}

