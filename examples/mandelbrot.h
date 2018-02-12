#pragma once

#include "../common/core.h"

class MandelbrotVd : public Shader {
public:

	FLOAT scale;
	vec2 center;

	MandelbrotVd() : scale(1), center(vec2(-0.5, 0)) {}

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		FLOAT maxIter = 1000;
		FLOAT maxThres = 5;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> c(cx, cy);
		std::complex<FLOAT> z(0, 0);

		vec4 dst = vec4(10000);

		FLOAT iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = z*z + c;
			++iter;
		}

		return vec3(iter);
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) override {

		vec4 col(0.0);
		col = vec4(render(fragCoord, iTime, iResolution), 1.0);
		fragColor = col;
	}
};

class MandelbrotV0 : public Shader {
public:
	FLOAT scale;
	vec2 center;

	MandelbrotV0() : scale(1), center(vec2(-0.5, 0)){}

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		FLOAT maxIter = 1000;
		FLOAT maxThres = 5.;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> c(cx, cy);
		std::complex<FLOAT> z(0, 0);

		vec4 dst = vec4(10000);

		FLOAT iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = z*z + c;
			iter += 1.0;
			dst = min(dst, vec4(cos(z.imag()) + sin(z.real()),
								std::abs(1.0 + z.real() + 0.5*sin(z.imag())),
								z.real()*z.real()+z.imag()*z.imag(),
								length(fract(vec2(z.real(), z.imag())) - 0.5 )));
		}

		if (iter < maxIter) {
			//vec3 color = vec3(dst.w);
			vec3 color = mix(vec3(0.), vec3(1.00, 0.80, 0.60), min(1.0, pow(dst.x*0.25, 0.20)));
			//color = mix(color, vec3(0.72, 0.70, 0.60), min(1.0, pow(dst.y*0.50, 0.50)));
			//color = mix(color, vec3(1.00, 1.00, 1.00), 1.0 - min(1.0, pow(dst.z*1.00, 0.15)));

			//color = 1.25*color*color;
			iter = iter + log2(log(maxThres) / log(std::abs(z)));
			//iter = iter - log2(log(std::abs(z))/log(maxThres));

			vec3 col = vec3(fabs(cos(iter / 5.0)),
				fabs(sin(iter / 5.0 + pi / 4.0)),
				fabs(sin(iter / 5.0)));
			return col;
		}
		else {
			return vec3(0, 0, 0);
		}
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) override {

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