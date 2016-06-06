#pragma once

#include "ofMain.h"

#define STRINGIFY(A) #A

namespace ofxProjectorBlendShader
{
	const string GetVertexShader()
	{
		stringstream ss;
		ss << "#version " << (ofIsGLProgrammableRenderer() ? "150" : "120") << endl;

		if (ofIsGLProgrammableRenderer()) {
			ss << STRINGIFY(
				// OF default uniforms and attributes
				uniform mat4 modelViewProjectionMatrix;

				in vec4 position;
				in vec2 texcoord;

				// App uniforms and attributes
				out vec2 vTexCoord;

				//--------------------------------------------------------------
				void main()
				{
					gl_Position = modelViewProjectionMatrix * position;
					vTexCoord = texcoord;
				}
			);
		}
		else {
			ss << STRINGIFY(
				//--------------------------------------------------------------
				void main()
				{
					gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
					gl_TexCoord[0] = gl_MultiTexCoord0;
				}
			);
		}

		return ss.str();
	}

	const string GetFragmentShader()
	{
		stringstream ss;
		ss << "#version " << (ofIsGLProgrammableRenderer() ? "150" : "120") << endl;
		if (!ofIsGLProgrammableRenderer() && ofGetUsingArbTex()) {
			ss << "#extension GL_ARB_texture_rectangle : enable" << endl;
		}
		ss << "#define MAX_BLENDS " << 8 << endl;
		ss << "#define SAMPLER_TYPE " << (ofGetUsingArbTex() ? "sampler2DRect" : "sampler2D") << endl;
		if (ofIsGLProgrammableRenderer()) {
			ss << "#define TEXTURE_FUNCTION texture" << endl;
		}
		else if (ofGetUsingArbTex()) {
			ss << "#define TEXTURE_FUNCTION texture2DRect" << endl;
		}
		else {
			ss << "#define TEXTURE_FUNCTION texture2D" << endl;
		}

		ss << STRINGIFY(
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

			uniform SAMPLER_TYPE uImage;
			uniform vec2 uDimensions;
			uniform vec2 uCanvas;
			uniform vec2 uOffset;

			uniform Edges uOverlap;
			uniform Edges uBlankout;

			uniform float uSolidEdgeEnabled;
			uniform vec4 uSolidEdgeColor;
		);
		ss << endl;

		if (ofIsGLProgrammableRenderer()) {
			ss << "in vec2 vTexCoord;" << endl;
			ss << "out vec4 vFragColor;" << endl;
		}

		ss << STRINGIFY(
			//--------------------------------------------------------------
			vec4 drawSmoothEdge(float location, float extend, in float blendPower, in float luminanceControl, in vec3 gammaCorrection)
			{
				float curve = location / extend;
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

			//--------------------------------------------------------------
			vec4 drawSmoothEdges(in vec2 location, in vec2 texCoord)
			{
				int idx = int(location.x / uDimensions.x);
				float blendPower = uBlendPower[idx];
				float luminanceControl = uLuminanceControl[idx] + 0.5;
				vec3 gammaCorrection = vec3(uGammaCorrection[idx]);

				vec4 blank = vec4(0.0);

				if (uBlankout.left + uOverlap.left > location.x)
				{
					if (uBlankout.left > location.x)
					{
						return blank;  //leftBlankout
					}

					if (uBlankout.bottom + uOverlap.bottom > location.y)
					{
						if (uBlankout.bottom > location.y)
						{
							// leftBottomBlankOut
							return blank;
						}

						// leftBottomBlend
						return TEXTURE_FUNCTION(uImage, texCoord) *
							drawSmoothEdge(location.x - uBlankout.left, uOverlap.left, blendPower, luminanceControl, gammaCorrection) *
							drawSmoothEdge(location.y - uBlankout.bottom, uOverlap.bottom, blendPower, luminanceControl, gammaCorrection);
					}

					if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
					{
						if (uDimensions.y - uBlankout.top < location.y)
						{
							// leftTopBlankout
							return blank;
						}

						// leftTopBlend
						return TEXTURE_FUNCTION(uImage, texCoord) *
							drawSmoothEdge(location.x - uBlankout.left, uOverlap.left, blendPower, luminanceControl, gammaCorrection) *
							drawSmoothEdge(uDimensions.y - uBlankout.top - location.y, uOverlap.top, blendPower, luminanceControl, gammaCorrection);
					}

					// leftBlankout
					return TEXTURE_FUNCTION(uImage, texCoord) *
						drawSmoothEdge(location.x - uBlankout.left, uOverlap.left, blendPower, luminanceControl, gammaCorrection);
				}

				if (uDimensions.x - uBlankout.right - uOverlap.right < location.x)
				{
					if (uDimensions.x - uBlankout.right < location.x)
					{
						// rightBlankout
						return blank;
					}

					if (uBlankout.bottom + uOverlap.bottom > location.y)
					{
						if (uBlankout.bottom > location.y)
						{
							// rightBottomBlankout
							return blank;
						}

						// rightBottomBlend
						return TEXTURE_FUNCTION(uImage, texCoord) *
							drawSmoothEdge(uDimensions.x - uBlankout.right - location.x, uOverlap.right, blendPower, luminanceControl, gammaCorrection) *
							drawSmoothEdge(location.y - uBlankout.bottom, uOverlap.bottom, blendPower, luminanceControl, gammaCorrection);
					}

					if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
					{
						if (uDimensions.y - uBlankout.top < location.y)
						{
							// rightTopBlankout
							return blank;
						}

						// rightTopBlend
						return TEXTURE_FUNCTION(uImage, texCoord) *
							drawSmoothEdge(uDimensions.x - uBlankout.right - location.x, uOverlap.right, blendPower, luminanceControl, gammaCorrection) *
							drawSmoothEdge(uDimensions.y - uBlankout.top - location.y, uOverlap.top, blendPower, luminanceControl, gammaCorrection);
					}

					// rightBlend
					return TEXTURE_FUNCTION(uImage, texCoord) *
						drawSmoothEdge(uDimensions.x - uBlankout.right - location.x, uOverlap.right, blendPower, luminanceControl, gammaCorrection);
				}

				if (uBlankout.bottom + uOverlap.bottom > location.y)
				{
					if (uBlankout.bottom > location.y)
					{
						// bottomBlankout
						return blank;
					}

					// bottomBlend
					return TEXTURE_FUNCTION(uImage, texCoord) *
						drawSmoothEdge(location.y - uBlankout.bottom, uOverlap.bottom, blendPower, luminanceControl, gammaCorrection);
				}

				if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
				{
					if (uDimensions.y - uBlankout.top < location.y)
					{
						// topBlankout
						return blank;
					}

					// topBlend
					return TEXTURE_FUNCTION(uImage, texCoord) *
						drawSmoothEdge(uDimensions.y - uBlankout.top - location.y, uOverlap.top, blendPower, luminanceControl, gammaCorrection);
				}

				return TEXTURE_FUNCTION(uImage, texCoord);
			}

			//--------------------------------------------------------------
			vec4 drawSolidEdges(in vec2 location, in vec2 texCoord)
			{
				vec4 blank = vec4(0.0);

				if (uBlankout.left + uOverlap.left > location.x)
				{
					if (uBlankout.left > location.x)
					{
						// leftBlankout
						return blank;
					}

					if (uBlankout.bottom + uOverlap.bottom > location.y)
					{
						if (uBlankout.bottom > location.y)
						{
							// leftBottomBlankout
							return blank;
						}

						// leftBottomColor
						return uSolidEdgeColor;
					}

					if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
					{
						if (uDimensions.y - uBlankout.top < location.y)
						{
							// leftTopBlankout
							return blank;
						}

						// leftTopColor
						return uSolidEdgeColor;
					}

					// leftColor
					return uSolidEdgeColor;
				}

				if (uDimensions.x - uBlankout.right - uOverlap.right < location.x)
				{
					if (uDimensions.x - uBlankout.right < location.x)
					{
						// rightBlankout
						return blank;
					}

					if (uBlankout.bottom + uOverlap.bottom > location.y)
					{
						if (uBlankout.bottom > location.y)
						{
							// rightBottomBlankout
							return blank;
						}

						// rightBottomColor
						return uSolidEdgeColor;
					}

					if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
					{
						if (uDimensions.y - uBlankout.top < location.y)
						{
							// rightTopBlankout
							return blank;
						}

						// rightTopColor
						return uSolidEdgeColor;
					}

					// rightColor
					return uSolidEdgeColor;
				}

				if (uBlankout.bottom + uOverlap.bottom > location.y)
				{
					if (uBlankout.bottom > location.y)
					{
						// bottomBlankout
						return blank;
					}

					// bottomColor
					return uSolidEdgeColor;
				}

				if (uDimensions.y - uBlankout.top - uOverlap.top < location.y)
				{
					if (uDimensions.y - uBlankout.top < location.y)
					{
						// topBlankout
						return blank;
					}

					// topColor
					return uSolidEdgeColor;
				}

				return TEXTURE_FUNCTION(uImage, texCoord);
			}
		);
		ss << endl;

		if (ofIsGLProgrammableRenderer()) {
			if (ofGetUsingArbTex()) {
				ss << STRINGIFY(
					//--------------------------------------------------------------
					void main()
					{
						vec2 texCoord = vTexCoord + uOffset;

						if (uSolidEdgeEnabled == 1.0)
						{
							vFragColor = drawSolidEdges(vTexCoord, texCoord);
						}
						else
						{
							vFragColor = drawSmoothEdges(vTexCoord, texCoord);
						}
					}
				);
			}
			else {
				ss << STRINGIFY(
					//--------------------------------------------------------------
					void main()
					{
						vec2 location = vTexCoord * uCanvas;
						vec2 texCoord = vTexCoord + uOffset / uCanvas;

						if (uSolidEdgeEnabled == 1.0)
						{
							vFragColor = drawSolidEdges(location, texCoord);
						}
						else
						{
							vFragColor = drawSmoothEdges(location, texCoord);
						}
					}
				);
			}
		}
		else {
			if (ofGetUsingArbTex()) {
				ss << STRINGIFY(
					//--------------------------------------------------------------
					void main()
					{
						vec2 texCoord = gl_TexCoord[0].xy + uOffset;

						if (uSolidEdgeEnabled == 1.0)
						{
							gl_FragData[0] = drawSolidEdges(gl_TexCoord[0].xy, texCoord);
						}
						else
						{
							gl_FragData[0] = drawSmoothEdges(gl_TexCoord[0].xy, texCoord);
						}
					}
				);
			}
			else {
				ss << STRINGIFY(
					//--------------------------------------------------------------
					void main()
					{
						vec2 location = gl_TexCoord[0].xy * uCanvas;
						vec2 texCoord = gl_TexCoord[0].xy + uOffset / uCanvas;

						if (uSolidEdgeEnabled == 1.0)
						{
							gl_FragData[0] = drawSolidEdges(location, texCoord);
						}
						else
						{
							gl_FragData[0] = drawSmoothEdges(location, texCoord);
						}
					}
				);
			}
		}

		return ss.str();
	}
}
