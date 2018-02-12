#pragma once

#include "common/core.h"
#include "common/type.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

enum zoom_event{ZOOM_IN=0, ZOOM_OUT=1};
typedef void (*ZoomCallback)(int event, vec2 center, vec2 scale);

class ScreenControl {
public:

	// frame
	cv::Mat frame;
	cv::Mat part_frame;
	cv::Mat iter;
	cv::Mat part_iter;

	// screen params
	vec2i st;
	vec2i ed;
	FLOAT scale;
	FLOAT scale_factor;
	vec2i Resolution;
	std::string screen_name;

	//mouse params
	vec2i last_position;
	vec2i down_position;

	//mode
	int mode;
	
	//callback
	ZoomCallback zoomcallback;

	ScreenControl(vec2i res, std::string name) : st(0), ed(res), Resolution(res),
		scale(0), scale_factor(0.9),
		screen_name(name),
		mode(0) { }


	void setName(std::string name) { screen_name = name; }
	void setResolution(vec2i res) { Resolution = res; }
	void setScaleFactor(FLOAT scf) { scale_factor = scf; }
	void setMode(int mode) { this->mode = mode; }
	void setZoomCallback(ZoomCallback call) { zoomcallback = call; }


	void initialize() {
		st = vec2i(0);
		ed = Resolution;
		scale = 1;
	}

	void getScreen(cv::Mat &input, cv::Mat &output, bool keep_res = true) {
		cv::Rect r(st.x, st.y, ed.x - st.x, ed.y - st.y);
		output = input(r).clone();

		if (keep_res)
			cv::resize(output, output, input.size(), 0, 0, CV_INTER_NN);
	}

	void update() {
		getScreen(frame, part_frame);
		getScreen(iter, part_iter);

		cv::imshow(screen_name, part_frame);
	}

	//event
	void onScaleUp() {
		if (scale > 0.1) {
			scale *= scale_factor;
			std::cout << "scale: x " << 1 / scale << std::endl;
			onScale(scale_factor);
		}
	}

	void onScaleDown() {
		if (scale < 1.0) {
			scale /= scale_factor;
			std::cout << "scale: x " << 1 / scale << std::endl;
			onScale(1 / scale_factor);
		}
	}

	void onScale(FLOAT scale) {
		vec2 s = st;
		vec2 e = ed;
		vec2 ce = (s + e) / 2.;
		s = ce + (s - ce)*scale;
		e = ce + (e - ce)*scale;

		st = s;
		ed = e;
		finetune();
	}

	void onMove(vec2i mv) {
		st += mv;
		ed += mv;
		finetune();
	}

	static void onMouse(int event, int x, int y, int flags, void *params) {
		((ScreenControl*)params)->onMouse(event, x, y, flags);
	}

	void onZoom() {
		vec2 res = Resolution;
		vec2 scp = (vec2)(ed - st) * 0.5 + (vec2)st;
		vec2 sl =  (vec2)(ed - st) * 0.5;
		vec2 mx = sl / res;
		mx = res * max(mx.x, mx.y);

		st = scp - mx;
		ed = mx + scp;
		finetune();
		update();
		cv::waitKey(1);

		if (zoomcallback != NULL)
			zoomcallback(ZOOM_IN, scp, sl);
		
	}

	void onMouse(int event, int x, int y, int flags) {

		if (mode == 0) {  //search
			if (flags == CV_EVENT_FLAG_LBUTTON) {
				std::cout << "point: (" << x << ", " << y << ") / iter: " << iter.at<cv::Vec4d>(y, x)[0] << std::endl;
			}
		}
		else if (mode == 1) {  //move 
			if (flags == CV_EVENT_FLAG_LBUTTON) {
				vec2i mv = last_position - vec2i(x, y);
				onMove(mv);
			}
		}
		else if (mode == 2) {
			if (event == CV_EVENT_LBUTTONDOWN) {
				down_position = vec2i(x, y);
			}
			else if (event == CV_EVENT_LBUTTONUP) {
				vec2 rng = down_position - vec2i(x, y);
				vec2 cp = (vec2)(down_position + vec2i(x, y)) / 2.;
				vec2 res = Resolution;
				if (length(rng) > 50.0) {
					vec2 scp = cp / res * (vec2)(ed - st) + (vec2)st;
					vec2 sl = abs(cp - (vec2)down_position) / res * (vec2)(ed - st);
					vec2 mx = sl / res;
					mx = res * max(mx.x, mx.y);

					st = scp - mx;
					ed = mx + scp;
					finetune();
					update();
					cv::waitKey(1);

					if (zoomcallback != NULL)
						zoomcallback(ZOOM_IN, scp, sl);
				}
			}
			if (event == CV_EVENT_LBUTTONDOWN) {
				down_position = vec2i(x, y);
			}
			else if (event == CV_EVENT_LBUTTONUP) {
				vec2 rng = down_position - vec2i(x, y);
				vec2 cp = (vec2)(down_position + vec2i(x, y)) / 2.;
				vec2 res = Resolution;
				if (length(rng) > 50.0) {

				}
			}
		}
		
		if (event == CV_EVENT_MOUSEMOVE) {
			last_position = vec2i(x, y);
		}
	}

	void finetune() {
		if (st.x < 0) {
			ed.x += -st.x;
			st.x = 0;
		}
		if (st.y < 0) {
			ed.y += -st.y;
			st.y = 0;
		}
		if (ed.x >= Resolution.x) {
			st.x -= ed.x - (Resolution.x - 1);
			ed.x = Resolution.x - 1;
		}
		if (ed.y >= Resolution.y) {
			st.y -= ed.y - (Resolution.y - 1);
			ed.y = Resolution.y - 1;
		}
		if (st.x < 0) { st.x = 0; }
		if (st.y < 0) { st.y = 0; }
	}
};