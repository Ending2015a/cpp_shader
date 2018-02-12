#pragma once
#include <complex>
#include "../common/core.h"

#include <glm/common.hpp>

class MandelbulbIQ : public Shader {
public:
	// Created by inigo quilez - iq/2013
	// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.



	FLOAT scale;
	vec2 center;
	vec2 eps;

	MandelbulbIQ() : scale(1), center(0, 0), eps(0.1, 0) {}

	vec2 isphere(vec4 &sph, vec3 &ro, vec3 &rd) {
		vec3 oc = ro - sph.xyz();

		FLOAT b = dot(oc, rd);
		FLOAT c = dot(oc, oc) - sph.w*sph.w;
		FLOAT h = b*b - c;

		if (h < 0.0)return vec2(-1.0);

		h = sqrt(h);
		return -b + vec2(-h, h);
	}

	FLOAT map(vec3 &p, vec4 &resColor) {
		vec3 w = p;
		FLOAT m = dot(w, w);
		vec4 trap = vec4(abs(w), m);
		FLOAT dz = 1.0;

		for (int i = 0; i < 4; ++i) {
			dz = 8.0*pow(sqrt(m), 7.0)*dz + 1.0;
			FLOAT r = length(w);
			FLOAT b = 8.0 * acos(w.y / r);
			FLOAT a = 8.0 * atan2(w.x, w.z);

			w = p + pow(r, 8.0) * vec3(sin(b)*sin(a), cos(b), sin(b)*cos(a));
			trap = min(trap, vec4(abs(w), m));

			m = dot(w, w);
			if (m > 256.0)
				break;
		}

		resColor = vec4(m, trap.yzw());

		return 0.25 * log(m) * sqrt(m) / dz;
	}
	
	FLOAT intersect(vec3 &ro, vec3 &rd, vec4 &rescol, FLOAT &px) {
		FLOAT res = -1.0;

		//bounding sphere
		vec2 dis = isphere(vec4(0., 0., 0., 1.25), ro, rd);
		if (dis.y < 0.0)
			return -1.0;
		dis.x = max(dis.x, 0.);
		dis.y = min(dis.y, 10.0);

		vec4 trap;

		FLOAT t = dis.x;
		for (int i = 0; i < 128; ++i) {
			vec3 pos = ro + rd*t;
			FLOAT th = 0.25 * px * t;
			FLOAT h = map(pos, trap);
			if (t > dis.y || h < th)break;
			t += h;
		}

		if (t < dis.y) {
			rescol = trap;
			res = t;
		}

		return res;
	}

	FLOAT softshadow(vec3 &ro, vec3 &rd, FLOAT k) {
		FLOAT res = 1.0;
		FLOAT t = 0.0;
		for (int i = 0; i < 64; ++i) {
			vec4 kk;
			FLOAT h = map(ro + rd*t, kk);
			res = min(res, k*h / t);
			if (res < 0.001)break;
			t += glm::clamp(h, 0.01, 0.2);
		}
		return glm::clamp(res, 0.0, 1.0);
	}
	
	vec3 calcNormal(vec3 &pos, FLOAT &t, FLOAT &px) {
		vec4 tmp;
		vec2 eps = vec2(0.25*px, 0.0);
		return normalize(vec3(
			map(pos + eps.xyy(), tmp) - map(pos - eps.xyy(), tmp),
			map(pos + eps.yxy(), tmp) - map(pos - eps.yxy(), tmp),
			map(pos + eps.yyx(), tmp) - map(pos - eps.yyx(), tmp)
		));
	}

	vec3 render(vec2 p, mat4 cam) {
		//light
		vec3 light1 = vec3(0.577, 0.577, -0.577);
		vec3 light2 = vec3(-0.707, 0.000, 0.707);

		//ray setup
		const FLOAT fle = 1.5;

		vec2 sp = (-iResolution.xy() + 2.0*p) / iResolution.y;
		FLOAT px = 2.0 / (iResolution.y * fle);

		vec3 ro = vec3(cam[0].w, cam[1].w, cam[2].w);
		vec3 rd = normalize((cam*vec4(sp, fle, 0.0)).xyz());

		//intersect fractal
		vec4 tra;
		FLOAT t = intersect(ro, rd, tra, px);

		vec3 col;
		if (t < 0.0) {
			col = vec3(0.8, 0.95, 1.0) * (0.6 + 0.4*rd.y);
			col += 5.0*vec3(0.8, 0.7, 0.5)*pow(glm::clamp(dot(rd, light1), 0.0, 1.0), 32.0);
		}
		else {
			col = vec3(0.01);
			col = glm::mix(col, vec3(0.1, 0.2, 0.3), glm::clamp(tra.y, 0.0, 1.0));
			col = glm::mix(col, vec3(0.02, 0.1, 0.3), glm::clamp(tra.z*tra.z, 0.0, 1.0));
			col = glm::mix(col, vec3(0.3, 0.1, 0.02), glm::clamp(glm::pow(tra.w, 6.0), 0.0, 1.0));
			col *= 0.5;

			//lighting terms
			vec3 pos = ro + t*rd;
			vec3 nor = calcNormal(pos, t, px);
			vec3 hal = normalize(light1 - rd);
			vec3 ref = glm::reflect(rd, nor);
			FLOAT occ = glm::clamp(0.05*log(tra.x), 0.0, 1.0);
			FLOAT fac = glm::clamp(1.0 + dot(rd, nor), 0.0, 1.0);

			//sun
			FLOAT sha1 = softshadow(pos + 0.001*nor, light1, 32.0);
			FLOAT dif1 = glm::clamp(dot(light1, nor), 0.0, 1.0)*sha1;
			FLOAT spe1 = pow(glm::clamp(dot(nor, hal), 0.0, 1.0), 32.0)*dif1*(0.04 + 0.96*pow(glm::clamp(1.0 - dot(hal, light1), 0.0, 1.0), 5.0));
			//bounce
			FLOAT dif2 = glm::clamp(0.5 + 0.5*dot(light2, nor), 0.0, 1.0)*occ;
			//sky
			FLOAT dif3 = (0.7 + 0.3*nor.y)*(0.2 + 0.8*occ);

			vec3 lin = vec3(0.0);
			lin += 7.0*vec3(1.5, 1.1, 0.7)*dif1;
			lin += 4.0*vec3(0.25, 0.2, 0.15)*dif2;
			lin += 1.5*vec3(0.1, 0.2, 0.3)*dif3;
			lin += 2.5*vec3(0.35, 0.3, 0.25)*(0.05 + 0.95*occ);
			lin += 4.0*fac*occ;

			col *= lin;
			col = pow(col, vec3(0.7, 0.9, 1.0));
			col += spe1*15.0;
		}

		return sqrt(col);
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) {

		FLOAT time = iTime*.1;

		//camera
		FLOAT di = 1.4 + 0.1 * cos(.29*time);
		vec3 ro = di * vec3(cos(.33*time), 0.8*sin(.37*time), sin(.31*time));
		vec3 ta = vec3(0.0, 0.1, 0.0);
		FLOAT cr = 0.5*cos(0.1*time);

		//camera matrix
		vec3 cw = glm::normalize(ta - ro);
		vec3 cp = vec3(sin(cr), cos(cr), 0.0);
		vec3 cu = normalize(cross(cw, cp));
		vec3 cv = normalize(cross(cu, cw));
		
		mat4 cam = mat4(cu.x, cu.y, cu.z, ro.x, 
						cv.x, cv.y, cv.z, ro.y, 
						cw.x, cw.y, cw.z, ro.z, 
						0.0, 0.0, 0.0, 1.0);

#ifdef AA
		vec3 col = vec3(0.);
		for (int i = 0; i < AA; ++i) {
			for (int j = 0; j < AA; ++j) {
				col += render(fragCoord + (vec2(i, j) / FLOAT(AA)), cam);
			}
		}
		col /= FLOAT(AA*AA);
#else
		vec3 col = render(fragCoord, cam);
#endif
		fragColor = vec4(col, 1.0);
	}
};

class MandelbulbV3D : public Shader {
public:
	FLOAT scale;
	vec2 center;

	FLOAT pw;  //power
	FLOAT maxiter;  //iteration limit
	FLOAT rayiter; //ray marching iteration limit
	FLOAT sdwiter;
	FLOAT steplimiter;
	FLOAT raymultiplyer; //ray marching step multiplyer
	FLOAT bailout;  //bailout
	FLOAT eps;

	MandelbulbV3D() {
		pw = 8.0;  //mandelbulb power
		maxiter = 24;  //mandelbulb iteration limit
		rayiter = 10000;  //ray marching iteration limit
		sdwiter = 1024; //softshadow iteration limit
		steplimiter = 0.2; //softshadow max step limiter
		raymultiplyer = 0.1;  //ray marching step multiplyer
		bailout = 2.0; //bailout
		eps = 1.0;  //epsilon
	}

	FLOAT md(vec3 p, FLOAT &trap) {
		vec3 z = p;
		FLOAT dr = 1.0;
		FLOAT r = length(z);
		trap = r;

		for (int i = 0; i<maxiter; ++i) {
			FLOAT th = atan2(z.y, z.x) *pw;
			FLOAT phi = asin(z.z / r) *pw;
			dr = pw*pow(r, pw - 1.)*dr + 1.;
			z = p + pow(r, pw) * vec3(cos(th)*cos(phi), cos(phi)*sin(th), -sin(phi));

			//orbit trap
			trap = min(trap, r);

			r = length(z);
			if (r > bailout)break;
		}
		return 0.5 * log(r) * r / dr;
	}

	FLOAT sdBox(vec3 p, vec3 b) {
		vec3 d = abs(p) - b;
		return min(max(d.x, max(d.y, d.z)), 0.) + length(max(d, 0.));
	}

	FLOAT map(vec3 p, FLOAT &trap, int &ID)
	{
		vec2 rt = vec2(cos(pi / 2.), sin(pi / 2.));
		vec3 rp = mat3(1., 0., 0., 0., rt.x, -rt.y, 0., rt.y, rt.x) *p;

		FLOAT d1 = md(rp, trap);
#ifdef CUT_Y
		FLOAT d2 = sdBox(p - vec3(0., 2., 0.), vec3(2.));
		d1 = max(d1, -d2);
#endif
		ID = 1;

		return d1;
	}

	FLOAT map(vec3 p) {
		FLOAT dmy; //dummy
		int dmy2; //dummy2
		return map(p, dmy, dmy2);
	}

	vec3 pal(FLOAT t, vec3 a, vec3 b, vec3 c, vec3 d) {
		return a + b*cos(2.*pi*(c*t + d));
	}

	FLOAT softshadow(vec3 ro, vec3 rd, FLOAT k) {
		FLOAT res = 1.0;
		FLOAT t = 0.0;
		for (int i = 0; i < sdwiter; ++i) {
			FLOAT h = map(ro + rd*t);
			res = min(res, k*h / t);
			if (res < 0.02)
				return 0.02;
			t += glm::clamp(h, 0.001, steplimiter);
		}
		return glm::clamp(res, 0.02, 1.0);
	}

	vec3 calcNor(vec3 p) {
		vec2 e = vec2(eps / iResolution.x, 0.);
		return normalize(vec3(map(p + e.xyy()) - map(p - e.xyy()),  //dx
			map(p + e.yxy()) - map(p - e.yxy()),  //dy
			map(p + e.yyx()) - map(p - e.yyx())   //dz
		));
	}

	FLOAT trace(vec3 ro, vec3 rd, FLOAT &trap, int &ID)
	{
		FLOAT t = 0;
		FLOAT len = 0;

		for (int i = 0; i < rayiter; ++i) {
			len = map(ro + rd * t, trap, ID);
			if (abs(len) < eps / iResolution.x || t > 40.0)
				break;
			t += len*raymultiplyer;
		}
		return t < 40.0 ? t : -1.0;
	}

	vec3 s2c(vec3 cp) {
		return cp.x * vec3(cos(cp.y)*cos(cp.z), sin(cp.z), sin(cp.y)*cos(cp.z));
	}

	vec3 render(vec2 p, vec3 cpos)
	{
		//screen coord
		vec2 uv = (-iResolution.xy() + 2.0 * p) / iResolution.y;  //(-x/y ~ x/y, -1 ~ 1)

																  // set camera
		vec3 ro = s2c(cpos); //ray (camera) origin (spherical 2 cartesin)
		vec3 ta = vec3(0.); //camera target
		vec3 cf = normalize(ta - ro); //camera forward vec
		vec3 cs = normalize(cross(cf, vec3(0.0, 1.0, 0.0)));  //camera right(side) vec = forward x y-ax
		vec3 cu = normalize(cross(cs, cf));  //camera up vec = side x forward
		vec3 rd = normalize(uv.x*cs + uv.y*cu + 1.5*cf); //ray direction

														 // marching
		FLOAT trap;  //orbit trap
		int objID;
		FLOAT d = trace(ro, rd, trap, objID);

		// lighting
		vec3 col;
		vec3 sd = normalize(s2c(cpos + vec3(0., sin(iTime), 0.)));  //light direcion
		vec3 sc = vec3(1.0, 0.9, 0.717); //light color

										 // sky
		if (d < 0.0)
			col = vec3(0.0);  //sky color
		else {
			// coloring
			vec3 pos = ro + rd * d;  //hit position
			vec3 nr = calcNor(pos);  //approx normal
			vec3 hal = normalize(sd - rd);  // blinn-phong h_hat

			col = pal(trap - 0.4, vec3(0.5), vec3(0.5), vec3(1.0), vec3(0.0, 0.10, 0.20)); //diffuse color
			vec3 ambc = vec3(0.3); //ambient color
			FLOAT gloss = 32.0;  //specular gloss

								 // simple blinn-phong lighting model
								 // plinn-phong = ambient + diffuse + specular
								 //   ambient = ambient color (here wa use approx self occlution)
								 //   diffuse = light color * diffuse color * dot(nr, sd) * intensity
								 //   specular = light color * specular color * pow(dot(nr, hal), gloss)

			FLOAT amb = (0.7 + 0.3*nr.y)*(0.2 + 0.8 * glm::clamp(0.05*log(trap), 0.0, 1.0));  //self occlution approx
			FLOAT sdw = softshadow(pos + 0.001*nr, sd, 16.0);  //shadow
			FLOAT dif = glm::clamp(dot(sd, nr), 0.0, 1.0) * sdw;  //diffuse
			FLOAT spe = pow(glm::clamp(dot(nr, hal), 0., 1.), gloss) * dif; // * self shadow


			vec3 lin(0.0);
			lin += ambc * (0.05 + 0.95*amb); //ambient color * approx ambient
			lin += sc * dif * 0.8; //diffuse * light color * light intenity
			col *= lin;

			col = pow(col, vec3(0.7, 0.9, 1.0)); // subsurface scattering
			col += spe*0.8; //specular
		}

		col = glm::clamp(pow(col, vec3(0.4545)), 0.0, 1.0); //gamma correction

		return col;
	}

	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) override {

		//calc cam pos (Spherical coord r, theta, phi)
		vec3 cpos = vec3(2.*pow(0.9, sin(iTime / 4.)*2.5 + 2.5), -iTime / 3., sin(iTime / 3.)*pi / 3.);

		vec4 col(0.0);
#ifdef AA
		for (int m = 0; m < AA; ++m) {
			for (int n = 0; n < AA; ++n) {
				vec2 p = vec2(m / (FLOAT)AA, n / (FLOAT)AA);
				vec2 px = fragCoord + p;
				col += vec4(render(px, cpos), 1.0);
			}
		}
		col /= (FLOAT)AA*AA;
#else
		col = vec4(render(fragCoord, cpos), 1.0);
#endif
		fragColor = col;
	}
};