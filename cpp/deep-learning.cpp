#include <opencv2/opencv.hpp>

#include <torch/script.h>

#include <opencv2/highgui.hpp>

#include <algorithm>

#include <iostream>

#include <time.h>

#include <main.hpp>

#include "deep-learning.hpp"

#include <opencv2/tracking.hpp>

#include "commands.hpp"

using namespace std;
using namespace cv;
Point p1(cam_width/2, 0 ), p2(cam_width/2,cam_height);
Point p3(0, cam_height/2), p4(cam_width, cam_height/2);
Point p5(0.45*cam_width,0.45*cam_height), p6(0.55*cam_width,0.55*cam_height);
int thickness = 2;
Scalar color_canvas = Scalar(100,100,100);

std::vector < std::string > classnames {
  "Ferry",
  "Buoy",
  "Vessel/ship",
  "Speed boat",
  "Boat",
  "Kayak",
  "Sail",
  "boat",
  "Swimming person",
  "Flying bird/plane",
  "Other"
};
Mat img;
int low_H = 0, low_S = 0, low_V = 0;
int hue_min, hue_max, sat_min, sat_max, val_min, val_max;


int * HSV;
int x_coord, y_coord;
vector < torch::Tensor > dets_copy;
Rect2d trackingBox;
bool apply_filters = false;
bool detect_mode = true;
bool tracking_mode = false;
bool manual_tracking = false;
bool coupling_mode = false;
bool fire_mode = false;
float x_1, x_2, y_1, y_2;
int ecart_x, ecart_y;
int rows, cols;
int width_frame = 1280, height_frame = 720;
int tracker_index = 0;
string tracker_memoire = "MEDIANFLOW";
Ptr < Tracker > tracker = TrackerMedianFlow::create();
//load model 
torch::jit::script::Module module = torch::jit::load("../yolov5.torchscript");

vector < torch::Tensor > non_max_suppression(torch::Tensor preds, float score_thresh = 0.5, float iou_thresh = 0.5) {
  vector < torch::Tensor > output;
  for (size_t i = 0; i < preds.sizes()[0]; ++i) {
    torch::Tensor pred = preds.select(0, i);

    // Filter by scores
    torch::Tensor scores = pred.select(1, 4) * get < 0 > (torch::max(pred.slice(1, 5, pred.sizes()[1]), 1));
    pred = torch::index_select(pred, 0, torch::nonzero(scores > score_thresh).select(1, 0));
    if (pred.sizes()[0] == 0) continue;

    // (center_x, center_y, w, h) to (left, top, right, bottom)
    pred.select(1, 0) = pred.select(1, 0) - pred.select(1, 2) / 2;
    pred.select(1, 1) = pred.select(1, 1) - pred.select(1, 3) / 2;
    pred.select(1, 2) = pred.select(1, 0) + pred.select(1, 2);
    pred.select(1, 3) = pred.select(1, 1) + pred.select(1, 3);

    // Computing scores and classes
    tuple < torch::Tensor, torch::Tensor > max_tuple = torch::max(pred.slice(1, 5, pred.sizes()[1]), 1);
    pred.select(1, 4) = pred.select(1, 4) * get < 0 > (max_tuple);
    pred.select(1, 5) = get < 1 > (max_tuple);

    torch::Tensor dets = pred.slice(1, 0, 6);

    torch::Tensor keep = torch::empty({
      dets.sizes()[0]
    });
    torch::Tensor areas = (dets.select(1, 3) - dets.select(1, 1)) * (dets.select(1, 2) - dets.select(1, 0));
    tuple < torch::Tensor, torch::Tensor > indexes_tuple = torch::sort(dets.select(1, 4), 0, 1);
    torch::Tensor v = get < 0 > (indexes_tuple);
    torch::Tensor indexes = get < 1 > (indexes_tuple);
    int count = 0;
    while (indexes.sizes()[0] > 0) {
      keep[count] = (indexes[0].item().toInt());
      count += 1;

      // Computing overlaps
      torch::Tensor lefts = torch::empty(indexes.sizes()[0] - 1);
      torch::Tensor tops = torch::empty(indexes.sizes()[0] - 1);
      torch::Tensor rights = torch::empty(indexes.sizes()[0] - 1);
      torch::Tensor bottoms = torch::empty(indexes.sizes()[0] - 1);
      torch::Tensor widths = torch::empty(indexes.sizes()[0] - 1);
      torch::Tensor heights = torch::empty(indexes.sizes()[0] - 1);
      for (size_t i = 0; i < indexes.sizes()[0] - 1; ++i) {
        lefts[i] = max(dets[indexes[0]][0].item().toFloat(), dets[indexes[i + 1]][0].item().toFloat());
        tops[i] = max(dets[indexes[0]][1].item().toFloat(), dets[indexes[i + 1]][1].item().toFloat());
        rights[i] = min(dets[indexes[0]][2].item().toFloat(), dets[indexes[i + 1]][2].item().toFloat());
        bottoms[i] = min(dets[indexes[0]][3].item().toFloat(), dets[indexes[i + 1]][3].item().toFloat());
        widths[i] = max(float(0), rights[i].item().toFloat() - lefts[i].item().toFloat());
        heights[i] = max(float(0), bottoms[i].item().toFloat() - tops[i].item().toFloat());
      }
      torch::Tensor overlaps = widths * heights;

      // FIlter by IOUs
      torch::Tensor ious = overlaps / (areas.select(0, indexes[0].item().toInt()) + torch::index_select(areas, 0, indexes.slice(0, 1, indexes.sizes()[0])) - overlaps);
      indexes = torch::index_select(indexes, 0, torch::nonzero(ious <= iou_thresh).select(1, 0) + 1);
    }
    keep = keep.toType(torch::kInt64);
    output.push_back(torch::index_select(dets, 0, keep.slice(0, 0, count)));
  }
  return output;
}

void tracking_algorithm(string tr) {
  if (tr == "BOOSTING") {
    tracker = TrackerBoosting::create();
    cout << "choose BOOSTING" << endl;
  } else if (tr == "MIL") {
    tracker = TrackerMIL::create();
    cout << "choose MIL" << endl;
  } else if (tr == "KCF") {
    tracker = TrackerKCF::create();
    cout << "choose KCF" << endl;
  } else if (tr == "TLD") {
    tracker = TrackerTLD::create();
    cout << "choose TLD" << endl;
  } else if (tr == "MEDIANFLOW") {
    tracker = TrackerMedianFlow::create();
    cout << "choose MEDIANFLOW" << endl;
  } else if (tr == "GOTURN") {
    tracker = TrackerGOTURN::create();
    cout << "choose GOTURN" << endl;
  } else if (tr == "MOSSE") {
    tracker = TrackerMOSSE::create();
    cout << "choose MOSSE" << endl;
  } else if (tr == "CSRT") {
    tracker = TrackerCSRT::create();
    cout << "choose CSRT" << endl;
  }

}

void add_bounding_boxes(Mat frame) {
draw_canvas(frame);
  if (manual_tracking) {
    //cout << "manual_tracking" << endl;
    cvtColor(frame, frame, COLOR_BGR2RGB);
    try
    {
    	trackingBox = selectROI("manual_selector", frame,true , false);

    	if (trackingBox.empty())
    	{
    	cout << "not tracking" << endl;
    	}
    	else 
    	{
    	cout << "tracking" << endl;
    	disable_all_tracking_functions(!coupling_mode);
    	}
    	tracking_mode = true;
    	detect_mode = false;
    	coupling_mode = false;
    	fire_mode = false;
    	manual_tracking = false;
    	destroyWindow("manual_selector");
    	throw 500;
     }
     catch(int num)
     {
     	cout<<"problem"<<endl;
     }
     
  }
  if (detect_mode) {

    //cout << "DETECTING" << endl;
    //cout<<x_coord<<"    "<<y_coord<<endl;
    // Preparing input tensor
    
    resize(frame, img, Size(640, 640));

    torch::Tensor imgTensor = torch::from_blob(img.data, {
      img.rows,
      img.cols,
      3
    }, torch::kByte);
    imgTensor = imgTensor.permute({
      2,
      0,
      1
    });
    imgTensor = imgTensor.toType(torch::kFloat);
    imgTensor = imgTensor.div(255);
    imgTensor = imgTensor.unsqueeze(0);
    // preds: [?, 15120, 9]
    torch::Tensor preds = module.forward({
      imgTensor
    }).toTuple() -> elements()[0].toTensor();
    vector < torch::Tensor > dets = non_max_suppression(preds, 0.4, 0.5);
    dets_copy = dets;
    cols = frame.cols;
    rows = frame.rows;
    if (dets.size() > 0) {
      // Visualize result
      for (size_t i = 0; i < dets[0].sizes()[0]; ++i) {
        float left = dets[0][i][0].item().toFloat() * frame.cols / 640;
        float top = dets[0][i][1].item().toFloat() * frame.rows / 640;
        float right = dets[0][i][2].item().toFloat() * frame.cols / 640;
        float bottom = dets[0][i][3].item().toFloat() * frame.rows / 640;
        float score = dets[0][i][4].item().toFloat();
        int classID = dets[0][i][5].item().toInt();

        rectangle(frame, Rect(left, top, (right - left), (bottom - top)), Scalar(255, 255, 255), 2);

        putText(frame,
          classnames[classID] + ": " + format("%.2f", score),
          Point(left, top),
          FONT_HERSHEY_SIMPLEX, 0.5 , Scalar(0,0,0), 2);
      }

    }
  } else if (tracking_mode) {
    tracker -> init(frame, trackingBox);
    if (tracker -> update(frame, trackingBox)) {
      ecart_x = (trackingBox.x + trackingBox.width / 2) - cam_width / 2; // + droite - gauche 
      ecart_y = cam_height / 2 - (trackingBox.y + trackingBox.height / 2); //  +haut    - bas  
      rectangle(frame, trackingBox, Scalar(255, 0, 0), 2, 8);
      if (coupling_mode) {
      move_camera(ecart_x,ecart_y, true);
        cout << "coupling" << endl;
      } else {
      move_camera(ecart_x,ecart_y, false);
        //cout << "TRACKING with" << tracker_memoire << endl;
      };
    } else {
      cout << "TRACKER LOST";
      on_cancel_tracking_button_clicked();
      switch_mode();
    }

  }
}

void call_draw_rectangle() {
  //cout << "click in manual tracking mode" << endl;
  manual_tracking = true;
}

void call_click() {
  cout << "x_coord    " << x_coord << "    y_coord" << y_coord << endl;
  if (dets_copy.size() > 0) {
    for (size_t i = 0; i < dets_copy[0].sizes()[0]; ++i) {
      x_1 = dets_copy[0][i][0].item().toFloat() * cols / 640;
      y_1 = dets_copy[0][i][1].item().toFloat() * rows / 640;
      x_2 = dets_copy[0][i][2].item().toFloat() * cols / 640;
      y_2 = dets_copy[0][i][3].item().toFloat() * rows / 640;
      if (x_1 <= x_coord && x_coord <= x_2 && y_1 <= y_coord && y_coord <= y_2 && detect_mode) {
        cout << "TRACKING" << endl;
        trackingBox = Rect2d(x_1, y_1, x_2 - x_1, y_2 - y_1);
        detect_mode = false;
        tracking_mode = true;
        coupling_mode = false;
        fire_mode = false;
        disable_all_tracking_functions(!coupling_mode);
        break;
      }

    }
  }
}

void memorisation_tracker(string algo_chosen) {
  tracker_memoire = algo_chosen;
  tracking_algorithm(tracker_memoire);
}

void switch_mode() {
  tracking_algorithm(tracker_memoire);
  detect_mode = true;
  tracking_mode = false;
  coupling_mode = false;
  fire_mode = false;
}
void coupling_mode_enabling(bool x) {
  coupling_mode = x;
}
void fire_mode_enabling(bool y) {
  fire_mode = y;
}

void disable_filtering() {
  apply_filters = false;
}

void apply_filtering() {
  apply_filters = true;
}

void draw_canvas(Mat frame)
{
line(frame, p1, p2, color_canvas, thickness, LINE_AA);
line(frame, p3, p4, color_canvas, thickness, LINE_AA);
rectangle(frame,p5,p6,color_canvas, thickness, LINE_AA);
}
