#include <pebble.h>

#define COMING_KEY 0
#define BUSY_KEY 1

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static GBitmap *thumb_up_bitmap;
static GBitmap *thumb_down_bitmap;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletInteger(BUSY_KEY, 1);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletInteger(COMING_KEY, 1);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, thumb_up_bitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, thumb_down_bitmap);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(thumb_up_bitmap);
  gbitmap_destroy(thumb_down_bitmap);
}

static void init(void) {
  app_message_open(32, 32);
  thumb_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_UP);
  thumb_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_DOWN);
  window = window_create();
  // window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
