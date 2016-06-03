STRINGIFY(
\n#version 120\n
\n#extension GL_ARB_texture_rectangle : enable\n

\n#define MAX_BLENDS 8\n

struct Edges
{
	float top;
	float left;
	float bottom;
	float right;
};

uniform float uBlendPower[MAX_BLENDS];
uniform float uLuminanceControl[MAX_BLENDS];
uniform float uGammaCorrection[MAX_BLENDS];

//uniform int uNumBlends;

uniform sampler2DRect uImage;
uniform vec2 uDimensions;
uniform vec2 uOffset;

uniform Edges uOverlap;
uniform Edges uBlankout;

uniform float uSolidEdgeEnabled;
uniform vec4	uSolidEdgeColor;

//uniform float projectors;
//uniform float threshold;

vec4 drawSmoothEdge(float loc, float extend, in float blendPower, in float luminanceControl, in vec3 gammaCorrection)
{
	float curve = loc / extend;
	if (curve < 0.5)
	{
		curve = luminanceControl * pow(2.0 * curve, blendPower);
	}
	else
	{
		curve = 1.0 - (1.0 - luminanceControl) * pow(2.0 * (1.0 - curve), blendPower);
	}

	return vec4(pow(curve, 1.0 / gammaCorrection.r),
		pow(curve, 1.0 / gammaCorrection.g),
		pow(curve, 1.0 / gammaCorrection.b),
		1.0);
}

vec4 drawSmoothEdges(in Edges overlap, in Edges blankout, in vec2 texCoord, in vec2 offset, in vec2 dimensions, in sampler2DRect sampler, in float blendPower, in float luminanceControl, in vec3 gammaCorrection)
{
	vec2 offsetCoord = texCoord + offset;
	vec4 blank = vec4(0.0);

	if (blankout.left + overlap.left > texCoord.x)
	{
		if (blankout.left > texCoord.x)
		{
			return blank;  //leftBlankout
		}

		if (blankout.bottom + overlap.bottom > texCoord.y)
		{
			if (blankout.bottom > texCoord.y)
			{
				// leftBottomBlankOut
				return blank;
			}

			// leftBottomBlend
			return texture2DRect(sampler, texCoord) *
				drawSmoothEdge(texCoord.x - blankout.left, overlap.left, blendPower, luminanceControl, gammaCorrection) *
				drawSmoothEdge(texCoord.y - blankout.bottom, overlap.bottom, blendPower, luminanceControl, gammaCorrection);
		}

		if (dimensions.y - blankout.top - overlap.top < texCoord.y)
		{
			if (dimensions.y - blankout.top < texCoord.y)
			{
				// leftTopBlankout
				return blank;
			}

			// leftTopBlend
			return texture2DRect(sampler, texCoord) *
				drawSmoothEdge(texCoord.x - blankout.left, overlap.left, blendPower, luminanceControl, gammaCorrection) *
				drawSmoothEdge(dimensions.y - blankout.top - texCoord.y, overlap.top, blendPower, luminanceControl, gammaCorrection);
		}

		// leftBlankout
		return texture2DRect(sampler, texCoord) *
			drawSmoothEdge(texCoord.x - blankout.left, overlap.left, blendPower, luminanceControl, gammaCorrection);
	}

	if (dimensions.x - blankout.right - overlap.right < texCoord.x)
	{
		if (dimensions.x - blankout.right < texCoord.x)
		{
			// rightBlankout
			return blank;
		}

		if (blankout.bottom + overlap.bottom > texCoord.y)
		{
			if (blankout.bottom > texCoord.y)
			{
				// rightBottomBlankout
				return blank;
			}

			// rightBottomBlend
			return texture2DRect(sampler, texCoord) *
				drawSmoothEdge(dimensions.x - blankout.right - texCoord.x, overlap.right, blendPower, luminanceControl, gammaCorrection) *
				drawSmoothEdge(texCoord.y - blankout.bottom, overlap.bottom, blendPower, luminanceControl, gammaCorrection);
		}

		if (dimensions.y - blankout.top - overlap.top < texCoord.y)
		{
			if (dimensions.y - blankout.top < texCoord.y)
			{
				// rightTopBlankout
				return blank;
			}

			// rightTopBlend
			return texture2DRect(sampler, texCoord) *
				drawSmoothEdge(dimensions.x - blankout.right - texCoord.x, overlap.right, blendPower, luminanceControl, gammaCorrection) *
				drawSmoothEdge(dimensions.y - blankout.top - texCoord.y, overlap.top, blendPower, luminanceControl, gammaCorrection);
		}

		// rightBlend
		return texture2DRect(sampler, texCoord) *
			drawSmoothEdge(dimensions.x - blankout.right - texCoord.x, overlap.right, blendPower, luminanceControl, gammaCorrection);
	}

	if (blankout.bottom + overlap.bottom > texCoord.y)
	{
		if (blankout.bottom > texCoord.y)
		{
			// bottomBlankout
			return blank;
		}

		// bottomBlend
		return texture2DRect(sampler, texCoord) *
			drawSmoothEdge(texCoord.y - blankout.bottom, overlap.bottom, blendPower, luminanceControl, gammaCorrection);
	}

	if (dimensions.y - blankout.top - overlap.top < texCoord.y)
	{
		if (dimensions.y - blankout.top < texCoord.y)
		{
			// topBlankout
			return blank;
		}

		// topBlend
		return texture2DRect(sampler, texCoord) *
			drawSmoothEdge(dimensions.y - blankout.top - texCoord.y, overlap.top, blendPower, luminanceControl, gammaCorrection);
	}

	return texture2DRect(sampler, offsetCoord);
}

vec4 drawSolidEdges(in Edges overlap, in Edges blankout, in vec2 texCoord, in vec2 offset, in vec2 dimensions, in sampler2DRect sampler, vec4 solidEdgeColor)
{
	vec2 offsetCoord = texCoord + offset;
	vec4 blank = vec4(0.0);

	if (blankout.left + overlap.left > texCoord.x)
	{
		if (blankout.left > texCoord.x)
		{
			// leftBlankout
			return blank;
		}

		if (blankout.bottom + overlap.bottom > texCoord.y)
		{
			if (blankout.bottom > texCoord.y)
			{
				// leftBottomBlankout
				return blank;
			}

			// leftBottomColor
			return solidEdgeColor;
		}

		if (dimensions.y - blankout.top - overlap.top < texCoord.y)
		{
			if (dimensions.y - blankout.top < texCoord.y)
			{
				// leftTopBlankout
				return blank;
			}

			// leftTopColor
			return solidEdgeColor;
		}

		// leftColor
		return solidEdgeColor;
	}

	if (dimensions.x - blankout.right - overlap.right < texCoord.x)
	{
		if (dimensions.x - blankout.right < texCoord.x)
		{
			// rightBlankout
			return blank;
		}

		if (blankout.bottom + overlap.bottom > texCoord.y)
		{
			if (blankout.bottom > texCoord.y)
			{
				// rightBottomBlankout
				return blank;
			}

			// rightBottomColor
			return solidEdgeColor;
		}

		if (dimensions.y - blankout.top - overlap.top < texCoord.y)
		{
			if (dimensions.y - blankout.top < texCoord.y)
			{
				// rightTopBlankout
				return blank;
			}

			// rightTopColor
			return solidEdgeColor;
		}

		// rightColor
		return solidEdgeColor;
	}

	if (blankout.bottom + overlap.bottom > texCoord.y)
	{
		if (blankout.bottom > texCoord.y)
		{
			// bottomBlankout
			return blank;
		}

		// bottomColor
		return solidEdgeColor;
	}

	if (dimensions.y - blankout.top - overlap.top < texCoord.y)
	{
		if (dimensions.y - blankout.top < texCoord.y)
		{
			// topBlankout
			return blank;
		}

		// topColor
		return solidEdgeColor;
	}

	return texture2DRect(sampler, offsetCoord);
}

void main()
{
	if (uSolidEdgeEnabled == 1.0)
	{
		gl_FragColor = drawSolidEdges(uOverlap, uBlankout, gl_TexCoord[0].xy, uOffset, uDimensions, uImage, uSolidEdgeColor);
	}
	else
	{
		int idx = int(gl_TexCoord[0].x / uDimensions.x);
		float blendPower = uBlendPower[idx];
		float luminanceControl = uLuminanceControl[idx] + 0.5;
		vec3 gammaCorrection = vec3(uGammaCorrection[idx]);

		gl_FragColor = drawSmoothEdges(uOverlap, uBlankout, gl_TexCoord[0].xy, uOffset, uDimensions, uImage, blendPower, luminanceControl, gammaCorrection);
	}
}
);
