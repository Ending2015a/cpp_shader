#pragma once

#include "../common/core.h"

#include <complex>

#define GPhi 1.61803398874989484820

class JuliaVd : public Shader {
public:

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = 1;
		vec2 ce(-0.5, 0);
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		FLOAT maxIter = 1000;
		FLOAT maxThres = 5;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> c(cx, cy);
		std::complex<FLOAT> z(0, 0);

		FLOAT iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = z*z*z + (c - 1.)*z - c;
			++iter;
		}

		iter = iter - log2(log(std::abs(z)) / log(maxThres));
		return vec3(iter);
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) {

		vec4 col(0.0);
		col = vec4(render(fragCoord, iTime, iResolution), 1.0);
		fragColor = col;
	}
};

class JuliaV0 : public Shader {
public:

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = 1;
		vec2 ce(-0.5, 0);
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		FLOAT maxIter = 1000;
		FLOAT maxThres = 5.;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> c(cx, cy);
		std::complex<FLOAT> z(0, 0);

		FLOAT iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = z*z*z + (c-1.)*z - c;
			iter += 1.0;
		}

		if (iter < maxIter) {
			//iter = iter - log2(log(std::abs(z)) / log(maxThres));

			vec3 col = vec3(fabs(cos(iter / 5.0)),
				fabs(sin(iter / 5.0 + pi / 4.0)),
				fabs(sin(iter / 5.0)));
			return col; // vec3(fabs(cos(dzlog)));
		}
		else {
			return vec3(0, 0, 0);
		}
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) {

		vec4 col(0.0);
#ifdef AA
		for (int m = 0; m < AA; ++m) {
			for (int n = 0; n < AA; ++n) {
				vec2 p = vec2(m / (FLOAT)AA, n / (FLOAT)AA);
				vec2 px = fragCoord + p;
				col += vec4(render(px, iTime, iResolution), 1.0);
			}
		}
		col /= (FLOAT)AA*AA;
#else
		col = vec4(render(fragCoord, iTime, iResolution), 1.0);
#endif
		fragColor = col;
	}
};