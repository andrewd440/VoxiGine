#version 430 core

layout (binding = 0) uniform usampler2D GBuffer0;
layout (binding = 2) uniform sampler2D DepthTexture;

layout(std140, binding = 2) uniform TransformBlock
{
// Member					Base Align		Aligned Offset		End
	mat4 Model;				//		16					0			64
	mat4 View;				//		16					64			128
	mat4 Projection;		//		16					128			192
	mat4 InvProjection;		//      16                  192         256
} Transforms;

layout(std140, binding = 4) uniform ResolutionBlock
{
	uvec2 Resolution;
};

layout(std140, binding = 1) uniform ProjectionInfoBlock
{
	float Near;
	float Far;
} ProjectionInfo;


float GetLinearDepth(ivec2 ScreenCoord)
{
	float Depth = texelFetch(DepthTexture, ScreenCoord, 0).r;
    float z = Depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * ProjectionInfo.Near) / (ProjectionInfo.Far + ProjectionInfo.Near - z * (ProjectionInfo.Far - ProjectionInfo.Near));	
}

vec3 GetNormal(ivec2 ScreenCoord)
{
	uvec4 Data0 = texelFetch(GBuffer0, ScreenCoord, 0);
	vec2 ColorZNormX = unpackHalf2x16(Data0.y);
	return normalize(vec3(ColorZNormX.y, unpackHalf2x16(Data0.z)));
}

const ivec2 Offsets[9] = 
{
	ivec2( 0,  0), // 0
	ivec2(-1,  1), // 1
	ivec2( 0,  1), // 2
	ivec2( 1,  1), // 3
	ivec2(-1,  0), // 4
	ivec2( 1,  0), // 5
	ivec2(-1, -1), // 6
	ivec2( 0, -1), // 7
	ivec2( 1, -1), // 8
};

uniform int SampleDistance = 1;
uniform float DepthSensitivity = 2.5;

void main()
{
	ivec2 FragPosition = ivec2(gl_FragCoord.xy);
	vec2 FResolution = vec2(Resolution);
	vec2 TexCoord = vec2(FragPosition) / FResolution;

	float Depths[9];
	vec3 Normals[9];

	for(int i = 0; i < 9; i++)
	{
		ivec2 TexCoord = FragPosition + SampleDistance * Offsets[i];
		Depths[i] = GetLinearDepth(TexCoord);
		Normals[i] = GetNormal(TexCoord);
	}

	vec3 HNSum = -Normals[1] + Normals[3] - 2 * Normals[4] + 2 * Normals[5] - Normals[6] +  Normals[8];
	vec3 VNSum = -Normals[1] - 2 * Normals[2] - Normals[3] + Normals[6] + 2 * Normals[7] + Normals[8];
	float Mag = length((HNSum + VNSum) / 2.0);

	// Cutoff at specific depth, prevents black areas
	if(Depths[0] * ProjectionInfo.Far <= 25.0)
	{
		float HDSum = -Depths[1] + Depths[3] - 2 * Depths[4] + 2 * Depths[5] - Depths[6] + Depths[8];
		float VDSum = -Depths[1] - 2 * Depths[2] - Depths[3] + Depths[6] + 2 * Depths[7] + Depths[8];
		Mag = max(DepthSensitivity * sqrt(HDSum * HDSum + VDSum * VDSum), Mag);
	}

	gl_FragColor = vec4(vec3((Mag >= 0.05) ? 0.2 : 1.0), 1.0);
}