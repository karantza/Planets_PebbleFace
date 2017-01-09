#include <pebble.h>

#define TRI_W 8

// Persistent storage key
#define SETTINGS_KEY 1
#define SETTINGS_VERSION_KEY 2
#define SETTINGS_CURRENT_VERSION 2

// Define our settings struct
typedef struct ClaySettings {
    bool Analog;
        
    // Things we should preserve when rebooting:
    // color settings

    GColor color_TIME;
    GColor color_DATE;
    GColor color_TEMP;
    GColor color_STEPS;
    GColor color_BATT;

    GColor color_NIGHT;
    GColor color_GROUND_DAY;  
    GColor color_GROUND_NIGHT;  
    GColor color_DAY;
    GColor color_SUN;
    GColor color_MOON;
    GColor color_MERCURY;
    GColor color_VENUS;
    GColor color_MARS;
    GColor color_JUPITER;
    GColor color_SATURN;

    GColor color_CLOUDY;
    GColor color_RAINY; 
    GColor color_SNOWY;
    GColor color_PARTLY;

    char dateFormat[11];
    
    // current conditions
    // 24 hour track
    // planet positions
        
} ClaySettings;

// An instance of the struct
static ClaySettings settings;



/* Main */
static Window *s_main_window;

/* Battery */
static Layer *s_battery_layer;
static int s_battery_level;
static bool s_charging;

/* Weather */
static TextLayer *s_temp_layer;;
static char forecast_str[25];
static Layer *s_forecast_layer;
static BitmapLayer *s_conditions_layer;

/* Time and date */
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static Layer *s_analog_layer, *s_24hour_layer;

/* Steps */
static TextLayer *s_steps_layer;
static GFont s_steps_font;
static char steps_str[12];

/* Bluetooth */
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;

/* Space */
static int s_sunset, s_sunrise, s_sun, s_moon, s_mercury, s_venus, s_mars, s_jupiter, s_saturn;
static bool s_space_ready;

static Layer *s_space_layer, *s_sky_layer;
//static GPath *ULp, *URp, *BRp, *BLp;

static char* condition_icons[] = {
    "clear-day", "clear-night", "rain", "snow", "sleet", "wind", "fog", "cloudy", "partly-cloudy-day", "partly-cloudy-night"
};

static GBitmap* s_condition_icon_bitmap[10];


static int upperright, lowerright, lowerleft, upperleft, step;
static GRect bounds;
static GPoint center;


static void prv_default_settings();
static void prv_update_display();
static void prv_load_settings();
static void prv_save_settings();
static GPoint hours(int hour, int w, int h, int b);
static void bluetooth_callback(bool connected);
static void battery_callback(BatteryChargeState state);
static void health_handler(HealthEventType event, void *context);
static void send_message();
static void update_time();
static void battery_update_proc(Layer *layer, GContext *ctx);
static void calculate_perimiter(Layer* layer);
static GPoint coord_for_light(int a);
static void label_update_proc(Layer* layer, GContext* ctx);
static void forecast_update_proc(Layer* layer, GContext* ctx);
static GPoint rayFrom(int tri, int radius);
static void analog_update_proc(Layer *layer, GContext *ctx);
static void sky_update_proc(Layer *layer, GContext *ctx);
static void space_update_proc(Layer *layer, GContext *ctx);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static bool loadColor(const DictionaryIterator* iter, const uint32_t key, GColor* field);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
static void outbox_sent_callback(DictionaryIterator *iterator, void *context);
