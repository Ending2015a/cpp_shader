#pragma once

#ifndef FLOAT
typedef double FLOAT;
#endif

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <utility>
#include <algorithm>

#include "type.h"

#include "buffer.h"
#include "Shader.h"
#include "utils.h"

class Program {

private:
	// [ID, shader] mapping
	std::map<int, Shader *> shaders;
	// rendering order
	std::vector<int> order;
	// store ID of the main shader 
	int main_shader_id;
	// manage buffers
	BufferManager bufferManager;

	void DFS(int node, std::map<int, vec3i> &eTime, int &time) {
		eTime[node].z = 1;
		++time;
		eTime[node].x = time;
		Shader *s = shaders[node];
		for (auto it = s->getParents().begin(); it != s->getParents().end(); ++it) {
			if (eTime[*it].z == 0) {
				DFS(*it, eTime, time);
			}
		}
		eTime[node].z = 2;
		eTime[node].y = ++time;
	}

	void calcOrder() {
		int time = 0;
		std::map<int, vec3i> eTime;
		//calc DFS
		DFS(main_shader_id, eTime, time);
		//add to pair list
		std::vector<std::pair<int, vec3i>> list;
		for (auto it = eTime.begin(); it != eTime.end(); ++it) {
			list.push_back(*it);
		}
		//sort pair list by finish time
		std::sort(list.begin(), list.end(), 
			[](const std::pair<int, vec3i> &a, const std::pair<int, vec3i>& b) -> bool {
				return a.second.y < b.second.y;
			});
		//add to order list
		order.clear();
		for (auto it = list.begin(); it != list.end(); ++it) {
			order.push_back(it->first);
		}
		
	}

	void initShader(vec2i res) {
		for (auto it = order.begin(); it != order.end(); ++it) {
			shaders[*it]->init(res);
		}
	}

public:
	Program() {}
	Program(Program &prog){}
	~Program(){}

	//registered new shader
	bool addShader(Shader &shader) {
		if (shaders.find(shader.getID()) != shaders.end()) {
			return false;
		}
		shader.setBuffer(bufferManager.newBuffer(shader.getResolution()));
		shaders[shader.getID()] = &shader;
		return true;
	}

	// find specified shaders by ID
	Shader *find(int ID) {
		if (shaders.find(ID) == shaders.end()) {
			return NULL;
		}
		return shaders[ID];
	}

	//connect path
	Shader &operator<<(Shader &shader) {
		main_shader_id = shader.getID();
		return shader;
	}

	// build rendering path
	void build(vec2i res) {
		calcOrder();
		initShader(res);
	}

	// render once
	Texture &render(FLOAT iTime=0) {
		//TODO:
		for (auto it = order.begin(); it != order.end(); ++it) {
			shaders[*it]->setTime(iTime);
			shaders[*it]->render();
		}
		std::cout << "Done!" << std::endl;
		return shaders[main_shader_id]->getBuffer().getTexture();
	}

	Texture &render(vec2i res, FLOAT iTime=0) {
		initShader(res);
		for (auto it = order.begin(); it != order.end(); ++it) {
			shaders[*it]->setTime(iTime);
			shaders[*it]->render();
		}
		std::cout << "Done!" << std::endl;
		return shaders[main_shader_id]->getBuffer().getTexture();
	}

	Texture &getOutput() {
		return shaders[main_shader_id]->getBuffer().getTexture();
	}
};


class RenderProgram {
public:
	virtual void build() = 0;
	virtual Texture &render(FLOAT iTime) = 0;
};

template<typename ShaderType>
class DefaultProgram : public RenderProgram {
public:
	Program program;
	ShaderType shader;
	vec2i Resolution;
	std::string name;

	DefaultProgram(vec2i res, std::string name="NoName Shader")
		: Resolution(res), name(name)  {    }

	virtual void build() override {
		shader.setName(name);
		program.addShader(shader);

		program << shader;

		program.build(Resolution);
	}

	virtual Texture &render(FLOAT iTime) override {
		return program.render(iTime);
	}

#ifdef OPENCV_SUPPORT
	void render(cv::Mat &frame, FLOAT iTime, int type) {
		if (type == CV_8UC3)
			program.render(iTime).to8U_BGR(frame);
		else if (type == CV_64FC4)
			program.render(iTime).to64F_BGRA(frame);
	}
#endif
};

typedef DefaultProgram<Shader> ExampleProgram;