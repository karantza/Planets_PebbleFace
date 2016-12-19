var space = require('./space');

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
                        function(e) {
                            console.log('PebbleKit JS ready!');
                        }
                       );

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
                        function(e) {
                            console.log('AppMessage received!');
                            getLocation();
                        }                     
                       );

function locationSuccess(pos) {
    var lat = pos.coords.latitude;
    var lon = pos.coords.longitude;
    
    var date = new Date();
    
    getWeather(lat, lon);
    getSpace(lat, lon, date);
}

function getWeather(lat, lon) {
    
    var apiKey = '89a75653232ead9e9acc0161ccb193a2';    
    var url = 'https://api.darksky.net/forecast/' + apiKey + '/' + lat + ',' + lon + '?units=uk2';
    console.log(url);
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(this.responseText);

        // Temperature in Kelvin requires adjustment
        var temperature = Math.round(json.currently.temperature);
        console.log('Temperature is ' + temperature);

        // Conditions
        var conditions = json.hourly.summary;      
        console.log('Conditions are ' + conditions);

        // Assemble dictionary using our keys
        var dictionary = {
            'TEMPERATURE': temperature,
            'CONDITIONS': conditions
        };

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
                              function(e) {
                                  console.log('Weather info sent to Pebble successfully!');
                              },
                              function(e) {
                                  console.log('Error sending weather info to Pebble!');
                              }
                             );
    } ;   
    xhr.open('GET', url);
    xhr.send();
}

function locationError(err) {
    console.log('Error requesting location!');
}

function getSpace(lat, lon, date) {
    space.calculate(lat, lon, date);
    // Assemble dictionary using our keys
    var dictionary = {
        'SPACE_SUN': space.sun,
        'SPACE_SUNRISE': space.sunrise,
        'SPACE_SUNSET': space.sunset,
        'SPACE_MOON': space.moon,
        'SPACE_MERCURY': space.mercury,
        'SPACE_VENUS': space.venus,
        'SPACE_MARS': space.mars,
        'SPACE_JUPITER': space.jupiter,
        'SPACE_SATURN': space.saturn,
    };

    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
                          function(e) {
                              console.log('Space info sent to Pebble successfully!');
                          },
                          function(e) {
                              console.log('Error sending space info to Pebble!');
                          }
                         );
}

function getLocation() {
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError,
        {timeout: 15000, maximumAge: 60000}
    );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
                        function(e) {
                            console.log('PebbleKit JS ready!');

                            // Get the initial weather
                            getLocation();
                        }
                       );