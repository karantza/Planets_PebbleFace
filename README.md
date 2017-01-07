# Planets_PebbleFace
A pebble watchface that shows the positions of the visible planets.

Thanks for your interest in this watchface! The main attraction here is the outer track, which represents both a 24 hour clock, as well as the sky itself. The track represents the plane of the ecliptic, with the Sun, Moon, and naked-eye planets traveling around the whole track clockwise, once per day. When the Sun (represented by the large white circle with little rays) reaches the very top, it is 12:00 noon (in your current time zone, not solar noon). When it is at the bottom, it is midnight. The "9 o'clock" position represents a time of 6AM, and the "3 o'clock" position represents 6PM. 

The background of the outer track represents the horizon; the dark blue segment is below the horizon, and the light blue (or black at night) segment is above. This also means that the time when the sun crosses the threshold is sunrise (on the left side) or sunset (on the right side). These might not always line up on each side due to DST, your location within the time zone, etc.

On this track are seven markers:
* The Sun, a large white circle with rays
* The Moon, a large white circle with no rays
* Mercury, small and blue
* Venus, small and yellow
* Mars, small and red
* Jupiter, small and purple
* Saturn, small and green

These markers are placed at their approximate positions in the sky. If you were to stand looking south (in the northern hemisphere), your watch will roughly reflect the sky you see. If planets are to the right of the watch, they'll be in the right part of the southern sky, etc.


The next feature, which is optional, is weather data from Dark Sky. When a Dark Sky API key is provided, an inner track will appear that corresponds to the current 24 hour forecast, aligned with the 24 hours of the outer track, starting at the sun and going clockwise. That is, the conditions in three hours are three segments clockwise of the current Sun position. This lets you see at a glance what the weather conditions will be during the next day, aligned with the convenient reference points of sunrise and sunset.

If weather data is available, it will also display the current conditions in the upper left of the display ([using Climacons](http://adamwhitcroft.com/climacons/)), and temperature in the upper right.

The main time-telling watchface can be toggled between digital and analog. In digital mode, the face displays the date and time, with a battery indicator bar in the lower left. In analog mode, an analog watch is displayed with the date in the lower left, and the battery indicated by the fraction of the minute and hour hand to be bolded. This lets you assess the battery state without adding another complication to an already complicated face :)


The lower right always displays your step count for the day.

I'm working to expose all the colors and settings for every possible element in the configuration page; let me know if you have any comments or suggestions! 

Thanks, 
*~Alex*
