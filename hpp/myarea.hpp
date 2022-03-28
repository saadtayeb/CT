#ifndef MYAREA_H
#define MYAREA_H
#include <opencv2/highgui.hpp>

#include <gtkmm.h>

class MyArea: public Gtk::DrawingArea {
  public: MyArea();
  virtual~MyArea();
  cv::VideoCapture videoCapture;
  void set_events(Gdk::EventMask events);
  void enable_detection();
  void disable_detection();
  void camera_visible();
  void camera_ir();
  protected:
    //Override default signal handler:
    bool on_draw(const Cairo::RefPtr < Cairo::Context > & cr) override;
  void on_size_allocate(Gtk::Allocation & allocation) override;
  bool on_eventbox_button_press(GdkEventButton * button_event);
  void on_get_pointer(int & x, int & y);
  void set_focus_on_click(bool focus_on_click);
  bool on_my_button_press_event(GdkEventButton * button_event);
  bool everyNowAndThen();
  Glib::RefPtr < Gdk::Pixbuf > m_image;

  private: sigc::connection everyNowAndThenConnection;
  cv::Mat webcam;
  cv::Mat output;
  int width,
  height;
  int detection_enable = 0;
  // 2-normal , 0-IR
};
#endif // MYAREA_H
