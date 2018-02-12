#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <direct.h>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

#define OPENCV_SUPPORT
#define OPENMP_SUPPORT
#define FLIP_TEXTURE_UP_DOWN
#define DEBUG_LOG
#define TIME_MEASURE
#define CUT_Y
#define AA 2

#include "common/core.h"

#include "examples/mandelbulb.h"
#include "examples/mandelbrot.h"

#include "scrcontrol.h"

//============== Mode =================
bool RenderMode = true;

//============== PARAMS ===============
//vec2i Resolution(5120, 2880);
vec2i Resolution(1280, 720);
std::string output_folder = "output\\";
int num_thread = 8;
bool oneFrame = true;
FLOAT iTime = 21.72;

//============== VIDEOS ===============
std::string output_video_name = "video.avi";
int total_frame = 30;
int fps = 30;

//============== Program ==============
DefaultProgram<MandelbulbV3D> program(Resolution, "Mandelbulb3D");
DefaultProgram<MandelbrotVd> dprogram(Resolution, "Mandelbrot debug");

std::string get_video_name() {
	return output_folder + output_video_name;
}

std::string get_frame_name(int i, int filled) {
	std::stringstream ss;
	ss << "frame_" << std::setw(filled) << std::setfill('0') << i << ".png";

	return output_folder + ss.str();
}

std::string get_image_name() {
	return output_folder + "image.png";
}

int render_mode(int argc, char **argv) {
	std::cout << "Resolution: (" << Resolution.x << ", " << Resolution.y << ")" << std::endl;
	std::cout << "Output Folder: " << output_folder << std::endl;

	_rmdir(output_folder.c_str());
	_mkdir(output_folder.c_str());

	program.build();

	if (oneFrame) {
		cv::Mat frame;
		program.render(frame, iTime, CV_8UC3);
		cv::imshow("image", frame);
		cv::imwrite(get_image_name(), frame);
		cv::waitKey(0);
	}
	else {
		int dig = total_frame > 0 ? (int)log10((double)total_frame) + 1 : 1;

		for (int curframe = 0; curframe < total_frame; ++curframe) {
			FLOAT time = (FLOAT)curframe / (FLOAT)fps;
			cv::Mat frame;
			program.render(frame, iTime, CV_8UC3);

			cv::imshow("image", frame);
			cv::imwrite(get_frame_name(curframe, dig), frame);

			std::cout << "Cur Frame: " << curframe + 1 << "/" << total_frame;
			cv::waitKey(5);
		}

		std::cout << "All frame done ! Generating video..." << std::endl;

		cv::VideoWriter writer;
		writer.open(get_video_name(), CV_FOURCC('M', 'J', 'P', 'G'), fps, cv::Size(Resolution.x, Resolution.y));

		for (int curframe = 0; curframe < total_frame; ++curframe) {

			std::cout << "Cur frame: " << curframe << std::endl;
			cv::Mat frame = cv::imread(get_frame_name(curframe, dig));
			writer.write(frame);
		}
		writer.release();
	}
	return 0;
}


ScreenControl screen(Resolution, "Debug");
vec2i last_mouse_point;
vec2i mouse_down;

void onRender() {
	program.render(screen.frame, iTime, CV_8UC3);
	dprogram.render(screen.iter, iTime, CV_64FC4);
}

void onZoom(int event, vec2 center, vec2 length) {
	vec2 res = Resolution;
	vec2 x = center;
	vec2 n = length;
	if (event == ZOOM_IN) {
		vec2 res = Resolution;
		vec2 cx = program.shader.scale * (-res + 2.0*x) / res.y + program.shader.center;
		vec2 scx = program.shader.scale * 2.*n / res;

		program.shader.scale = max(scx.x, scx.y);
		program.shader.center = cx;
		dprogram.shader.scale = max(scx.x, scx.y);
		dprogram.shader.center = cx;
		
		onRender();
		screen.initialize();
	}
	else if (event == ZOOM_OUT) {

	}
}

int debug_mode(int argc, char **argv) {

	cv::namedWindow("Debug");
	cv::setMouseCallback("Debug", ScreenControl::onMouse, (void*)&screen);
	screen.setZoomCallback(onZoom);

	program.build();
	dprogram.build();

	onRender();

	screen.initialize();

	while (true) {

		screen.update();
		
		int key = cv::waitKey(10);
		if (key != -1)std::cout << key << std::endl;
		switch (key) {
		case 43: //+
			screen.onScaleUp();
			break;
		case 45: //-
			screen.onScaleDown();
			break;
		case 113: //q
			screen.setMode(0);
			std::cout << "search" << std::endl;
			break;
		case 119: //w
			screen.setMode(1);
			std::cout << "move" << std::endl;
			break;
		case 101: //e
			screen.setMode(2);
			std::cout << "zoom" << std::endl;
			break;
		case 48: // 0
			screen.initialize();
			program.shader.center = vec2(-0.5, 0);
			program.shader.scale = 1;
			dprogram.shader.center = vec2(-0.5, 0);
			dprogram.shader.scale = 1;
			onRender();
		case 32:
			screen.onZoom();
			break;
		case 115:
			cv::imwrite("image.png", screen.frame);
			std::cout << "save to image.png" << std::endl;
			break;
		case 27: //ESC
			return 0;
		}
	}

	return 0;
}


int main(int argc, char **argv) {

	if (RenderMode){
		std::cout << " ::Render Mode" << std::endl;
		render_mode(argc, argv);
	}
	else{
		std::cout << " ::Debug Mode" << std::endl;
		debug_mode(argc, argv);
	}

	return 0;
}