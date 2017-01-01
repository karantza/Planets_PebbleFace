#include <pebble.h>

#define COLOR_TIME GColorLightGray
#define COLOR_TEXT GColorDarkGray 
#define COLOR_DIVIDER GColorDarkGray
#define COLOR_NIGHT GColorBlack
#define COLOR_GROUND_DAY GColorOxfordBlue  
#define COLOR_GROUND_NIGHT GColorDukeBlue  
#define COLOR_DAY GColorVividCerulean
#define COLOR_SUN GColorWhite
#define COLOR_MOON GColorWhite
#define COLOR_MERCURY GColorVividCerulean
#define COLOR_VENUS GColorRajah
#define COLOR_MARS GColorRed
#define COLOR_JUPITER GColorFashionMagenta
#define COLOR_SATURN GColorMalachite

#define COLOR_CLOUDY GColorLightGray
#define COLOR_RAINY GColorBrightGreen
#define COLOR_SNOWY GColorPictonBlue
#define COLOR_PARTLY GColorDarkGray

#define TRI_W 8

static const GPathInfo UL = { .num_points = 3, .points = (GPoint []) {{0, 0}, {TRI_W, 0}, {0,TRI_W}} };
static const GPathInfo UR = { .num_points = 3, .points = (GPoint []) {{PBL_DISPLAY_WIDTH , 0}, {PBL_DISPLAY_WIDTH - TRI_W, 0}, {PBL_DISPLAY_WIDTH, TRI_W}} };
static const GPathInfo BR = { .num_points = 3, .points = (GPoint []) {{PBL_DISPLAY_WIDTH , PBL_DISPLAY_HEIGHT }, {PBL_DISPLAY_WIDTH - TRI_W, PBL_DISPLAY_HEIGHT }, {PBL_DISPLAY_WIDTH ,PBL_DISPLAY_HEIGHT - TRI_W}} };
static const GPathInfo BL = { .num_points = 3, .points = (GPoint []) {{0, PBL_DISPLAY_HEIGHT }, {TRI_W, PBL_DISPLAY_HEIGHT }, {0,PBL_DISPLAY_HEIGHT - TRI_W}} };

/* Main */
static Window *s_main_window;

/* Battery */
static Layer *s_battery_layer;
static int s_battery_level;
static bool s_charging;

/* Weather */
static TextLayer *s_city_layer, *s_temp_layer, *s_conditions_layer;
static GFont s_city_font, s_conditions_font;
static char forecast_str[25];
static Layer *s_forecast_layer;

/* Time and date */
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static Layer *s_analog_layer;

/* Steps */
static TextLayer *s_steps_layer;
static char steps_str[12];

/* Bluetooth */
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;

/* Space */
static int s_sunset, s_sunrise, s_sun, s_moon, s_mercury, s_venus, s_mars, s_jupiter, s_saturn;
static bool s_space_ready;

static Layer *s_space_layer;
static GPath *ULp, *URp, *BRp, *BLp;
/*
// Triggered when bluetooth connects/disconnects
static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
    
    if(!connected) {
        vibes_double_pulse();
    }
}
*/

// Triggered when the battery has changed level
static void battery_callback(BatteryChargeState state) {
    // Record the new battery level
    s_battery_level = state.charge_percent;
    s_charging = state.is_charging; 
    
    // Update meter
    layer_mark_dirty(s_battery_layer);
}

static void health_handler(HealthEventType event, void *context) {
    // Which type of event occurred?
    switch(event) {
        case HealthEventSignificantUpdate:
        case HealthEventMovementUpdate:
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Health Event");

            HealthMetric metric = HealthMetricStepCount;
            time_t start = time_start_of_today();
            time_t end = time(NULL);
            HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);

            if(mask & HealthServiceAccessibilityMaskAvailable) {                
                snprintf(steps_str, 12, "%d Steps", (int)health_service_sum_today(metric));
                
                text_layer_set_text(s_steps_layer, steps_str);
                //layer_mark_dirty(s_steps_layer);

                APP_LOG(APP_LOG_LEVEL_DEBUG, "steps: %s ", steps_str);
            } else {
                text_layer_set_text(s_steps_layer, "No Step Data");
                APP_LOG(APP_LOG_LEVEL_DEBUG, "steps unavailable");
            }
            
        }
        break;
        default:
        break;
    }
}
// Triggered when the time has changed
static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    // Create a long-lived buffer, and show the time
    static char buffer[] = "00:00";
    if(clock_is_24h_style()) {
        strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
        strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
    text_layer_set_text(s_time_layer, buffer);
    
    // Show the date
    static char date_buffer[16];
    strftime(date_buffer, sizeof(date_buffer), "%a %b %e", tick_time);
    text_layer_set_text(s_date_layer, date_buffer);


    if(tick_time->tm_min % 15 == 0) {
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);

        // Send the message!
        app_message_outbox_send();
    }
}

// battery layer update proc
static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    
    // Find the width of the bar
    int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);
    
    // Draw the background
    
    graphics_context_set_stroke_color(ctx, s_charging ? GColorMediumSpringGreen : COLOR_TEXT);
    graphics_draw_rect (ctx, GRect(0, 0, bounds.size.w, bounds.size.h));
    
    // Draw the bar
    graphics_context_set_fill_color(ctx, s_charging ? GColorMediumSpringGreen : COLOR_TEXT);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), GCornerNone , 0);
}

static int upperright, lowerright, lowerleft, upperleft, step;
static GRect bounds;
static GPoint center;

// perimiter calc
static void calculate_perimiter(Layer* layer) {
    bounds = layer_get_bounds(layer);    
    center = grect_center_point(&bounds);

    int perimiter = (bounds.size.w + bounds.size.h) * 2;
    step = perimiter / 60;
    int topcount = bounds.size.w / step;
    int sidecount = bounds.size.h / step;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Perimiter: %d, step: %d, topcount: %d, sidecount: %d", perimiter, step, topcount, sidecount);

    int halftop = topcount / 2;
    
    upperright = halftop;
    lowerright = halftop + sidecount;
    lowerleft = halftop + sidecount + topcount;
    upperleft = halftop + sidecount + topcount + sidecount;
}

static GPoint coord_for_light(int a) {
    GPoint point;
    if (a < upperright) {
        // top right
        point.x = a * step + center.x + 5;
        point.y = 1;

    } else if (a >= upperright && a < lowerright) {
        // right
        point.x = bounds.size.w - 1;
        point.y = (a - upperright) * step + 9;

    } else if (a >= lowerright && a < lowerleft) {
        // bottom
        point.x = bounds.size.w - (a - lowerright + 1) * (step) + 3;
        point.y = bounds.size.h - 1;

    } else if (a >= lowerleft && a < upperleft) {
        // left
        point.x = 1;
        point.y = bounds.size.h - (a - lowerleft + 1) * step + 1;

    } else {
        // top left
        point.x = (a - upperleft) * step + 7;
        point.y = 1;
    }
    return point;
}

// Draw forecast ring
static void forecast_update_proc(Layer* layer, GContext* ctx) {
    GRect fcst_bounds = layer_get_unobstructed_bounds(layer);

    graphics_context_set_stroke_width(ctx, 4);

    int b = 5;

    int w = fcst_bounds.size.w - 2*b;
    int h = fcst_bounds.size.h - 2*b;
    
    // 12 = W + H
    int step = (w + h) / 12;
    int wCount = w / step;
    int hCount = 12 - wCount;
    
    int wStep = w / wCount;
    int hStep = h / hCount;
    
    GPoint hours[24] = {
        
        GPoint(3*w/6+b,h+b), // 0
        GPoint(2*w/6+b,h+b), // 1
        GPoint(1*w/6+b,h+b), // 2
        
        GPoint(b,6*h/6+b), // 3
        
        GPoint(b,5*h/6+b), // 4
        GPoint(b,4*h/6+b), // 5
        GPoint(b,3*h/6+b), // 6
        GPoint(b,2*h/6+b), // 7
        GPoint(b,1*h/6+b), // 8
        
        GPoint(b,b), // 9
        
        GPoint(1*w/6+b,b), // 10
        GPoint(2*w/6+b,b), // 11
        GPoint(3*w/6+b,b), // 12
        GPoint(4*w/6+b,b), // 13
        GPoint(5*w/6+b,b), // 14
        
        GPoint(w+b,b), // 15
        
        GPoint(w+b,1*h/6+b), // 16 
        GPoint(w+b,2*h/6+b), // 17
        GPoint(w+b,3*h/6+b), // 18
        GPoint(w+b,4*h/6+b), // 19
        GPoint(w+b,5*h/6+b), // 20
        
        GPoint(w+b,h+b), // 21
        
        GPoint(5*w/6+b,h+b), // 22
        GPoint(4*w/6+b,h+b) // 23
    };

    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    int hour = tick_time->tm_hour;
    
    for (int i = 0; i < 24; i++) {
        GPoint* p1 = &hours[(i + hour) % 24];
        GPoint* p2 = &hours[(i + hour + 1) % 24];
        //c, r, s, p, _, ? = cloudy, rain, snow, partly cloudy, clear, unknown
        switch (forecast_str[i]) {
            case 'c':
            graphics_context_set_stroke_color(ctx, COLOR_CLOUDY);
            break;
            case 'r':
            graphics_context_set_stroke_color(ctx, COLOR_RAINY);
            break;
            case 's':
            graphics_context_set_stroke_color(ctx, COLOR_SNOWY);
            break;
            case 'p':
            graphics_context_set_stroke_color(ctx, COLOR_PARTLY);
            break;
            case '_':
            continue; // don't draw clear segments!
            default:
                APP_LOG(APP_LOG_LEVEL_INFO, "Unknown forecast character:");
                APP_LOG(APP_LOG_LEVEL_INFO, forecast_str);
            graphics_context_set_stroke_color(ctx, GColorRed);

        }
        graphics_draw_line(ctx, *p1, *p2);
    }
    
}

// Analog hands drawing

static GPoint rayFrom(int tri, int radius)
{
    GPoint ray = {center.x + sin_lookup(tri)*radius / TRIG_MAX_RATIO , center.y - cos_lookup(tri)*radius / TRIG_MAX_RATIO };
    return ray;
}

static void analog_update_proc(Layer *layer, GContext *ctx) {
 
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    
    int minArmLen = center.x - 10;
    int hourArmLen = center.x - 30;
     
    int hourHand = (tick_time->tm_hour * 60 + tick_time->tm_min) / 2; // 0 to 1440 = 0h tp 24h
    int hourAngle = DEG_TO_TRIGANGLE(hourHand);
    
    int minHand = (tick_time->tm_min * 6);
    int minAngle = DEG_TO_TRIGANGLE(minHand);


    // Draw hour shadow    
    {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, center, rayFrom(hourAngle, 20));
        graphics_context_set_stroke_width(ctx, 9);
        graphics_draw_line(ctx, rayFrom(hourAngle, 20), rayFrom(hourAngle, hourArmLen));
    }
    
    // Draw hub shadow
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, 3);
    
    // Draw hour hand    
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_line(ctx, center, rayFrom(hourAngle, 20));
        graphics_context_set_stroke_width(ctx, 7);
        graphics_draw_line(ctx, rayFrom(hourAngle, 20), rayFrom(hourAngle, hourArmLen));
    }
    
    // Draw minute shadow
    {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, center, rayFrom(minAngle, 20));
        graphics_context_set_stroke_width(ctx, 7);
        graphics_draw_line(ctx, rayFrom(minAngle, 20), rayFrom(minAngle, minArmLen));
    }
    
    // Draw minute hand
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_line(ctx, center, rayFrom(minAngle, 20));
        graphics_context_set_stroke_width(ctx, 5);
        graphics_draw_line(ctx, rayFrom(minAngle, 20), rayFrom(minAngle, minArmLen));
        
    }
    
    // Draw hub
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, center, 2);
    

}


// Space drawing
static void space_update_proc(Layer *layer, GContext *ctx) {

    // Draw background
    bool daytime = !(s_sun > s_sunset && s_sun < s_sunrise);
    
    for (int a = 0; a < 60; a++) {    
        
        if (!s_space_ready) {
            return; //graphics_context_set_fill_color(ctx, GColorRed);
        } else {
            if (a > s_sunset && a < s_sunrise) {
                graphics_context_set_fill_color(ctx, daytime ? COLOR_GROUND_DAY : COLOR_GROUND_NIGHT);
            } else {
                graphics_context_set_fill_color(ctx, daytime ? COLOR_DAY : COLOR_NIGHT);
            }
        }
        GPoint point = coord_for_light(a);
        GRect rect = {GPoint(point.x - 5, point.y - 5), GSize(10,10)};
        graphics_fill_rect (ctx, rect, 0, GCornerNone);
    }
    
    /*
    for (int a = 0; a < 60; a += 5) {  
        GPoint point = coord_for_light(a);
        if (a > s_sunset && a < s_sunrise) {
            graphics_context_set_stroke_color(ctx, daytime ? GColorLightGray : GColorRed);
        } else {
            graphics_context_set_stroke_color(ctx, daytime ? GColorBlack : GColorRed);
        }

        graphics_draw_line(ctx, center, point);
    }

    graphics_context_set_fill_color(ctx, GColorBlack);
    
    GRect centerRect = {GPoint(5,5),GSize(bounds.size.w-10, bounds.size.h-10)};
    graphics_fill_rect(ctx, centerRect, 0, GCornerNone);
    
    */
    
    // Draw corners
    graphics_context_set_fill_color(ctx, GColorBlack);    
    graphics_context_set_stroke_color(ctx, GColorBlack);    
    gpath_draw_filled (ctx, ULp);
    gpath_draw_filled (ctx, URp);
    gpath_draw_filled (ctx, BRp);
    gpath_draw_filled (ctx, BLp);

    gpath_draw_outline (ctx, ULp);
    gpath_draw_outline (ctx, URp);
    gpath_draw_outline (ctx, BRp);
    gpath_draw_outline (ctx, BLp);
    
    // Draw objects
    if (!s_space_ready) return;
    
    // sun
    graphics_context_set_fill_color(ctx, COLOR_SUN);
    graphics_context_set_stroke_color(ctx, COLOR_SUN);
    graphics_context_set_stroke_width(ctx, 3);

    GPoint sunpoint = coord_for_light(s_sun);
    graphics_fill_circle(ctx, sunpoint, 8);

    for (int i = 0; i < 8; i++) {
        int t = i * 0x2000;
        GPoint ray = {sunpoint.x + sin_lookup(t)*10 / TRIG_MAX_RATIO , sunpoint.y + cos_lookup(t)*10 / TRIG_MAX_RATIO };
        graphics_draw_line(ctx, sunpoint, ray);
    }

    graphics_context_set_fill_color(ctx, COLOR_MOON);
    graphics_fill_circle(ctx, coord_for_light(s_moon), 6);

    graphics_context_set_fill_color(ctx, COLOR_MERCURY);
    graphics_fill_circle(ctx, coord_for_light(s_mercury), 3);

    graphics_context_set_fill_color(ctx, COLOR_VENUS);
    graphics_fill_circle(ctx, coord_for_light(s_venus), 5);

    graphics_context_set_fill_color(ctx, COLOR_MARS);
    graphics_fill_circle(ctx, coord_for_light(s_mars), 4);

    graphics_context_set_fill_color(ctx, COLOR_JUPITER);
    graphics_fill_circle(ctx, coord_for_light(s_jupiter), 5);

    graphics_context_set_fill_color(ctx, COLOR_SATURN);
    graphics_fill_circle(ctx, coord_for_light(s_saturn), 5);
    
    // Draw dividers
    GRect d1 = {GPoint(10, 65), GSize(bounds.size.w - 20,2)};
    GRect d2 = {GPoint(10, 132), GSize(bounds.size.w - 20,2)};
    
    graphics_context_set_fill_color(ctx, COLOR_DIVIDER);
 
    graphics_fill_rect (ctx, d1, 0, GCornerNone);
    graphics_fill_rect (ctx, d2, 0, GCornerNone);
}

// tick has occurred 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    
}

// appmessage
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received message");

    static char temperature_buffer[8];
    static char conditions_buffer[64];
    static char city_buffer[20];
    
    // Read tuples for data
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    Tuple *forecast_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST);

    // If all data is available, use it
    if(temp_tuple) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received weather data");

        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d˚", (int)temp_tuple->value->int32); // TODO units
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
        snprintf(forecast_str, sizeof(forecast_str), "%s", forecast_tuple->value->cstring);
        
        text_layer_set_text(s_temp_layer, temperature_buffer);
        text_layer_set_text(s_conditions_layer, conditions_buffer);
    }
    
    Tuple *city_tuple = dict_find(iterator, MESSAGE_KEY_CITY);
    // If all data is available, use it
    if(city_tuple) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received city data");
        snprintf(city_buffer, sizeof(city_buffer), "%s", city_tuple->value->cstring);        
        text_layer_set_text(s_city_layer, city_buffer);
    }
    
    
    // Read tuples for data
    Tuple *sun_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_SUN);
    Tuple *sunrise_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_SUNRISE);
    Tuple *sunset_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_SUNSET);
    Tuple *moon_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_MOON);
    Tuple *mercury_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_MERCURY);
    Tuple *venus_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_VENUS);
    Tuple *mars_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_MARS);
    Tuple *jupiter_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_JUPITER);
    Tuple *saturn_tuple = dict_find(iterator, MESSAGE_KEY_SPACE_SATURN);

    if(sun_tuple) {

        s_sunset = sunset_tuple->value->int32;
        s_sunrise = sunrise_tuple->value->int32;
        s_sun = sun_tuple->value->int32;
        s_moon = moon_tuple->value->int32;
        s_mercury = mercury_tuple->value->int32;
        s_venus = venus_tuple->value->int32;
        s_mars = mars_tuple->value->int32;
        s_jupiter = jupiter_tuple->value->int32;
        s_saturn = saturn_tuple->value->int32;
        s_space_ready = true;
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received space data: %d, %d, %d, %d", s_sun, s_sunrise, s_sunset, s_moon);

        layer_mark_dirty(s_space_layer);
    }
    
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
    
    Layer* window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_unobstructed_bounds(window_layer);

    int w = window_bounds.size.w;
    int h = window_bounds.size.h;
    

    ULp = gpath_create(&UL);
    URp = gpath_create(&UR);
    BRp = gpath_create(&BR);
    BLp = gpath_create(&BL);

    
    // Create GFonts
    //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
    //s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
    s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    
    s_city_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    s_conditions_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
        
    
    // Weather forecast layer
    s_forecast_layer = layer_create(GRect(5, 5, w - 10, h - 10));
    layer_set_update_proc(s_forecast_layer, forecast_update_proc);
    layer_add_child(window_get_root_layer(window), s_forecast_layer);
    
    // Create space layer
    s_space_layer = layer_create(window_bounds);
    layer_set_update_proc(s_space_layer, space_update_proc);
    layer_add_child(window_get_root_layer(window), s_space_layer);
    
    
    // City layer
    s_city_layer = text_layer_create(GRect(15, 10, 100, 20));
    text_layer_set_background_color(s_city_layer, GColorClear);
    text_layer_set_text_color(s_city_layer, COLOR_TEXT);
    text_layer_set_text(s_city_layer, "--");
    text_layer_set_font(s_city_layer, s_city_font);
    text_layer_set_text_alignment(s_city_layer, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_city_layer));

    // Temp layer
    s_temp_layer = text_layer_create(GRect(window_bounds.size.w - 50 - 15, 10, 50, 20));
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, COLOR_TIME);
    text_layer_set_text(s_temp_layer, "--˚");
    text_layer_set_font(s_temp_layer, s_date_font);
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));

    // Conditions layer
    s_conditions_layer = text_layer_create(GRect(10, 31, window_bounds.size.w - 20, 40));
    text_layer_set_background_color(s_conditions_layer, GColorClear);
    text_layer_set_text_color(s_conditions_layer, COLOR_TEXT);
    text_layer_set_text(s_conditions_layer, "Getting Forecast");
    text_layer_set_font(s_conditions_layer, s_conditions_font);
    text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_conditions_layer, GTextOverflowModeTrailingEllipsis );
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_conditions_layer));
    
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(5, 63, 139, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, COLOR_TIME);
    text_layer_set_text(s_time_layer, "--:--");
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    
    // Create date TextLayer
    s_date_layer = text_layer_create(GRect(0, 106, 144, 30));
    text_layer_set_text_color(s_date_layer, COLOR_TIME);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    text_layer_set_text(s_date_layer, "xxx xxx xx");
    text_layer_set_font(s_date_layer, s_date_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

    // Analog hands layer
    s_analog_layer = layer_create(window_bounds);
    layer_set_update_proc(s_analog_layer, analog_update_proc);
    layer_add_child(window_get_root_layer(window), s_analog_layer);
    
    // Create battery meter Layer
    s_battery_layer = layer_create(GRect(15, 145, 50, 5));
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(window_get_root_layer(window), s_battery_layer);
    
    // Steps layer
    s_steps_layer = text_layer_create(GRect(15, 138, window_bounds.size.w - 30, 50));
    text_layer_set_background_color(s_steps_layer, GColorClear);
    text_layer_set_text_color(s_steps_layer, COLOR_TEXT);
    text_layer_set_text(s_steps_layer, "------ Steps");
    text_layer_set_font(s_steps_layer, s_conditions_font);
    text_layer_set_text_alignment(s_steps_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_steps_layer));

/*    
    // Create the Bluetooth icon GBitmap
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
    
    // Create the BitmapLayer to display the GBitmap
    s_bt_icon_layer = bitmap_layer_create(GRect(59, 12, 30, 30));
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
    */
    // Initialize the display
    update_time();

        
    battery_callback(battery_state_service_peek());
    //bluetooth_callback(connection_service_peek_pebble_app_connection());
    
}

static void main_window_unload(Window *window) {
    //fonts_unload_custom_font(s_time_font);
    //fonts_unload_custom_font(s_date_font);
    /*
    gbitmap_destroy(s_bt_icon_bitmap);

    bitmap_layer_destroy(s_bt_icon_layer);
    */
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_steps_layer);
    
    text_layer_destroy(s_city_layer);
    text_layer_destroy(s_temp_layer);
    text_layer_destroy(s_conditions_layer);

    layer_destroy(s_battery_layer);
    layer_destroy(s_space_layer);
    layer_destroy(s_analog_layer);
    layer_destroy(s_forecast_layer);
    
    gpath_destroy(ULp);
    gpath_destroy(URp);
    gpath_destroy(BRp);
    gpath_destroy(BLp);
}
    
static void init() {
    s_space_ready = false;
    
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);
    
    window_set_background_color(s_main_window, GColorBlack);
        
    Layer* window_layer = window_get_root_layer(s_main_window);
    calculate_perimiter(window_layer);
    
    // Register with Event Services
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    
    #if defined(PBL_HEALTH)
    // Attempt to subscribe 
    if(!health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
    health_handler(HealthEventSignificantUpdate, NULL);
    #else
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    #endif
    
    
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
        
    const int inbox_size = 128;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);
/*
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });
    */
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
