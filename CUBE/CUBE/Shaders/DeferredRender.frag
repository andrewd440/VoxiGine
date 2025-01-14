#version 430 core

layout (location = 0) out uvec4 color0;

in VS_OUT 
{
	vec3 Normal;
	vec3 Color;
	flat uint MaterialID;
} fs_in;

void main()
{
	uvec4 OutVec0 = uvec4(0);

	OutVec0.x = packHalf2x16(fs_in.Color.xy);
	OutVec0.y = packHalf2x16(vec2(fs_in.Color.z, fs_in.Normal.x));
	OutVec0.z = packHalf2x16(fs_in.Normal.yz);
	OutVec0.w = fs_in.MaterialID;

	color0 = OutVec0;
}