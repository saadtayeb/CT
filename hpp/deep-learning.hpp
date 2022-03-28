#ifndef DEEP_LEARNING_H
#define DEEP_LEARNING_H

#include <opencv2/opencv.hpp>

#include <opencv2/highgui.hpp>

#include "opencv2/tracking.hpp"

#include <torch/script.h>
extern int cam_width ;
extern int cam_height;
extern int x_coord, y_coord;
extern int ecart_x, ecart_y;
void call_click();
void add_bounding_boxes(cv::Mat frame);
void memorisation_tracker(std::string algo_chosen);
void switch_mode();
void coupling_mode_enabling(bool x);
void fire_mode_enabling(bool y);
void call_draw_rectangle();
void apply_filtering();
void disable_filtering();
void draw_canvas(cv::Mat frame);
#endif // DEEP_LEARNING_H
