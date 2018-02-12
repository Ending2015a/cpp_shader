#pragma once

#include "../common/core.h"
#include "../effects/fxaa.h"

#define AA 2

class CollatzVd : public Shader {
public:
	FLOAT scale;
	vec2 center;

	CollatzVd() : scale(0.03), center(-0.703, 0) {};

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		int maxIter = 1000;
		FLOAT maxThres = 1000000000;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> z(cx, cy);

		int iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = (2.0 + z * 7.0 - (2.0 + z * 5.0)*std::cos(pi*z)) / 4.0;
			++iter;
		}

		return vec3(iter);
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) {
		vec4 col(0.0);
		col = vec4(render(fragCoord, iTime, iResolution), 1.0);
		fragColor = col;
	}
};


class CollatzV0 : public Shader {
public:
	FLOAT scale;
	vec2 center;

	CollatzV0() : scale(0.03), center(-0.703, 0) {};

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		int maxIter = 1000;
		FLOAT maxThres = 100;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> z(cx, cy);

		int iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = (2.0 + z * 7.0 - (2.0 + z * 5.0)*std::cos(pi*z)) / 4.0;
			++iter;
		}

		if (iter < maxIter) {

			vec3 col = vec3(sqrt(log(iter+1.)/log(maxIter)));
			return col;
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


class CollatzV1 : public Shader {
public:
	FLOAT scale;
	vec2 center;

	CollatzV1() : scale(0.03), center(-0.703, 0) {};

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		int maxIter = 1000;
		FLOAT maxThres = 1000000000;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> z(cx, cy);

		int iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			z = (2.0 + z * 7.0 - (2.0 + z * 5.0)*std::cos(pi*z)) / 4.0;
			++iter;
		}

		if (iter < maxIter) {

			vec3 col = vec3(fabs(cos(iter / 5.0)),
				fabs(sin(iter / 5.0 + pi / 4.0)),
				fabs(sin(iter / 5.0)));
			return col;
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

class CollatzV2 : public Shader {
public:
	FLOAT scale;
	vec2 center;

	CollatzV2() : scale(0.03), center(-0.703, 0) {};

	vec3 render(vec2 fragCoordScreen, FLOAT time, vec2 iResolution) {

		FLOAT sc = scale;
		vec2 ce = center;
		FLOAT zoom = time * 2;
		sc = sc * pow(0.9, zoom);

		int maxIter = 1000;
		FLOAT maxThres = 100;

		FLOAT cx = ce.x + sc * (-iResolution.x + 2.0 * fragCoordScreen.x) / iResolution.y;
		FLOAT cy = ce.y + sc * (-iResolution.y + 2.0 * fragCoordScreen.y) / iResolution.y;
		std::complex<FLOAT> z(cx, cy);
		std::complex<FLOAT> dz(1, 0);
		std::complex<FLOAT> lz = z;
		std::complex<FLOAT> ldz = dz;

		int iter = 0;
		while (iter < maxIter && std::abs(z) < maxThres) {
			lz = z;
			ldz = dz;
			dz = ((7.0 + pi*std::sin(pi*z)*(2.0 + z*5.0) - 5.0*std::cos(pi*z)) / 4.0);
			z = (2.0 + z * 7.0 - (2.0 + z * 5.0)*std::cos(pi*z)) / 4.0;
			++iter;
		}

		if (iter < maxIter) {

			FLOAT dzlog = log(std::abs(ldz));
			FLOAT aslog = fabs(sin(dzlog / 5.0));
			FLOAT inner = clamp((aslog - 0.2), 0.0, 0.5) * 2;
			FLOAT outer = 1.0 - smoothstep(0.5, 0.8, aslog - 0.2);
			FLOAT grad = sin(smoothstep(0.3, 0.7, aslog - 0.2)*pi);

			FLOAT esc = log(iter + 1) / log(maxIter);

			vec3 clr1 = vec3(fabs(cos(iter / 5.0)),
				fabs(sin(iter / 5.0 + pi / 4.0)),
				fabs(sin(iter / 5.0)));
			return clamp(clr1 + grad*clr1*0.3, 0.0, 1.0) * inner *outer;
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

class Collatz : public RenderProgram {
public:
	Program program;
	CollatzV1 shader;
	vec2i Resolution;

	Collatz(vec2i res) { Resolution = res; }
	
	virtual void build() {
		shader.setName("collatz base");
		program.addShader(shader);

		program << shader;

		program.build(Resolution);
	}

	virtual Texture &render(FLOAT iTime) {
		return program.render(iTime);
	}
#ifdef OPENCV_SUPPORT
	void render(cv::Mat &frame, FLOAT iTime) {
		return render(iTime).to8U_BGR(frame);
	}
#endif
};

class CollatzDebug : public RenderProgram {
public:
	Program program;
	CollatzVd shader;
	vec2i Resolution;

	CollatzDebug(vec2i res) { Resolution = res; }

	virtual void build() {
		shader.setName("collatz base");
		program.addShader(shader);

		program << shader;

		program.build(Resolution);
	}

	virtual Texture &render(FLOAT iTime) {
		return program.render(iTime);
	}
#ifdef OPENCV_SUPPORT
	void getIter(cv::Mat &frame, FLOAT iTime) {
		return render(iTime).to64F_BGRA(frame);
	}
	void render(cv::Mat &frame, FLOAT iTime) {
		return render(iTime).to8U_BGR(frame);
	}
#endif
};