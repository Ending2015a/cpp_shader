#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include "type.h"
#include "buffer.h"

#include "debug.h"

#ifdef OPENMP_SUPPORT
#include <omp.h>
	#ifndef NUM_THREAD
		#define NUM_THREAD 8
	#endif
#endif

class Shader {
private:
	int ID;
	std::vector<int> lastID;
	static int TotalShader;

protected:
	vec2 iResolution;
	FLOAT iTime;
	std::string name;
	Buffer oChannel;
	std::vector<Buffer> iChannel;

public:

	Shader() : iResolution(0), iTime(0), name("NoName"), lastID() { ID = Shader::TotalShader++; }
	Shader(std::string name) : iResolution(0), iTime(0), name(name), lastID() { ID = Shader::TotalShader++; }

	virtual int getID() final { return ID; }

	virtual void setTime(FLOAT time) final { iTime = time; }
	virtual FLOAT getTime() final { return iTime; }

	virtual void setResolution(vec2i res) final { iResolution = vec2(res.x, res.y); }
	virtual vec2i getResolution() final { return vec2i(iResolution.x, iResolution.y); }

	virtual void setName(std::string name) final { this->name = name; }
	virtual std::string getName() final { return name; }

	virtual Buffer &getBuffer() final { return oChannel; }
	virtual void setBuffer(Buffer &buf) final { oChannel = buf; }

	virtual std::vector<int> &getParents() { return lastID; }

	//connect rendering path
	virtual Shader &operator<<(Shader &shader) final {
		lastID.push_back(shader.ID);
		iChannel.push_back(shader.oChannel);
		return shader;
	}

	//initialize shader & buffer
	virtual void init(vec2i res) {
		setResolution(res);
		oChannel.check(res);
		oChannel.init();
	}

	//main shader program
	virtual void mainImage(vec4 &fragColor, vec2 fragCoord) {
		// Normalized pixel coordinates (from 0 to 1)
		vec2 uv = fragCoord / iResolution;

		// Time varying pixel color
		vec3 col = 0.5 + 0.5*cos(iTime + vec3(uv.x, uv.y, uv.x) + vec3(0, 2, 4));

		// Output to screen
		fragColor = vec4(col, 1.0);
	}

	//render
	virtual void render() final {
		TIC("render");

		std::cout << "shader " << name << ": rendering..." << std::endl;
		std::cout << "resolution: " << oChannel.getResolution().x << ", " << oChannel.getResolution().y << std::endl;
		Texture tmp_buf(oChannel.getResolution());
		FLOAT total = iResolution.x * iResolution.y;
		int pixel = 0;

#ifdef OPENMP_SUPPORT
	#pragma omp parallel for num_threads(NUM_THREAD) schedule(dynamic) shared(tmp_buf, pixel, total)
#endif
		for (int i = 0; i < tmp_buf.end - tmp_buf.raw_data; ++i) {
			vec2 cd = vec2(i % tmp_buf.resolution.x, i / tmp_buf.resolution.x);
			mainImage(tmp_buf.raw_data[i], cd);
#ifdef OPENMP_SUPPORT
			#pragma omp atomic
#endif
			pixel += 1;
#ifdef OPENMP_SUPPORT
			if(omp_get_thread_num() == 0)
#endif
				if(pixel % 100 == 0)
					printf("Rendering... %5.2lf %%\r", pixel / total*100.);
		}
		printf("Rendering... Done   \n");

		//copy to output buffer
		oChannel.copyFrom(tmp_buf);
		TOC("render");
		_LOG_ALL();
	}
};

int Shader::TotalShader = 0;