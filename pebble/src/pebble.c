#include <pebble.h>

#define	COMING_KEY	0
#define	BUSY_KEY	1
#define	MSG_KEY		2
#define	QUERY_KEY	3
#define	TIMESTR_KEY	4

// Max number of bell calls logged
#define MAX_CALLS	5

// max buffer sizes for in/out app messages
#define INBOUND_SIZE	512
#define OUTBOUND_SIZE	32

#define REFRESH_RATE    30000 /* ms */

/* Main window state */
static Window *window;
static TextLayer *text_layer;

static TextLayer *stats[MAX_CALLS];
static char *call_text[MAX_CALLS];

static ActionBarLayer *action_bar;
static GBitmap *thumb_up_bitmap;
static GBitmap *thumb_down_bitmap;

/* Strings window state */
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
  "Off my lawn"
};

static SimpleMenuItem coming_menu_items[ARRAY_LENGTH(coming_menu_strings)];
static SimpleMenuItem busy_menu_items[ARRAY_LENGTH(busy_menu_strings)];
const SimpleMenuSection menu_sections[] = {
  {.title = "Coming", .items = coming_menu_items,
       .num_items = ARRAY_LENGTH(coming_menu_strings)},
  {.title = "Busy", .items = busy_menu_items,
       .num_items = ARRAY_LENGTH(busy_menu_strings)},
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
  int incr = 144 / (MAX_CALLS + 1);

  text_layer = text_layer_create((GRect) {
	.origin = { 0, 0 }, 
	.size = { bounds.size.w, incr }
  });

  for (int i = 0; i < MAX_CALLS; i++) {
    call_text[i] = malloc(32);

    stats[i] = text_layer_create((GRect) {
	  .origin = {0, incr * (i+1)},
	  .size = {bounds.size.w, incr}
	});


    text_layer_set_text_alignment(stats[i], GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(stats[i]));
  }

  text_layer_set_text(text_layer, "Pebble Bell rings ...");
  text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  thumb_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_UP);
  thumb_down_bitmap = 
      gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUMB_DOWN);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, thumb_up_bitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, thumb_down_bitmap);

  // pull down available bell ring data
  send_key(QUERY_KEY);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);

  for (int i = 0; i < MAX_CALLS; i++) {
    text_layer_destroy(stats[i]);
    free(call_text[i]);
  }
  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(thumb_up_bitmap);
  gbitmap_destroy(thumb_down_bitmap);
}

#define SEC_PER_HOUR	(60 * 60)
#define SEC_PER_MINUTE	(60)
void received_message(DictionaryIterator *iterator, void *context)
{
  Tuple *t = dict_find(iterator, TIMESTR_KEY);
  char *buf = malloc(INBOUND_SIZE);

  if ((t == NULL) || (buf == NULL)) {
    return;
  }

  memcpy(buf, t->value, t->length);

  char *curr_ring = buf;
  size_t len = strlen(buf);

  // FIXME: Pebble does not report UTC natively but you could query your phone
  time_t cur_time = time(NULL) + 7 * SEC_PER_HOUR;

  // walk through the comma delimited timestamps
  int stat_idx = 0;
  for (size_t i = 0; i < (len + 1) && (stat_idx < MAX_CALLS); i++) {
    if ((buf[i] == ',') || (i == len)) {
      buf[i] = '\0';

      uint32_t t = atoi(curr_ring);
      uint32_t delta = cur_time - t;

      int h = (delta) / SEC_PER_HOUR;
      int m = (delta % SEC_PER_HOUR) / SEC_PER_MINUTE;
      int s = (delta % SEC_PER_HOUR) % SEC_PER_MINUTE;

      snprintf(call_text[stat_idx], 32, "%2dhr %2dm %2ds ago", h, m, s);

      text_layer_set_text(stats[stat_idx], call_text[stat_idx]);

      curr_ring = &buf[i+1];
      stat_idx++;
    }
  }
  
  free(buf);
}

void message_dropped(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "message dropped: %d\n", reason);
}

void tick_handler(void *data)
{
  send_key(QUERY_KEY);
  app_timer_register(REFRESH_RATE, tick_handler, NULL);
}

static void init(void) {
  app_message_open(INBOUND_SIZE, OUTBOUND_SIZE);
  app_message_register_inbox_received(received_message);
  app_message_register_inbox_dropped(message_dropped);
  app_timer_register(REFRESH_RATE, tick_handler, NULL);

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
