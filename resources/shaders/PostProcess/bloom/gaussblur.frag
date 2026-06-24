#version 450

layout (binding = 1) uniform sampler2D samplerColor;

layout (binding = 0) uniform UBO 
{
	float blurScale;
	float blurStrength;
	float sigma;
	int kernelSize;
} ubo;

layout (constant_id = 0) const int blurdirection = 0;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec2 tex_offset = 1.0 / textureSize(samplerColor, 0) * ubo.blurScale;

	vec3 result = texture(samplerColor, inUV).rgb;
	float totalWeight = 1.0;

	for (int i = 1; i <= ubo.kernelSize; ++i)
	{
		float weight = exp(-(float(i * i)) / (2.0 * ubo.sigma * ubo.sigma));
		totalWeight += 2.0 * weight;

		if (blurdirection == 1)
		{
			result += (texture(samplerColor, inUV + vec2(tex_offset.x * i, 0.0)).rgb +
					   texture(samplerColor, inUV - vec2(tex_offset.x * i, 0.0)).rgb) * weight;
		}
		else
		{
			result += (texture(samplerColor, inUV + vec2(0.0, tex_offset.y * i)).rgb +
					   texture(samplerColor, inUV - vec2(0.0, tex_offset.y * i)).rgb) * weight;
		}
	}

	result = (result / totalWeight) * ubo.blurStrength;
	outFragColor = vec4(result, 1.0);
}
