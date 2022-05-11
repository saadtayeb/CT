#include "myarea.hpp"

#include "commands.hpp"

#include <cairomm/context.h>

#include <giomm/resource.h>

#include <gdkmm/general.h>

#include <glibmm/fileutils.h>

#include <iostream>

#include "opencv2/imgproc.hpp"

#include "opencv2/core.hpp"

#include "opencv2/highgui.hpp"

#include "opencv2/tracking.hpp"

#include "deep-learning.hpp"

#include "commands.hpp"

#include "main.hpp"
int cam_width  = 960;
int cam_height = 640;

using namespace std;
using namespace cv;
//"/home/saad/Desktop/yolo/video2.avi"
MyArea::MyArea():
  videoCapture("../video.mp4") {
  videoCapture.set(CAP_PROP_FRAME_HEIGHT, 720);
  videoCapture.set(CAP_PROP_FRAME_WIDTH, 1280);

    // Lets refresh drawing area very now and then.
    everyNowAndThenConnection = Glib::signal_timeout().connect(sigc::mem_fun( * this, & MyArea::everyNowAndThen),20);
  }
MyArea::~MyArea() {
  everyNowAndThenConnection.disconnect();
}

bool MyArea::everyNowAndThen() {
  auto win = get_window();
  if (win) {
    Gdk::Rectangle r(0, 0, cam_width, cam_height);
    win -> invalidate_rect(r, false);
  }
  return true;

}

void MyArea::on_size_allocate(Gtk::Allocation & allocation) {
  // Call the parent to do whatever needs to be done:
  DrawingArea::on_size_allocate(allocation);

  // Remember the new allocated size for resizing operation:
  width = cam_width;
  height = cam_height;
}

void MyArea::set_events(Gdk::EventMask events) {}

void MyArea::set_focus_on_click(bool focus_on_click = true) {}

bool MyArea::on_my_button_press_event(GdkEventButton * button_event) {

  return true;
}

void MyArea::camera_visible() {
  videoCapture.open(2,CAP_GSTREAMER);
  videoCapture.set(CAP_PROP_FRAME_HEIGHT, 720);
  videoCapture.set(CAP_PROP_FRAME_WIDTH, 1080);
  detection_enable = 0;
}

void MyArea::enable_detection() {
  detection_enable = 1;
  cout << "enable detection " << endl;
}

void MyArea::disable_detection() {
  detection_enable = 0;
  cout << "disable detection" << endl;
}

void MyArea::camera_ir() {
  videoCapture.open("../video.mp4");
  detection_enable = 0;	
}

void MyArea::on_get_pointer(int & x, int & y) {

}

bool MyArea::on_draw(const Cairo::RefPtr < Cairo::Context > & cr) {

  if (width == 0 || height == 0)
    return false;
  if (videoCapture.read(webcam)) {
    resize(webcam, output, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
	cvtColor(output, output, COLOR_BGR2RGB);
    if (detection_enable == 1) {
      add_bounding_boxes(output);
    }
    Glib::RefPtr < Gdk::Pixbuf > pixbuf =
      Gdk::Pixbuf::create_from_data((guint8 * ) output.data,
        Gdk::COLORSPACE_RGB,
        false,
        8,
        output.cols,
        output.rows,
        (int) output.step);

    // Display
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr -> paint();
    get_pointer(x_coord, y_coord);
    return true;
  } else {
    no_image();
    m_image = Gdk::Pixbuf::create_from_file("../images/no_image.png");
    Gdk::Cairo::set_source_pixbuf(cr, m_image,
      (cam_width - m_image -> get_width()) / 2, (cam_height - m_image -> get_height()) / 2);
    cr -> paint();
    return false;
  }
}
