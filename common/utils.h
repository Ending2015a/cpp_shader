#pragma once

#include "buffer.h"
#include "type.h"

#ifndef pi 
#define pi 3.1415926535897932384626433832795
#endif

//max
template<typename T>
T max(T a, T b) {
	return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
	return a < b ? a : b;
}

//texture
vec4 texture(sampler2D &tex, vec2 crd) {
	crd = clamp(crd, 0.0, 1.0);
	vec2 p = crd * vec2(tex.getResolution().x - 1.0, tex.getResolution().y - 1.0);
	vec2 l = floor(p);
	vec2 u = ceil(p);
	vec2 f = p - l;
	vec4 cA = mix(tex.pixel(vec2i(l.x, l.y)), tex.pixel(vec2i(u.x, l.y)), f.x);
	vec4 cB = mix(tex.pixel(vec2i(l.x, u.y)), tex.pixel(vec2i(u.x, u.y)), f.x);
	return mix(cA, cB, f.y);
}