#include "main.h"

// Initialize the default settings
static void prv_default_settings() {
    settings.Analog = true;
    
    strcpy(settings.dateFormat, "%b %e");
    
    settings.color_TIME = GColorWhite;
    settings.color_DATE = GColorWhite;
    settings.color_TEMP = GColorWhite;
    settings.color_STEPS = GColorWhite;
    settings.color_BATT = GColorWhite;

    settings.color_NIGHT = GColorBlack;
    settings.color_GROUND_DAY = GColorOxfordBlue;  
    settings.color_GROUND_NIGHT = GColorDukeBlue;  
    settings.color_DAY = GColorVividCerulean;
    settings.color_SUN = GColorWhite;
    settings.color_MOON = GColorWhite;
    settings.color_MERCURY = GColorVividCerulean;
    settings.color_VENUS = GColorRajah;
    settings.color_MARS = GColorRed;
    settings.color_JUPITER = GColorFashionMagenta;
    settings.color_SATURN = GColorMalachite;

    settings.color_CLOUDY = GColorLightGray;
    settings.color_RAINY = GColorJaegerGreen; 
    settings.color_SNOWY = GColorPictonBlue;
    settings.color_PARTLY = GColorDarkGray;
    
}

static void prv_update_display() {
    // hide and display things according to settings
    Layer* winrl = window_get_root_layer(s_main_window);
    Layer* window_layer = window_get_root_layer(s_main_window);
    GRect window_bounds = layer_get_unobstructed_bounds(window_layer);
    int w = window_bounds.size.w;
    int h = window_bounds.size.h;


    if (settings.Analog) {
        layer_remove_from_parent(text_layer_get_layer(s_time_layer));
        layer_remove_from_parent(s_battery_layer);
        layer_add_child(winrl, s_analog_layer);

        layer_set_frame(text_layer_get_layer(s_date_layer), GRect(20, h-38, w, 30));
        text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
    } else {
        layer_add_child(winrl, text_layer_get_layer(s_time_layer));  
        layer_add_child(winrl, s_battery_layer);  
        layer_remove_from_parent(s_analog_layer);

        layer_set_frame(text_layer_get_layer(s_date_layer), GRect(0, h/2+15, w, 30));
        text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    }

    layer_mark_dirty(s_battery_layer);
    layer_mark_dirty(s_forecast_layer);
    layer_mark_dirty(s_analog_layer);
    layer_mark_dirty(s_space_layer);
    layer_mark_dirty(s_sky_layer);
    
    update_time();
}

// Read settings from persistent storage
static void prv_load_settings() {
    prv_default_settings();
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
    persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    prv_update_display();
}


static GPoint hours(int hour, int w, int h, int b)
{
    w -= b*2;
    h -= b*2;
    switch (hour % 24) {
        case 0:
        return GPoint(3*w/6+b,h+b); // 0
        case 1:
        return GPoint(2*w/6+b,h+b); // 1
        case 2:
        return GPoint(1*w/6+b,h+b); // 2

        case 3:
        return GPoint(b,6*h/6+b); // 3

        case 4:
        return GPoint(b,5*h/6+b); // 4
        case 5:
        return GPoint(b,4*h/6+b); // 5
        case 6:
        return GPoint(b,3*h/6+b); // 6
        case 7:
        return GPoint(b,2*h/6+b); // 7
        case 8:
        return GPoint(b,1*h/6+b); // 8

        case 9:
        return GPoint(b,b); // 9

        case 10:
        return GPoint(1*w/6+b,b); // 10
        case 11:
        return GPoint(2*w/6+b,b); // 11
        case 12:
        return GPoint(3*w/6+b,b); // 12
        case 13:
        return GPoint(4*w/6+b,b); // 13
        case 14:
        return GPoint(5*w/6+b,b); // 14

        case 15:
        return GPoint(w+b,b); // 15

        case 16:
        return GPoint(w+b,1*h/6+b); // 16 
        case 17:
        return GPoint(w+b,2*h/6+b); // 17
        case 18:
        return GPoint(w+b,3*h/6+b); // 18
        case 19:
        return GPoint(w+b,4*h/6+b); // 19
        case 20:
        return GPoint(w+b,5*h/6+b); // 20

        case 21:
        return GPoint(w+b,h+b); // 21

        case 22:
        return GPoint(5*w/6+b,h+b); // 22
        default:
        return GPoint(4*w/6+b,h+b); // 23
    }
}


// Triggered when bluetooth connects/disconnects
static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

    if(!connected) {
        vibes_double_pulse();
    }
}


// Triggered when the battery has changed level
static void battery_callback(BatteryChargeState state) {
    // Record the new battery level
    s_battery_level = state.charge_percent;
    s_charging = state.is_charging; 

    // Update meter
    layer_mark_dirty(s_battery_layer);
    layer_mark_dirty(s_analog_layer);
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
                int steps = (int)health_service_sum_today(metric);

                if (steps > 999) {
                    snprintf(steps_str, 12, "%d,%03d", steps/1000, steps%1000);            
                } else {
                    snprintf(steps_str, 12, "%d", steps);
                }
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

static void send_message() {

    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
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

    if (settings.Analog) layer_mark_dirty(s_analog_layer);

    // Show the date
    static char date_buffer[16];
    strftime(date_buffer, sizeof(date_buffer), settings.dateFormat, tick_time);
    text_layer_set_text(s_date_layer, date_buffer);

    if(tick_time->tm_min % 15 == 0) {
        send_message();
    }
}

// battery layer update proc
static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // Find the width of the bar
    int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);

    // Draw the background

    graphics_context_set_stroke_color(ctx, s_charging ? GColorMediumSpringGreen : settings.color_BATT);
    graphics_draw_rect (ctx, GRect(0, 0, bounds.size.w, bounds.size.h));

    // Draw the bar
    graphics_context_set_fill_color(ctx, s_charging ? GColorMediumSpringGreen : settings.color_BATT);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), GCornerNone , 0);
}

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

// Draw the 24 hour labels
static void label_update_proc(Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_unobstructed_bounds(layer);

    for (int i = 0; i < 24; i+=1) {
        //GPoint p = hours(i, bounds.size.w, bounds.size.h, 5);
        GPoint p1 = hours(i, bounds.size.w, bounds.size.h, 17);
        GPoint p2 = hours(i, bounds.size.w, bounds.size.h, 0);

        char hourStr[3];// = "12";
        snprintf(hourStr, sizeof(hourStr), "%d", i);

        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_draw_line(ctx, p1, p2);
    }
}

// Draw forecast ring
static void forecast_update_proc(Layer* layer, GContext* ctx) {
    GRect fcst_bounds = layer_get_unobstructed_bounds(layer);


    int b = 9;

    int w = fcst_bounds.size.w;
    int h = fcst_bounds.size.h;

    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    int hour = tick_time->tm_hour;

    for (int i = 0; i < 24; i++) {
        GPoint p1 = hours(i + hour,w,h,b);
        GPoint p2 = hours(i + hour + 1,w,h,b);
        //c, r, s, p, _, ? = cloudy, rain, snow, partly cloudy, clear, unknown
        switch (forecast_str[i]) {
            case 'c':
            graphics_context_set_stroke_width(ctx, 3);
            graphics_context_set_stroke_color(ctx, settings.color_CLOUDY);
            break;
            case 'r':
            graphics_context_set_stroke_width(ctx, 4);
            graphics_context_set_stroke_color(ctx, settings.color_RAINY);
            break;
            case 's':
            graphics_context_set_stroke_width(ctx, 4);
            graphics_context_set_stroke_color(ctx, settings.color_SNOWY);
            break;
            case 'p':
            graphics_context_set_stroke_width(ctx, 2);
            graphics_context_set_stroke_color(ctx, settings.color_PARTLY);
            break;
            case '_':
            continue; // don't draw clear segments!
            default:
            graphics_context_set_stroke_color(ctx, GColorBlack);

        }
        graphics_draw_line(ctx, p1, p2);
    }
}

// Analog hands drawing
static GPoint rayFrom(int tri, int radius) {
    GPoint ray = {center.x + sin_lookup(tri)*radius / TRIG_MAX_RATIO , center.y - cos_lookup(tri)*radius / TRIG_MAX_RATIO };
    return ray;
}

static void analog_update_proc(Layer *layer, GContext *ctx) {

    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    int minArmStart = (center.x - 25) * (100-s_battery_level) / 100 + 5;
    int hourArmStart = (center.x - 45) * (100-s_battery_level) / 100 + 5;

    int minArmEnd = center.x - 20;// - minArmStart;
    int hourArmEnd = center.x - 40;// - minArmStart;


    int hourHand = (tick_time->tm_hour * 60 + tick_time->tm_min) / 2; // 0 to 1440 = 0h tp 24h
    int hourAngle = DEG_TO_TRIGANGLE(hourHand);

    int minHand = (tick_time->tm_min * 6);
    int minAngle = DEG_TO_TRIGANGLE(minHand);

    // Draw correct clock markings
    {
        for (int i = 0; i < 12; i++) {
            GPoint pt1 = rayFrom(DEG_TO_TRIGANGLE(i * 30), center.x - 25);
            GPoint pt2 = rayFrom(DEG_TO_TRIGANGLE(i * 30), center.x - 22);
            bool big = i%3==0;

            graphics_context_set_stroke_color(ctx, GColorBlack);
            graphics_context_set_stroke_width(ctx, 2);
            graphics_draw_line(ctx, pt1, pt2);
            graphics_context_set_stroke_color(ctx, big?GColorWhite:GColorLightGray);
            graphics_context_set_stroke_width(ctx, 1);
            graphics_draw_line(ctx, pt1, pt2);
        }
    }

    // Draw hub shadow
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, 3);

    // Draw minute shadow
    {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, center, rayFrom(minAngle, minArmStart));
        graphics_context_set_stroke_width(ctx, 7);
        graphics_draw_line(ctx, rayFrom(minAngle, minArmStart), rayFrom(minAngle, minArmEnd));
    }

    // Draw minute hand
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_line(ctx, center, rayFrom(minAngle, minArmStart));
        graphics_context_set_stroke_width(ctx, 5);
        graphics_draw_line(ctx, rayFrom(minAngle, minArmStart), rayFrom(minAngle, minArmEnd));  
    }

    // Draw hour shadow    
    {
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, center, rayFrom(hourAngle, hourArmStart));
        graphics_context_set_stroke_width(ctx, 9);
        graphics_draw_line(ctx, rayFrom(hourAngle, hourArmStart), rayFrom(hourAngle, hourArmEnd));
    }

    // Draw hour hand    
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_line(ctx, center, rayFrom(hourAngle, hourArmStart));
        graphics_context_set_stroke_width(ctx, 7);
        graphics_draw_line(ctx, rayFrom(hourAngle, hourArmStart), rayFrom(hourAngle, hourArmEnd));
    }

    // Draw hub
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, center, 2);

}


// Space drawing
static void sky_update_proc(Layer *layer, GContext *ctx) {

    // Draw background
    bool daytime = !(s_sun > s_sunset && s_sun < s_sunrise);

    for (int a = 0; a < 60; a++) {    

        if (!s_space_ready) {
            return; //graphics_context_set_fill_color(ctx, GColorRed);
        } else {
            if (a > s_sunset && a < s_sunrise) {
                graphics_context_set_fill_color(ctx, daytime ? settings.color_GROUND_DAY : settings.color_GROUND_NIGHT);
            } else {
                graphics_context_set_fill_color(ctx, daytime ? settings.color_DAY : settings.color_NIGHT);
            }
        }
        GPoint point = coord_for_light(a);
        GRect rect = {GPoint(point.x - 5, point.y - 5), GSize(10,10)};
        graphics_fill_rect (ctx, rect, 0, GCornerNone);
    }
}

static void space_update_proc(Layer *layer, GContext *ctx) {

    // Draw objects
    if (!s_space_ready) return;

    // sun
    graphics_context_set_fill_color(ctx, settings.color_SUN);
    graphics_context_set_stroke_color(ctx, settings.color_SUN);
    graphics_context_set_stroke_width(ctx, 3);

    GPoint sunpoint = coord_for_light(s_sun);
    graphics_fill_circle(ctx, sunpoint, 8);

    for (int i = 0; i < 8; i++) {
        int t = i * 0x2000;
        GPoint ray = {sunpoint.x + sin_lookup(t)*10 / TRIG_MAX_RATIO , sunpoint.y + cos_lookup(t)*10 / TRIG_MAX_RATIO };
        graphics_draw_line(ctx, sunpoint, ray);
    }

    graphics_context_set_fill_color(ctx, settings.color_MOON);
    graphics_fill_circle(ctx, coord_for_light(s_moon), 6);


    graphics_context_set_fill_color(ctx, settings.color_VENUS);
    graphics_fill_circle(ctx, coord_for_light(s_venus), 5);

    graphics_context_set_fill_color(ctx, settings.color_MARS);
    graphics_fill_circle(ctx, coord_for_light(s_mars), 4);

    graphics_context_set_fill_color(ctx, settings.color_JUPITER);
    graphics_fill_circle(ctx, coord_for_light(s_jupiter), 5);

    graphics_context_set_fill_color(ctx, settings.color_SATURN);
    graphics_fill_circle(ctx, coord_for_light(s_saturn), 5);

    graphics_context_set_fill_color(ctx, settings.color_MERCURY);
    graphics_fill_circle(ctx, coord_for_light(s_mercury), 3);
}

// tick has occurred 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();

}

static bool loadColor(const DictionaryIterator* iter, const uint32_t key, GColor* field)
{
    Tuple *cfg_color = dict_find(iter, key);
    if (cfg_color) {
        GColor old = *field;
        *field = GColorFromHEX(cfg_color->value->int32); 
        return !gcolor_equal(*field, old);
    }
    return false;
}

// appmessage
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received message");

    // Check for settings tuples
    bool changedSettings = false;

    Tuple *cfg_analog_tuple = dict_find(iterator, MESSAGE_KEY_CFG_ANALOG);
    if (cfg_analog_tuple) {
        changedSettings |= (settings.Analog != (cfg_analog_tuple->value->int32 == 1));
        settings.Analog = cfg_analog_tuple->value->int32 == 1;
    }
    
    Tuple *cfg_dateformat_tuple = dict_find(iterator, MESSAGE_KEY_CFG_DATE_FORMAT);
    if (cfg_dateformat_tuple) {
        strcpy(settings.dateFormat, cfg_dateformat_tuple->value->cstring);
        changedSettings = true;
        APP_LOG(APP_LOG_LEVEL_DEBUG, "New date format: %s ", steps_str);
    }
    
    
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_TIME, &settings.color_TIME);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_DATE, &settings.color_DATE);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_TEMP, &settings.color_TEMP);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_STEPS, &settings.color_STEPS);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_BATT, &settings.color_BATT);

    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_NIGHT, &settings.color_NIGHT);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_GROUND_DAY, &settings.color_GROUND_DAY);  
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_GROUND_NIGHT, &settings.color_GROUND_NIGHT);  
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_DAY, &settings.color_DAY);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_SUN, &settings.color_SUN);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_MOON, &settings.color_MOON);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_MERCURY, &settings.color_MERCURY);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_VENUS, &settings.color_VENUS);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_MARS, &settings.color_MARS);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_JUPITER, &settings.color_JUPITER);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_SATURN, &settings.color_SATURN);

    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_CLOUDY, &settings.color_CLOUDY);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_RAINY, &settings.color_RAINY); 
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_SNOWY, &settings.color_SNOWY);
    changedSettings |= loadColor(iterator, MESSAGE_KEY_COLOR_PARTLY, &settings.color_PARTLY);
    
    // TODO remaining settings

    
    if (changedSettings) {
        prv_save_settings();
        send_message();
    }


    // Weather data
    static char temperature_buffer[8];

    // Read tuples for data
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    Tuple *forecast_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST);

    // If all data is available, use it
    if(temp_tuple) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received weather data");

        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d˚", (int)temp_tuple->value->int32); // TODO units
        snprintf(forecast_str, sizeof(forecast_str), "%s", forecast_tuple->value->cstring);

        text_layer_set_text(s_temp_layer, temperature_buffer);

        
        char* conditions = conditions_tuple->value->cstring;
        for (int i = 0; i < 10; i++) {
            if (strcmp(conditions, condition_icons[i]) == 0) {
                bitmap_layer_set_bitmap(s_conditions_layer, s_condition_icon_bitmap[i]);
            }
        }
        
        layer_mark_dirty(text_layer_get_layer(s_temp_layer));
        layer_mark_dirty(bitmap_layer_get_layer(s_conditions_layer));

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
        layer_mark_dirty(s_sky_layer);
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

    //GRect cityRect = GRect(20, 15, 100, 25);
    GRect tempRect = GRect(w - 50 - 18, 15, 50, 20);
    GRect timeRect = GRect(0, h/2-30, w, 50);
    GRect stepRect = GRect(20, h-38, w - 40, 20);

    GRect conditionRect = GRect(18,18, 26,26);    
    GRect btRect = GRect(w/2-10, 20, 20, 20);
    GRect batteryRect = GRect(23, h-28, 40, 5);

    GRect dateRect;
    GTextAlignment dateAlign;

    if (settings.Analog) {
        dateRect = GRect(20, h-38, w, 30);
        dateAlign = GTextAlignmentLeft;
    } else {
        dateRect = GRect(0, h/2+15, w, 30);
        dateAlign = GTextAlignmentCenter;
    }

    s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
    s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

    s_steps_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);


    // Conditions layer
    s_condition_icon_bitmap[0] = gbitmap_create_with_resource(RESOURCE_ID_COND_CLEAR);
    s_condition_icon_bitmap[1] = gbitmap_create_with_resource(RESOURCE_ID_COND_CLEAR_NIGHT);
    s_condition_icon_bitmap[2] = gbitmap_create_with_resource(RESOURCE_ID_COND_RAIN);
    s_condition_icon_bitmap[3] = gbitmap_create_with_resource(RESOURCE_ID_COND_SNOW);
    s_condition_icon_bitmap[4] = gbitmap_create_with_resource(RESOURCE_ID_COND_SLEET);
    s_condition_icon_bitmap[5] = gbitmap_create_with_resource(RESOURCE_ID_COND_WIND);
    s_condition_icon_bitmap[6] = gbitmap_create_with_resource(RESOURCE_ID_COND_FOG);
    s_condition_icon_bitmap[7] = gbitmap_create_with_resource(RESOURCE_ID_COND_CLOUDY);
    s_condition_icon_bitmap[8] = gbitmap_create_with_resource(RESOURCE_ID_COND_PC);
    s_condition_icon_bitmap[9] = gbitmap_create_with_resource(RESOURCE_ID_COND_PC_NIGHT);

    s_conditions_layer = bitmap_layer_create(conditionRect);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_conditions_layer));

    // Weather forecast layer
    s_forecast_layer = layer_create(GRect(5, 5, w - 10, h - 10));
    layer_set_update_proc(s_forecast_layer, forecast_update_proc);
    layer_add_child(window_get_root_layer(window), s_forecast_layer);

    // Create the Bluetooth icon GBitmap
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

    // Create the BitmapLayer to display the GBitmap
    s_bt_icon_layer = bitmap_layer_create(btRect);
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

    // Create sky layer
    s_sky_layer = layer_create(window_bounds);
    layer_set_update_proc(s_sky_layer, sky_update_proc);
    layer_add_child(window_get_root_layer(window), s_sky_layer);

    // Create 24 hour layer
    s_24hour_layer = layer_create(window_bounds);
    layer_set_update_proc(s_24hour_layer, label_update_proc);
    layer_add_child(window_get_root_layer(window), s_24hour_layer);

    // Create space layer
    s_space_layer = layer_create(window_bounds);
    layer_set_update_proc(s_space_layer, space_update_proc);
    layer_add_child(window_get_root_layer(window), s_space_layer);

    // Temp layer
    s_temp_layer = text_layer_create(tempRect);
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, settings.color_TEMP);
    text_layer_set_text(s_temp_layer, "");
    text_layer_set_font(s_temp_layer, s_date_font);
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));

    // Create time TextLayer
    s_time_layer = text_layer_create(timeRect);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, settings.color_TIME);
    text_layer_set_text(s_time_layer, "--:--");
    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);


    // Create date TextLayer
    s_date_layer = text_layer_create(dateRect);
    text_layer_set_text_color(s_date_layer, settings.color_DATE);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_alignment(s_date_layer, dateAlign);
    text_layer_set_text(s_date_layer, "");
    text_layer_set_font(s_date_layer, s_date_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

    // Create battery meter Layer
    s_battery_layer = layer_create(batteryRect);
    layer_set_update_proc(s_battery_layer, battery_update_proc);

    // Steps layer
    s_steps_layer = text_layer_create(stepRect);
    text_layer_set_background_color(s_steps_layer, GColorClear);
    text_layer_set_text_color(s_steps_layer, settings.color_STEPS);
    text_layer_set_text(s_steps_layer, "");
    text_layer_set_font(s_steps_layer, s_steps_font);
    text_layer_set_text_alignment(s_steps_layer, GTextAlignmentRight);
    #if defined(PBL_HEALTH)
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_steps_layer));
    #endif

    // Analog hands layer
    s_analog_layer = layer_create(window_bounds);
    layer_set_update_proc(s_analog_layer, analog_update_proc);

    // Things that are different if you're analog vs digital
    if (!settings.Analog) layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    if (!settings.Analog) layer_add_child(window_get_root_layer(window), s_battery_layer);
    if (settings.Analog) layer_add_child(window_get_root_layer(window), s_analog_layer);

    // Initialize the display
    update_time();
    battery_callback(battery_state_service_peek());
    bluetooth_callback(connection_service_peek_pebble_app_connection());

}

static void main_window_unload(Window *window) {

    gbitmap_destroy(s_bt_icon_bitmap);
    bitmap_layer_destroy(s_bt_icon_layer);

    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_steps_layer);

    text_layer_destroy(s_temp_layer);

    layer_destroy(s_battery_layer);
    layer_destroy(s_space_layer);
    layer_destroy(s_sky_layer);
    layer_destroy(s_analog_layer);
    layer_destroy(s_forecast_layer);
}

static void init() {
    prv_load_settings();

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

    const int inbox_size = 1024;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);

    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });

}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
