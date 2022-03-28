#include "main.hpp"

#include "myarea.hpp"

#include <deep-learning.hpp>

#include <iostream>

#include <gtkmm/application.h>

#include <gtkmm/window.h>

#include "commands.hpp"


bool telemetre_status = true;

Gtk::Window * window = nullptr;
Gtk::EventBox * m_box = nullptr;
Gtk::ComboBox * tracking_algorithm_list = nullptr;
Gtk::Box * camera_mode_box = nullptr;
Gtk::Box * tracking_box = nullptr;
Gtk::Box * coupling_box = nullptr;
Gtk::Box * fire_box = nullptr;
Gtk::Box * telemetrie_box = nullptr;
Gtk::Label * camera_control_label = nullptr;
Gtk::Button * apply_button = nullptr;
Gtk::Button * manual_tracking_button = nullptr;
Gtk::Button * cancel_tracking_button = nullptr;
Gtk::Button * enable_joystick_button = nullptr;
Gtk::Button * disable_joystick_button = nullptr;
Gtk::Button * coupling_button = nullptr;
Gtk::Button * decoupling_button = nullptr;
Gtk::Button * fire_button = nullptr;
Gtk::Button * auto_button = nullptr;
Gtk::Button * telemetre_dynamic_button = nullptr;
Gtk::Button * telemetre_static_button = nullptr;
Gtk::Button * apply_distance_button = nullptr;
Gtk::SpinButton * spinbox_distance = nullptr;
Gtk::RadioButton * visible_camera_radio_button = nullptr;
Gtk::RadioButton * ir_camera_radio_button = nullptr;
Gtk::Button * activate_detection_button = nullptr;
Gtk::Button * desactivate_detection_button = nullptr;
Gtk::Button* apply_filters_button = nullptr;
Gtk::Button* remove_filters_button = nullptr;
Gtk::SpinButton * spinbox_bearing = nullptr;
Gtk::Scale* hue_min_scale = nullptr;
Gtk::Scale* hue_max_scale = nullptr;
Gtk::Scale* saturation_min_scale = nullptr;
Gtk::Scale* saturation_max_scale = nullptr;
Gtk::Scale* value_min_scale = nullptr;
Gtk::Scale* value_max_scale = nullptr;
using namespace std;
using namespace cv;
Myhardware hardware;

//SIGNAL_FUNCTIONS
void no_image()
{
    disable_all_detecting_functions();
    coupling_box->set_sensitive(false);
    tracking_box->set_sensitive(false);
    cout <<"no image"<<endl;
}	

void init_conditions()
{
  enable_all_detecting_functions();
  enable_all_tracking_functions();
//  display_without_filtering();
  disable_all_coupling_functions();
  fire_button -> set_sensitive(false);
  on_desactivate_detection_button_clicked();
 }

///-----------------------------------------------------------CAMERA VISIBLE/IR--------------------------------------------------------

void on_visible_camera_radio_button_toggled() {
  ir_camera_radio_button -> set_sensitive(true);
  visible_camera_radio_button -> set_sensitive(false);
  init_conditions();
}

void on_ir_camera_radio_button_toggled() {
  visible_camera_radio_button -> set_sensitive(true);
  ir_camera_radio_button -> set_sensitive(false);
  init_conditions();
}

///-----------------------------------------------------------DETECTION--------------------------------------------------------
void disable_all_detecting_functions() {
  apply_button -> set_sensitive(false);
  enable_joystick_button -> set_sensitive(false);
  spinbox_bearing -> set_sensitive(false);
  disable_joystick_button -> set_sensitive(false);
  hardware.joystick_disconnect();
  desactivate_detection_button -> set_sensitive(false);
  activate_detection_button -> set_sensitive(false);

  
}

void enable_all_detecting_functions() {
  apply_button -> set_sensitive(true);
  enable_joystick_button -> set_sensitive(true);
  spinbox_bearing -> set_sensitive(true);
  disable_joystick_button -> set_sensitive(false);
   desactivate_detection_button -> set_sensitive(true);
  hardware.joystick_disconnect();

}

void on_apply_button_clicked() {
  hardware.go_to_bearing(spinbox_bearing -> get_value());
}

void on_enable_joystick_button_clicked() {

  apply_button -> set_sensitive(false);
  disable_joystick_button -> set_sensitive(true);
  spinbox_bearing -> set_sensitive(false);
  enable_joystick_button -> set_sensitive(false);
  hardware.joystick_connect();
}

void on_disable_joystick_button_clicked() {
  enable_all_detecting_functions();
}

void on_activate_detection_button_clicked() {
  activate_detection_button -> set_sensitive(false);
  desactivate_detection_button -> set_sensitive(true);
  tracking_box -> set_sensitive(true);
  enable_all_telemetrie_functions(telemetre_status);
}

void on_desactivate_detection_button_clicked() {
  activate_detection_button -> set_sensitive(true);
  desactivate_detection_button -> set_sensitive(false);
  tracking_box -> set_sensitive(false);

}

///-----------------------------------------------------------TRACKING--------------------------------------------------------
void enable_all_tracking_functions() {
  tracking_algorithm_list -> set_sensitive(true);
  manual_tracking_button -> set_sensitive(true);
  auto_button -> set_sensitive(true);
  cancel_tracking_button -> set_sensitive(false);
  enable_all_detecting_functions();
  telemetrie_box->set_sensitive(false);
}
void disable_all_tracking_functions(bool enable_canceling) {
  //cout << "disable_all_tracking_functions" << endl;
  tracking_algorithm_list -> set_sensitive(false);
  manual_tracking_button -> set_sensitive(false);
  auto_button -> set_sensitive(false);
  disable_all_detecting_functions();
  coupling_button -> set_sensitive(enable_canceling);
  cancel_tracking_button -> set_sensitive(enable_canceling);
    enable_all_telemetrie_functions( telemetre_status);
  
}

void on_tracking_algorithm_list_changed() {
  cout << tracking_algorithm_list -> get_entry_text() << endl;
  memorisation_tracker(tracking_algorithm_list -> get_entry_text());
}

void on_auto_button_clicked() {
  cout << "on_auto_button_clicked" << endl;
  auto_button -> set_sensitive(false);
  manual_tracking_button -> set_sensitive(true);
}

void on_manual_tracking_button_clicked() {
  cout << "on_manual_tracking_button_clicked" << endl;
  manual_tracking_button -> set_sensitive(false);
  auto_button -> set_sensitive(true);
  call_draw_rectangle();

}

void on_cancel_tracking_button_clicked() {
  switch_mode();
  enable_all_tracking_functions();
  disable_all_coupling_functions();
  disable_all_firing_functions();
}

bool on_m_box_button_press_event(GdkEventButton * ) {
  cout << "MOUSE_CLICKED" << endl;
  if (!auto_button -> get_sensitive()) {
    call_click();
  }

  return true;
}
///-----------------------------------------------------------TELEMETRIE--------------------------------------------------------

void on_telemetre_dynamic_button_clicked()
{
 telemetre_dynamic_button->set_sensitive(false);
 telemetre_static_button->set_sensitive(true);
 spinbox_distance->set_sensitive(false);
 apply_distance_button->set_sensitive(false);
 telemetre_distance(0);
 telemetre_status = true;
}

void on_telemetre_static_button_clicked()
{
 telemetre_dynamic_button->set_sensitive(true);
 telemetre_static_button->set_sensitive(false);
 spinbox_distance->set_sensitive(true);
 apply_distance_button->set_sensitive(true);
 telemetre_status = false;
}

void on_apply_distance_button_clicked()
{
 telemetre_distance(spinbox_distance -> get_value());
}

void enable_all_telemetrie_functions( bool b)
{
 telemetrie_box->set_sensitive(true);
if (b)
{

  on_telemetre_dynamic_button_clicked();
}   
else 
{
  on_telemetre_static_button_clicked();
}
}


///-----------------------------------------------------------FILTERS--------------------------------------------------------
/*
void display_without_filtering()
{
apply_filters_button->set_sensitive(true);
remove_filters_button->set_sensitive(false);
hue_min_scale->set_sensitive(false);hue_max_scale->set_sensitive(false);
saturation_min_scale->set_sensitive(false);saturation_max_scale->set_sensitive(false);
value_min_scale->set_sensitive(false);value_max_scale->set_sensitive(false);
}

void on_apply_filters_button_clicked()
{
hue_min_scale->set_sensitive(true);
hue_max_scale->set_sensitive(true);
saturation_min_scale->set_sensitive(true);
saturation_max_scale->set_sensitive(true);
value_min_scale->set_sensitive(true);
value_max_scale->set_sensitive(true);
apply_filters_button->set_sensitive(false);
remove_filters_button->set_sensitive(true);
apply_filtering();
}

int* filtering()
{
static int F[6];
F[0]=int(hue_min_scale->get_value());
F[1]=int(hue_max_scale->get_value());
F[2]=int(saturation_min_scale->get_value());
F[3]=int(saturation_max_scale->get_value());
F[4]=int(value_min_scale->get_value());
F[5]=int(value_max_scale->get_value());
return F;
}

void on_remove_filters_button_clicked()
{
display_without_filtering();
disable_filtering();
}
*/
///-----------------------------------------------------------COUPLING-------------------------------------------------------
void on_coupling_button_clicked() {
  disable_all_detecting_functions();
  disable_all_tracking_functions(false);
  coupling_button -> set_sensitive(false);
  decoupling_button -> set_sensitive(true);
  coupling_mode_enabling(true);
  fire_button -> set_sensitive(true);
  cout << "coupling" << endl;
  telemetrie_box->set_sensitive(false);
}
void on_decoupling_button_clicked() {
  cancel_tracking_button -> set_sensitive(true);
  decoupling_button -> set_sensitive(false);
  coupling_button -> set_sensitive(true);
  coupling_mode_enabling(false);
  fire_button -> set_sensitive(false);
  cout << "decoupling" << endl;
  enable_all_telemetrie_functions( telemetre_status);
}
void disable_all_coupling_functions() {
  coupling_button -> set_sensitive(false);
  decoupling_button -> set_sensitive(false);
}
///-----------------------------------------------------------FIRE-------------------------------------------------------
void on_fire_button_clicked() {
  cout << "fire signal " << endl;
}

void  disable_all_firing_functions()
{
  fire_button -> set_sensitive(false);
}
///-----------------------------------------------------------MAIN-------------------------------------------------------
int main(int argc, char ** argv) {

  auto app = Gtk::Application::create(argc, argv, "pfe.exemple");

  //Load the GtkBuilder file and instantiate its widgets:
  auto refBuilder = Gtk::Builder::create();
  try {
    refBuilder -> add_from_file("../glade.glade");
  } catch (const Glib::FileError & ex) {
    std::cerr << "FileError: " << ex.what() << std::endl;
    return 1;
  } catch (const Glib::MarkupError & ex) {
    std::cerr << "MarkupError: " << ex.what() << std::endl;
    return 1;
  } catch (const Gtk::BuilderError & ex) {
    std::cerr << "BuilderError: " << ex.what() << std::endl;
    return 1;
  }
  MyArea area;

  //Get the GtkBuilder-instantiated Dialog:
  refBuilder -> get_widget("camera_control_label", camera_control_label);
  refBuilder -> get_widget("window", window);
  refBuilder -> get_widget("m_box", m_box);

  //BUILDER LINK WIDGETS  
  //Get the GtkBuilder-instantiated Button, and connect a signal handler:
  refBuilder -> get_widget("cancel_tracking_button", cancel_tracking_button);
  refBuilder -> get_widget("tracking_algorithm_list", tracking_algorithm_list);
  refBuilder -> get_widget("spinbox_bearing", spinbox_bearing);
  refBuilder -> get_widget("apply_button", apply_button);
  //refBuilder->get_widget("apply_filters_button", apply_filters_button);
  //  refBuilder->get_widget("remove_filters_button", remove_filters_button);
  refBuilder -> get_widget("manual_tracking_button", manual_tracking_button);
  refBuilder -> get_widget("auto_button", auto_button);
  refBuilder -> get_widget("disable_joystick_button", disable_joystick_button);
  refBuilder -> get_widget("enable_joystick_button", enable_joystick_button);
  //   refBuilder->get_widget("hue_min_scale", hue_min_scale);
  //   refBuilder->get_widget("saturation_min_scale", saturation_min_scale);
  //   refBuilder->get_widget("value_min_scale", value_min_scale);
  //   refBuilder->get_widget("hue_max_scale", hue_max_scale);
  //   refBuilder->get_widget("saturation_max_scale", saturation_max_scale);
  //   refBuilder->get_widget("value_max_scale", value_max_scale);
  refBuilder -> get_widget("coupling_button", coupling_button);
  refBuilder -> get_widget("decoupling_button", decoupling_button);
  refBuilder -> get_widget("camera_mode_box", camera_mode_box);
  refBuilder -> get_widget("tracking_box", tracking_box);
  refBuilder -> get_widget("coupling_box", coupling_box);
  refBuilder -> get_widget("fire_box", fire_box);
  refBuilder -> get_widget("fire_button", fire_button);
  refBuilder -> get_widget("telemetre_dynamic_button", telemetre_dynamic_button);
  refBuilder -> get_widget("telemetre_static_button", telemetre_static_button);
  refBuilder -> get_widget("apply_distance_button", apply_distance_button);
  refBuilder -> get_widget("spinbox_distance", spinbox_distance);
  refBuilder -> get_widget("visible_camera_radio_button", visible_camera_radio_button);
  refBuilder -> get_widget("ir_camera_radio_button", ir_camera_radio_button);
  refBuilder -> get_widget("activate_detection_button", activate_detection_button);
  refBuilder -> get_widget("desactivate_detection_button", desactivate_detection_button);
  refBuilder -> get_widget("telemetrie_box", telemetrie_box);
  //SIGNALS
  visible_camera_radio_button -> signal_clicked().connect(sigc::mem_fun(area, & MyArea::camera_visible));
  ir_camera_radio_button -> signal_clicked().connect(sigc::mem_fun(area, & MyArea::camera_ir));
  m_box -> signal_button_press_event().connect(sigc::ptr_fun( & on_m_box_button_press_event));
  tracking_algorithm_list -> signal_changed().connect(sigc::ptr_fun(on_tracking_algorithm_list_changed));
  cancel_tracking_button -> signal_clicked().connect(sigc::ptr_fun(on_cancel_tracking_button_clicked));
  apply_button -> signal_clicked().connect(sigc::ptr_fun(on_apply_button_clicked));
  auto_button -> signal_clicked().connect(sigc::ptr_fun(on_auto_button_clicked));
  
  telemetre_dynamic_button -> signal_clicked().connect(sigc::ptr_fun(on_telemetre_dynamic_button_clicked));
  telemetre_static_button -> signal_clicked().connect(sigc::ptr_fun(on_telemetre_static_button_clicked));
  apply_distance_button -> signal_clicked().connect(sigc::ptr_fun(on_apply_distance_button_clicked));
   
  manual_tracking_button -> signal_clicked().connect(sigc::ptr_fun(on_manual_tracking_button_clicked));
 // enable_joystick_button -> signal_clicked().connect(sigc::mem_fun(hardware, & Myhardware::joystick_connect));
 // disable_joystick_button -> signal_clicked().connect(sigc::mem_fun(hardware, & Myhardware::joystick_disconnect));
  enable_joystick_button -> signal_clicked().connect(sigc::ptr_fun(on_enable_joystick_button_clicked));
  disable_joystick_button -> signal_clicked().connect(sigc::ptr_fun(on_disable_joystick_button_clicked));
//apply_filters_button->signal_clicked().connect(sigc::ptr_fun(on_apply_filters_button_clicked));
//remove_filters_button->signal_clicked().connect(sigc::ptr_fun(on_remove_filters_button_clicked));
  coupling_button -> signal_clicked().connect(sigc::ptr_fun(on_coupling_button_clicked));
  decoupling_button -> signal_clicked().connect(sigc::ptr_fun(on_decoupling_button_clicked));
  fire_button -> signal_clicked().connect(sigc::ptr_fun(on_fire_button_clicked));
  visible_camera_radio_button -> signal_clicked().connect(sigc::ptr_fun(on_visible_camera_radio_button_toggled));
  ir_camera_radio_button -> signal_clicked().connect(sigc::ptr_fun(on_ir_camera_radio_button_toggled));
  activate_detection_button -> signal_clicked().connect(sigc::ptr_fun(on_activate_detection_button_clicked));
  activate_detection_button -> signal_clicked().connect(sigc::mem_fun(area, & MyArea::enable_detection));
  desactivate_detection_button -> signal_clicked().connect(sigc::ptr_fun(on_desactivate_detection_button_clicked));
  desactivate_detection_button -> signal_clicked().connect(sigc::mem_fun(area, & MyArea::disable_detection));
  ///--------------------------------------------------------INITIAL CONDITIONS-------------------------------------------------
  
  init_conditions();
  visible_camera_radio_button -> set_sensitive(false);
  m_box -> add(area);
  area.show();
  
  /////////////////////////////////

  return app -> run( * window);

  return 0;
}///-----------------------------------------------------------TRACKING--------------------------------------------------------
