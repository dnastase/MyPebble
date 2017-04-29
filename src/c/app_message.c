#include <pebble.h>

Window* wndMain;
TextLayer* wndTime;

// Keys for AppMessage Dictionary
// These should correspond to the values you defined in appinfo.json/Settings
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

// Write message to buffer & send
static void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, MESSAGE_KEY, "I'm a Pebble!");
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}
  
  send_message();
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Dropped Message: %d", reason);
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "NotAcked Message: %d", reason);
}

static void main_window_load(Window* window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  wndTime = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(wndTime, GColorClear);
  text_layer_set_text_color(wndTime, GColorBlack);
  text_layer_set_text(wndTime, "00:00");
  text_layer_set_font(wndTime, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(wndTime, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(wndTime));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
   static char s_time_buffer[16];
   strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
   text_layer_set_text(wndTime, s_time_buffer);
}

static void main_window_unload(Window* window) {
  // Destroy TextLayer
  text_layer_destroy(wndTime);
}

void init(void) 
{
   // Register AppMessage handlers
   app_message_register_inbox_received(in_received_handler); 
   app_message_register_inbox_dropped(in_dropped_handler); 
   app_message_register_outbox_failed(out_failed_handler);

   //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
   app_message_open(512, 512);
   
   wndMain = window_create();
   // Set handlers to manage the elements inside the Window
   window_set_window_handlers(wndMain, (WindowHandlers) {
     .load = main_window_load,
     .unload = main_window_unload
   });
   
   window_stack_push(wndMain, true);

   send_message();
   
   tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Inited"); 
}

void deinit(void) 
{
   tick_timer_service_unsubscribe();
   app_message_deregister_callbacks();
   window_destroy(wndMain);
}

//static void init(void) {
//	s_window = window_create();
//	window_stack_push(s_window, true);
//	
//	// Register AppMessage handlers
//	app_message_register_inbox_received(in_received_handler); 
//	app_message_register_inbox_dropped(in_dropped_handler); 
//	app_message_register_outbox_failed(out_failed_handler);
//
//  // Initialize AppMessage inbox and outbox buffers with a suitable size
//  const int inbox_size = 128;
//  const int outbox_size = 128;
//	app_message_open(inbox_size, outbox_size);
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "Inited"); 
//}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}