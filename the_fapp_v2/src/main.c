#include <pebble.h>
#include "cl_util.h"

#define KEY_START 0
#define NUM_SAMPLES 1
#define ACCEL_STEP_MS 50
  
static Window *window;
static TextLayer *x_layer, *y_layer, *z_layer;
static int latest_data[3 * NUM_SAMPLES];
GRect bounds;

static TextLayer *top_text;
static TextLayer *middle_text;
static TextLayer *bottom_text;
static TextLayer *fpm_text;
Layer *window_layer;
int faps = 0;
bool pos = true;
bool menu = true;

int time_elapsed=0;
static AppTimer *timer;
AccelData accel;
double fpm = 0;
bool paused = false;
bool stopped = false;
//int state = 0;


static void make_menu(){
  menu = true;
  text_layer_set_text(top_text, "THE");
  text_layer_set_text(middle_text, "FAPP");
  text_layer_set_text(bottom_text,"Waiting for Android..");
}

static void clear(){
  faps = 0;
  time_elapsed = 0;
  paused = false;
  stopped = false;
}

static void changePaused(){
   if(!menu){
    paused=!paused;
    if(paused){
      text_layer_set_text(bottom_text,"Fapping Paused.");
    }else{
      text_layer_set_text(bottom_text, "Fapping in Progress.");  
    }
  }
}

static void timer_callback(void *data) {
  time_elapsed++;
 // APP_LOG(APP_LOG_LEVEL_DEBUG,"fap:%d",faps);
  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}


static void start_fapping(){
  //draws the fapping arena
  //draw title
    menu = false;
    //state = 1;
    paused = false;
    GFont top_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FAP_22));
    text_layer_set_font(top_text, top_font);
    text_layer_set_text(top_text, "FAPP COUNTER");
    
    //draw middle area
     GRect move_pos = (GRect) { .origin = { -15, 45 }, .size = { 180, 180 } };
    layer_set_frame(text_layer_get_layer(middle_text),move_pos);
    
    //draw number of faps
     static char body_text[50];
     snprintf(body_text, sizeof(body_text), "%u", faps);
     text_layer_set_text(middle_text, body_text);
    
    text_layer_set_text(bottom_text, "Fapping in Progress.");   
}

void middle_click_handler(ClickRecognizerRef recognizer, void *context) {
  changePaused();
}

void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, middle_click_handler);
}

static void window_load(Window *window) 
{
  GFont top_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FAP_35));
  GFont middle_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FAP_50));
  GFont bottom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEFAULT_20));  
  GFont fpm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEFAULT_18));  
  
  top_text = text_layer_create(bounds);
  middle_text = text_layer_create(bounds);
  bottom_text = text_layer_create(bounds);
  fpm_text = text_layer_create(bounds);
  text_layer_set_font(top_text, top_font);
  text_layer_set_font(middle_text, middle_font);
  text_layer_set_font(bottom_text, bottom_font);
  text_layer_set_font(fpm_text, fpm_font);
  text_layer_set_text_alignment(top_text, GTextAlignmentCenter);
  text_layer_set_text_alignment(middle_text, GTextAlignmentCenter);
  text_layer_set_text_alignment(bottom_text, GTextAlignmentCenter);
  text_layer_set_text_alignment(fpm_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(top_text));
  layer_add_child(window_layer, text_layer_get_layer(middle_text));
  layer_add_child(window_layer, text_layer_get_layer(bottom_text));
  layer_add_child(window_layer, text_layer_get_layer(fpm_text));
  GRect move_pos = (GRect) { .origin = { -15, 39 }, .size = { 180, 180 } };
  layer_set_frame(text_layer_get_layer(middle_text),move_pos);
  GRect move_pos2 = (GRect) { .origin = { -15, 105 }, .size = { 180, 180 } };
  layer_set_frame(text_layer_get_layer(bottom_text),move_pos2);
  GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
  layer_set_frame(text_layer_get_layer(fpm_text),move_pos3);
  
  make_menu();
  window_set_click_config_provider(window, config_provider);
  
}


static void redraw_text(){
  //sets fap_value
  static char body_text[50];
  snprintf(body_text, sizeof(body_text), "%u", faps);
  text_layer_set_text(middle_text, body_text);
  
}



static void window_unload(Window *window) 
{
  text_layer_destroy(top_text);
  text_layer_destroy(middle_text);
  text_layer_destroy(bottom_text);
  text_layer_destroy(fpm_text);
  //layer_destroy(window_layer);
}



static void accel_new_data(AccelData *data, uint32_t num_samples)
{
 // APP_LOG(APP_LOG_LEVEL_DEBUG,"YO");
	for(uint32_t i = 0; i < num_samples; i++)
	{
    
    	if(!menu){
        //faps++;
        int y_grav = data[i].y;
        if(pos){
          if(y_grav<0){
            pos=false;
            if(!paused && !stopped){
              faps++;
            }
          }
        }else{
          if(y_grav>0){
            pos=true;
          }
        }
        data[i].x = faps;
        int p = 1;
        if(paused){
          p = 0;
        }
        data[i].y = p;
        
        fpm=(double)((double)faps/(double)(time_elapsed/20))*60;
        data[i].z = fpm;
        
      	latest_data[(i * 3) + 0] = (int)(0 + data[i].x);	//0, 3, 6
	    	latest_data[(i * 3) + 1] = (int)(0 + data[i].y);	//1, 4, 7
		    latest_data[(i * 3) + 2] = (int)(0 + data[i].z);	//2, 5, 8
        redraw_text();
      }
  }
  
}

static void in_dropped_handler(AppMessageResult reason, void *context) 
{ 
	cl_interpret_message_result(reason);
}

static void send_next_data()
{
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	for(int i = 0; i < NUM_SAMPLES; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			int value = 0 + latest_data[(3 * i) + j];
			Tuplet t = TupletInteger((3 * i) + j, value);
			dict_write_tuplet(iter, &t);
		}
	}
	
	app_message_outbox_send();
}

static void out_sent_handler(DictionaryIterator *iter, void *context)
{
	//CAUTION - INFINITE LOOP
	send_next_data();
}

static void process_tuple(Tuple *t)
{
	switch(t->key){
	case KEY_START: 
    //start
    clear();
    start_fapping();
		send_next_data();
	break;
  case 1: 
   // clear
    clear();
		send_next_data();
    paused = !paused;
	break;
  case 2: 
   // paused
    changePaused();
		send_next_data();
	break;
  case 3: 
   // stop
    stopped = true;
    make_menu();
		send_next_data();
	break;
	}
}

static void in_received_handler(DictionaryIterator *iter, void *context)
{	
	//Get data
	Tuple *t = dict_read_first(iter);
	if(t)
	{
		process_tuple(t);
	}
	
	//Get next
	while(t != NULL)
	{
		t = dict_read_next(iter);
		if(t)
		{
			process_tuple(t);
		}
	}
}

static void out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context)
{
	cl_interpret_message_result(result);
}

static void init(void) 
{
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
  //window_set_click_config_provider(window, config_provider); //<-unslash later
  window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  
  
	cl_set_debug(true);

  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
	accel_data_service_subscribe(NUM_SAMPLES, (AccelDataHandler)accel_new_data);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);

	int in_size = app_message_inbox_size_maximum();
	int out_size = app_message_outbox_size_maximum();
	app_log(APP_LOG_LEVEL_INFO, "C", 0, "I/O Buffer: %d/%d", in_size, out_size);
	app_message_open(in_size, out_size);

	window_stack_push(window, true);
}

static void deinit(void) 
{
	accel_data_service_unsubscribe();
	window_destroy(window);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}