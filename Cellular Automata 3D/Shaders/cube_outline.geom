#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			vec4 pos1 = gl_in[i].gl_Position;
			vec4 pos2 = gl_in[j].gl_Position;

			int count = 0;
			if (gl_in[i].gl_Position.x == gl_in[j].gl_Position.x) count++;			
			if (gl_in[i].gl_Position.y == gl_in[j].gl_Position.y) count++;
			if (gl_in[i].gl_Position.z == gl_in[j].gl_Position.z) count++;

			if (count == 2)
			{
				gl_Position = projection * view * gl_in[i].gl_Position;
				EmitVertex();
				gl_Position = projection * view * gl_in[j].gl_Position;
				EmitVertex();
				EndPrimitive();
			}
		}
	}
}

