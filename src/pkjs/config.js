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
            }
        ]
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
            }
        ]
    }, 
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];