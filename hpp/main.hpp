#ifndef MAIN_H
#define MAIN_H
#include <opencv2/highgui.hpp>

#include <gtkmm.h>


void no_image();
void init_conditions();
void on_visible_camera_radio_button_toggled();
void on_ir_camera_radio_button_toggled();
void disable_all_detecting_functions();
void enable_all_detecting_functions();
void on_apply_button_clicked();
void on_enable_joystick_button_clicked();
void on_disable_joystick_button_clicked();
void on_activate_detection_button_clicked();
void on_desactivate_detection_button_clicked();
void enable_all_tracking_functions();
void disable_all_tracking_functions(bool enable_canceling);
void on_tracking_algorithm_list_changed();
void on_auto_button_clicked();
void on_manual_tracking_button_clicked();
void on_cancel_tracking_button_clicked();
bool on_m_box_button_press_event(GdkEventButton * );
void on_telemetre_dynamic_button_clicked();
void on_telemetre_static_button_clicked();
void on_apply_distance_button_clicked();
void enable_all_telemetrie_functions( bool b);

//void display_without_filtering();
//int * filtering();
//void on_apply_filters_button_clicked();
//void on_remove_filters_button_clicked();
void on_coupling_button_clicked();
void on_decoupling_button_clicked();
void disable_all_coupling_functions();
void on_fire_button_clicked();
void  disable_all_firing_functions();
#endif // MAIN_H
