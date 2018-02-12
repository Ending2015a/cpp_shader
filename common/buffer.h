#pragma once

#include <deque>
#include <map>

#include <assert.h>

#include <glm/glm.hpp>
#include "type.h"



#ifdef OPENCV_SUPPORT
#include <opencv2/core.hpp>
#endif

#define SRC_NULL 0
#define SRC_SHADER 1
#define SRC_AUDIO 2

#ifndef FLIP_TEXTURE_UP_DOWN
#define __FLIP_ false
#else
#define __FLIP_ true
#endif


class Buffer;
class BufferManager;

struct Texture {
	vec2i resolution;
	vec4 *raw_data;
	vec4 **data;
	vec4 *end;

	bool created;

	Texture() : raw_data(NULL), data(NULL), end(NULL), created(false) {}
	Texture(vec2i Resolution) {
		create(Resolution);
	}

	Texture(const Texture &tex){
		tex.copyTo(*this);
	}

	~Texture() {
		clear();
	}

	void copyTo(Texture &tex) const{
		if (!tex.created) {
			tex.create(resolution);
		}
		else if(tex.resolution != resolution){
			tex.clear();
			tex.create(resolution);
		}

		for (int i = 0; i < resolution.x*resolution.y; ++i){
			tex.raw_data[i] = raw_data[i];
		}
	}

	void copyFrom(const Texture &tex) {
		if (!created) {
			create(tex.resolution);
		}
		else if (tex.resolution != resolution) {
			clear();
			create(resolution);
		}
		for (int i = 0; i < resolution.x*resolution.y; ++i) {
			raw_data[i] = tex.raw_data[i];
		}
	}

	void create(vec2i Resolution) {
		resolution = Resolution;
		data = new vec4*[resolution.y];
		raw_data = new vec4[resolution.x*resolution.y];
		for (int i = 0; i < resolution.y; ++i) {
			data[i] = raw_data + i*resolution.x;
		}
		end = raw_data + resolution.x * resolution.y;
		created = true;
	}

	void clear() {
		if (data) delete[] data;
		if (raw_data)delete[] raw_data;
		created = false;
	}

	void zeros() {
		for (vec4 *ptr = raw_data; ptr != end; ++ptr) {
			*ptr = vec4(0);
		}
	}

	vec4 *operator[](int index) {
		assert(index < resolution.y);
		return data[index];
	}

#ifdef OPENCV_SUPPORT

	//return CV_64FC4 type mat
	void to64F_BGRA(cv::Mat &output, bool flip= __FLIP_) {
		output = cv::Mat(resolution.y, resolution.x, CV_64FC4);
		for (int i = 0; i < output.rows; ++i) {
			for (int j = 0; j < output.cols; ++j) {
				vec4 &c = data[i][j];
				if(flip)
					output.at<cv::Vec4d>(output.rows-1-i, j) = cv::Vec4d(c.z, c.y, c.x, c.w);
				else
					output.at<cv::Vec4d>(i, j) = cv::Vec4d(c.z, c.y, c.x, c.w);
			}
		}
	}

	//return CV_8UC3 type mat (BGR order)
	void to8U_BGR(cv::Mat &output, bool flip= __FLIP_) {
		output = cv::Mat(resolution.y, resolution.x, CV_8UC3);
		for (int i = 0; i < output.rows; ++i) {
			for (int j = 0; j < output.cols; ++j) {
				vec4 c = data[i][j]*255.0;
				if(flip)
					output.at<cv::Vec3b>(output.rows-1-i, j) = cv::Vec3b((uchar)c.z, (uchar)c.y, (uchar)c.x);
				else
					output.at<cv::Vec3b>(i, j) = cv::Vec3b((uchar)c.z, (uchar)c.y, (uchar)c.x);
			}
		}
	}

	//read from CV Mat. Mat type can be 8U/16U/32F/64F + C1/C3/C4 (BGR order)
	void fromCVMat(cv::Mat &input) {
		if (created)clear();
		create(vec2i(input.cols, input.rows));

		int chans = input.channels();
		int depth = input.type() & CV_MAT_DEPTH_MASK;

		cv::Mat clone;

		switch (chans) {
		case 1:
			cv::cvtColor(input, clone, CV_GRAY2RGBA, 1);
			break;
		case 3:
			cv::cvtColor(input, clone, CV_BGR2RGBA, 3);
			break;
		case 4:
			cv::cvtColor(input, clone, CV_BGRA2RGBA, 4);
			break;
		default:
			assert(false);
		}
			

		switch (depth) {
		case CV_8U:
			clone.convertTo(clone, CV_64FC4, 1.0 / 255.0);
			break;
		case CV_16U:
			clone.convertTo(clone, CV_64FC4, 1.0 / 65535.0);
			break;
		case CV_32F:
			clone.convertTo(clone, CV_64FC4);
			break;
		case CV_64F:
			break;
		default:
			assert(false);
		}

		//clone = 64FC4 / RGBA order
		for (int i = 0; i < input.rows; ++i) {
			for (int j = 0; j < input.cols; ++j) {
				cv::Vec4d c = clone.at<cv::Vec4d>(i, j);
				data[i][j] = vec4(c[0], c[1], c[2], c[3]);
			}
		}
	}

#endif
};

class Buffer{
	friend BufferManager;
private:
	Texture *tex;
public:
	uchar type;
	Buffer() { tex = NULL; type = SRC_NULL; }
	//construct with a texture (no copy)
	Buffer(Texture &tex) { this->tex = &tex; type = SRC_NULL; }
	Buffer(const Buffer &buf) { tex = buf.tex; type = buf.type; }
	
	//the ending point of texture
	virtual vec4 *end() final{
		return tex->end;
	}

	//the begining point of texture
	virtual vec4 *begin() final{
		return tex->raw_data;
	}

	//the resolution of the texture
	virtual vec2i getResolution() final{
		return tex->resolution;
	}

	virtual Texture &getTexture() {
		return *tex;
	}

	virtual bool hasTexture() {
		return tex != NULL;
	}

	vec4 *operator[](int index) {
		return tex->operator[](index);
	}

	vec4 pixel(vec2i p) {
		return tex->operator[](p.y)[p.x];
	}

	//ensure texture size is set to specified resolution
	void check(vec2i res) {
		assert(tex != NULL);
		if (!tex->created) {
			tex->create(res);
		}
		else if (tex->resolution != res) {
			tex->clear();
			tex->create(res);
		}
	}

	//copy from another texture
	virtual void copyFrom(Texture &ntex) {
		ntex.copyTo(*tex);
	}

	//initialize texture
	virtual void init() {
		assert(tex != NULL);
		tex->zeros();
	}
};


class BufferManager {
private:
	std::deque<Texture> textures;
public:
	BufferManager() : textures() { }

	Buffer newBuffer(vec2i res) {
		Buffer buf;
		textures.emplace_back(res);
		buf.tex = &textures.back();
		assert(buf.tex != NULL);
		return buf;
	}
};

typedef Buffer sampler2D;