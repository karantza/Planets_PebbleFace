module.exports = [
    {
        "type": "heading",
        "defaultValue": "Planets Configuration"
    },
    {
        "type": "text",
        "defaultValue": "Digital or analog watchface that shows the visible planets, and a 24 hour weather forecast."
    }, 
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Time"
            },
            {
                "type": "toggle",
                "messageKey": "CFG_ANALOG",
                "defaultValue": true,
                "label": "Analog Face"
            },
            {
                "type": "input",
                "messageKey": "CFG_DATE_FORMAT",
                "defaultValue": "%b %e",
                "label": '<a href="http://strftime.net/">Date Format</a>',
                "attributes": {
                    "placeholder": "%b %e",
                    "limit": 10
                }
            },
            {
                "type": "color",
                "messageKey": "COLOR_TIME",
                "defaultValue": "FFFFFF",
                "label": "Time/Hands",
            },
            {
                "type": "color",
                "messageKey": "COLOR_DATE",
                "defaultValue": "FFFFFF",
                "label": "Date",
            }
        ]
    }, 
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Weather"
            },
            {
                "type": "toggle",
                "messageKey": "CFG_CELSIUS",
                "defaultValue": true,
                "label": "Use Celsius"
            },
            {
                "type": "input",
                "messageKey": "CFG_DARKSKY_KEY",
                "defaultValue": "",
                "label": "Dark Sky API Key",
            },
            {
                "type": "color",
                "messageKey": "COLOR_TEMP",
                "defaultValue": "FFFFFF",
                "label": "Current Temp Color",
            },
            {
                "type": "color",
                "messageKey": "COLOR_CLOUDY",
                "defaultValue": "AAAAAA",
                "label": "Cloudy",
            },
            {
                "type": "color",
                "messageKey": "COLOR_RAINY",
                "defaultValue": "00AA55",
                "label": "Rain",
            },
            {
                "type": "color",
                "messageKey": "COLOR_SNOWY",
                "defaultValue": "55AAFF",
                "label": "Snow",
            },
            {
                "type": "color",
                "messageKey": "COLOR_PARTLY",
                "defaultValue": "555555",
                "label": "Partly Cloudy",
            }
        ]
    }, 
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Astronomy"
            },  
            {
                "type": "color",
                "messageKey": "COLOR_DAY",
                "defaultValue": "00AAFF",
                "label": "Sky (day)",
            },
            {
                "type": "color",
                "messageKey": "COLOR_GROUND_DAY",
                "defaultValue": "000055",
                "label": "Ground (day)",
            },
            {
                "type": "color",
                "messageKey": "COLOR_NIGHT",
                "defaultValue": "000000",
                "label": "Sky (night)",
            },
            {
                "type": "color",
                "messageKey": "COLOR_GROUND_NIGHT",
                "defaultValue": "0000AA",
                "label": "Ground (night)",
            },
            {
                "type": "heading",
                "defaultValue": "Object Colors",
                "size":5
            },
            {
                "type": "color",
                "messageKey": "COLOR_SUN",
                "defaultValue": "FFFFFF",
                "label": "Sun",
            },
            {
                "type": "color",
                "messageKey": "COLOR_MOON",
                "defaultValue": "FFFFFF",
                "label": "Moon",
            },
            {
                "type": "color",
                "messageKey": "COLOR_MERCURY",
                "defaultValue": "00AAFF",
                "label": "Mercury",
            },
            {
                "type": "color",
                "messageKey": "COLOR_VENUS",
                "defaultValue": "FFAA55",
                "label": "Venus",
            },
            {
                "type": "color",
                "messageKey": "COLOR_MARS",
                "defaultValue": "FF0000",
                "label": "Mars",
            },
            {
                "type": "color",
                "messageKey": "COLOR_JUPITER",
                "defaultValue": "FF00AA",
                "label": "Jupiter",
            },
            {
                "type": "color",
                "messageKey": "COLOR_SATURN",
                "defaultValue": "00FF55",
                "label": "Saturn",
            }
        ]
    }, 
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Other"
            },
            {
                "type": "color",
                "messageKey": "COLOR_STEPS",
                "defaultValue": "FFFFFF",
                "label": "Step count",
            },
            {
                "type": "color",
                "messageKey": "COLOR_BATT",
                "defaultValue": "FFFFFF",
                "label": "Battery bar",
            }
        ]
    }, 
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];