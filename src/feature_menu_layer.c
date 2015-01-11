#include "pebble.h"
#include <counter_windows.h>

#define NUM_MENU_SECTIONS 3
#define NUM_FIRST_MENU_ITEMS 1
#define NUM_SECOND_MENU_ITEMS 3
#define NUM_THIRD_MENU_ITEMS 1
  
static int minute_array[4] = {0,0,0,0};
static Window *window;

// This is a menu layer
// You have more control than with a simple menu layer
static MenuLayer *menu_layer;

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;

    case 1:
      return NUM_SECOND_MENU_ITEMS;

    case 2:
      return NUM_THIRD_MENU_ITEMS;    
    default:
      return 0;
  }
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  if (section_index == 0 || section_index == 2)
    return 0;
  
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      // menu_cell_basic_header_draw(ctx, cell_layer, "Run Time");
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Set Intervals");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  
  int number = 0;
  
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      number = 0;
    break;

    case 1:
      number = cell_index->row+1;
    break;
    
    case 2:
       // There is title draw for something more simple than a basic menu item
      menu_cell_title_draw(ctx, cell_layer, "Start Run");
    return;
  }
  
  static char total_run_time [10];
  if (minute_array[number]%60 < 10) {
    snprintf(total_run_time, 10, "%d:0%d", (int) minute_array[number]/60, (int) minute_array[number]%60);
  }
  else {
    snprintf(total_run_time, 10, "%d:%d", (int) minute_array[number]/60, (int) minute_array[number]%60);
  }
  // This is a basic menu item with a title and subtitle
  if (number == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "Total Run Time", total_run_time, NULL);
  }
  else {
    static char menu_interval_number_text [15];
    snprintf(menu_interval_number_text, 15, "Interval #%d", number);
    menu_cell_basic_draw(ctx, cell_layer, menu_interval_number_text, total_run_time, NULL);
  }
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
    // Use the row to specify which item will receive the select action
    count_init("Run Time", "Intervals will end when this time is reached.", 0, minute_array);
      break;
    
    case 1: {
      static char interval_number_text [15];
      snprintf(interval_number_text, 15, "Interval #%d", cell_index->row+1);
      count_init(interval_number_text, "in this interval", cell_index->row+1, minute_array);
      break;
      }
  }
}

// This initializes the menu upon window load
void window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

void window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
}

int main(void) {
  window = window_create();

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true /* Animated */);

  app_event_loop();

  window_destroy(window);
}
