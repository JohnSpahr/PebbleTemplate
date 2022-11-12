/*
  Disintegration - Pebble Watch Face

  Created by John Spahr (johnspahr.org)

  Thanks to: "PebbleFaces" example, the watchface creation guide hosted by Rebble, the Rebble team, and The Cure!
*/
#include <pebble.h>

static Window *window;            // window object
static BitmapLayer *bitmap_layer; // layer to display the bitmap
static GBitmap *cover;      // color cover bitmap
static TextLayer *s_time_layer;   // the time (text layer)
static GFont s_time_font;         // custom time font

bool isConnected = true; // global boolean value that is used to track if the watch is connected to the phone

static void update_time()
{
  // get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // write the current time into buffer...
  static char s_buffer[8];

  // format current time
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // set textlayer text to current time
  text_layer_set_text(s_time_layer, s_buffer);

  // change text color depending on connection status
  if (isConnected)
  {
    // if watch is connected to phone, set color to orange
    text_layer_set_text_color(s_time_layer, GColorOrange);
  }
  else
  {
    // if watch is NOT connected to phone, set color to red
    text_layer_set_text_color(s_time_layer, GColorRed);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time(); // update the time on timer tick
}

static void load_background()
{
  cover = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COVER); // create bitmap from image resource
  bitmap_layer_set_bitmap(bitmap_layer, cover);                        // set bitmap layer image
}

static void bluetooth_callback(bool connected)
{
  if (connected)
  {
    // when connected...
    isConnected = true; // indicate that watch is connected to phone globally
    update_time();      // update time in case disconnection brackets are visible
  }
  else
  {
    // otherwise...
    isConnected = false;  // inidcate that watch is NOT connected to phone globally
    vibes_double_pulse(); // issue a vibrating alert on disconnect
    update_time();        // update time to display disconnection brackets
  }
}

static void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window); // get current window layer
  GRect bounds = layer_get_bounds(window_layer);       // get screen bounds

  bitmap_layer = bitmap_layer_create(bounds);                          // create bitmap layer
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer)); // add bitmap layer to window
  bitmap_layer_set_background_color(bitmap_layer, GColorBlack);        // set bitmap layer background color to black

  // create time text layer...
  s_time_layer = text_layer_create(
      GRect(0, 4, bounds.size.w, bounds.size.h));

  // create GFonts for text layers...
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TYPEFACE_48));

  // time text layer setup...
  text_layer_set_background_color(s_time_layer, GColorClear);        // set clear background color
  text_layer_set_text_color(s_time_layer, GColorOrange);              // set text color
  text_layer_set_text(s_time_layer, "00:00");                        // set default text to 00:00
  text_layer_set_font(s_time_layer, s_time_font);                    // set font and size of 48pt
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter); // center align text

  // add clock text layers to window layer...
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // load album cover bitmap
  load_background();
}

static void window_unload(Window *window)
{
  // get rid of bitmap layer
  bitmap_layer_destroy(bitmap_layer);

  // destroy bitmap
  gbitmap_destroy(cover);

  // destroy clock text layer
  text_layer_destroy(s_time_layer);

  // unload GFont
  fonts_unload_custom_font(s_time_font);
}

static void init(void)
{
  window = window_create(); // create a new window. woohoo!
  window_set_window_handlers(window, (WindowHandlers){
                                         .load = window_load,
                                         .unload = window_unload,
                                     }); // set functions for window handlers

  window_stack_push(window, true); // push window

  update_time(); // register with TickTimerService

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); // set tick_handler function as time handler (start keeping time, essentially. kinda important on a watch.)

  // register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers){
      .pebble_app_connection_handler = bluetooth_callback});
}

static void deinit(void)
{
  window_destroy(window); // literally obliterate the window
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}