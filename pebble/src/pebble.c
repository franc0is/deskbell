#include <pebble.h>

#define COMING_KEY 0
#define BUSY_KEY 1
#define MSG_KEY 2

/* Main window state */
static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static GBitmap *thumb_up_bitmap;
static GBitmap *thumb_down_bitmap;

/* Strings window state */
static void menu_select_callback(int index, void *ctx);
static Window *menu_window;
static SimpleMenuLayer *menu_layer;
static char *coming_menu_strings[] = {
  "B rite there",
  "in a minute",
  "Give me 15"
};
static char *busy_menu_strings[] = {
  "OOO",
  "In a meeting",
  "Get off my lawn"
};

static SimpleMenuItem coming_menu_items[ARRAY_LENGTH(coming_menu_strings)];
static SimpleMenuItem busy_menu_items[ARRAY_LENGTH(busy_menu_strings)];
const SimpleMenuSection menu_sections[] = {
  {.title = "Coming", .items = coming_menu_items, .num_items = ARRAY_LENGTH(coming_menu_strings)},
  {.title = "Busy", .items = busy_menu_items, .num_items = ARRAY_LENGTH(busy_menu_strings)},
};

static void send_string(char *string, bool coming) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet message = TupletCString(MSG_KEY, string);
  Tuplet value = TupletInteger(coming ? COMING_KEY : BUSY_KEY, 1);
  dict_write_tuplet(iter, &message);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

static void send_key(uint32_t key) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletInteger(key, 1);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

static void coming_select_callback(int index, void *ctx) {
  send_string(coming_menu_strings[index], true /* coming */);
}

static void busy_select_callback(int index, void *ctx) {
  send_string(busy_menu_strings[index], false /* coming */);
}

static void menu_window_load(Window *window) {
  // Now we prepare to initialize the simple menu layer
  // We need the bounds to specify the simple menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  for (unsigned int i = 0; i < ARRAY_LENGTH(coming_menu_strings); ++i) {
    coming_menu_items[i] = (SimpleMenuItem) {
      .title = coming_menu_strings[i],
      .callback = coming_select_callback
    };
  }

  for (unsigned int i = 0; i < ARRAY_LENGTH(busy_menu_strings); ++i) {
    busy_menu_items[i] = (SimpleMenuItem) {
      .title = busy_menu_strings[i],
      .callback = busy_select_callback
    };
  }

  // Initialize the simple menu layer
  menu_layer = simple_menu_layer_create(bounds, menu_window, menu_sections,
      ARRAY_LENGTH(menu_sections), NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(menu_layer));
}

static void menu_window_unload(Window *window) {
  simple_menu_layer_destroy(menu_layer);
  window_destroy(window);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  menu_window = window_create();
   window_set_window_handlers(menu_window, (WindowHandlers) {
    .load = menu_window_load,
    .unload = menu_window_unload,
  });
  const bool animated = true;
  window_stack_push(menu_window, animated);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_key(COMING_KEY);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_key(BUSY_KEY);
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

  thumb_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_UP);
  thumb_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_DOWN);
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
