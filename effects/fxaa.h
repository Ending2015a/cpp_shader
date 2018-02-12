#pragma once

#include "../common/core.h"

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     16.0

class FXAA : public Shader {
	// Everyday011 - Rush
	// By David Ronai / @Makio64

	// Check BufferA
	// FXAA from https://github.com/mattdesl/glsl-fxaa



	void texcoords(vec2 fragCoord, vec2 resolution,
		vec2 &v_rgbNW, vec2 &v_rgbNE,
		vec2 &v_rgbSW, vec2 &v_rgbSE,
		vec2 &v_rgbM) {
		vec2 inverseVP = 1.0 / resolution;
		v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
		v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
		v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
		v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
		v_rgbM = vec2(fragCoord * inverseVP);
	}

	vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
		vec2 v_rgbNW, vec2 v_rgbNE,
		vec2 v_rgbSW, vec2 v_rgbSE,
		vec2 v_rgbM) {
		vec4 color;
		vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
		vec3 rgbNW = texture(tex, v_rgbNW).xyz();
		vec3 rgbNE = texture(tex, v_rgbNE).xyz();
		vec3 rgbSW = texture(tex, v_rgbSW).xyz();
		vec3 rgbSE = texture(tex, v_rgbSE).xyz();
		vec4 texColor = texture(tex, v_rgbM);
		vec3 rgbM = texColor.xyz();
		vec3 luma = vec3(0.299, 0.587, 0.114);
		FLOAT lumaNW = dot(rgbNW, luma);
		FLOAT lumaNE = dot(rgbNE, luma);
		FLOAT lumaSW = dot(rgbSW, luma);
		FLOAT lumaSE = dot(rgbSE, luma);
		FLOAT lumaM = dot(rgbM, luma);
		FLOAT lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
		FLOAT lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
		vec2 dir;
		dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
		dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
		FLOAT dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
			(0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
		FLOAT rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
		dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
			max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
				dir * rcpDirMin)) * inverseVP;
		vec3 rgbA = 0.5 * (
			texture(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz() +
			texture(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz());
		vec3 rgbB = rgbA * 0.5 + 0.25 * (
			texture(tex, fragCoord * inverseVP + dir * -0.5).xyz() +
			texture(tex, fragCoord * inverseVP + dir * 0.5).xyz());
		FLOAT lumaB = dot(rgbB, luma);
		if ((lumaB < lumaMin) || (lumaB > lumaMax))
			color = vec4(rgbA, texColor.z);
		else
			color = vec4(rgbB, texColor.z);
		return color;
	}

	void mainImage(vec4 &fragColor, vec2 fragCoord)
	{
		vec2 v_rgbNW;
		vec2 v_rgbNE;
		vec2 v_rgbSW;
		vec2 v_rgbSE;
		vec2 v_rgbM;
		texcoords(fragCoord, iResolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
		fragColor = fxaa(iChannel[0], fragCoord, iResolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
	}
};