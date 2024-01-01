#include "http_pages.h"
#include <Arduino.h>

const char* httpDefaultCss PROGMEM = R"rawliteral(
html {font-family: Arial; display: inline-block; text-align: center;}
h2 {font-size: 2.0rem;}
h3 {font-size: 1.0rem; text-align: left;}
p {font-size: 3.0rem;}
body {max-width: 600px; margin:0px auto; padding-bottom: 25px; background-color: #eeeeee;}
input:checked+.slider {background-color: #2196F3}
input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
.button {
    background-color: #04AA6D; /* Green */
    border: none;
    border-radius: 4px;
    color: white;
    padding: 15px 32px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    transition-duration: 0.4s;
    margin: 10px;
}
.button:hover {
background-color: #1ed492; /* Green */
color: white;
}
div.config {
    text-align: right;
}
div.rect {
    border: 1px solid #888888;
    background-color: #dddddd;
    margin: 10px;
    padding: 5px;
}
div#footer {
    text-align: right;
    font-size: 0.7rem;
}
a {
    color: #8888;
    text-decoration:none;
}
a:hover {
    color: #000;
    text-decoration:none;
}
form#gpioConfig select, form#gpioConfig input, form#networkConfig input, form#networkConfig select, form#timeConfig input, form#moduleConfig input {
    width: 100px;
    height: 25px;
    background-color: #eeeeee;
    margin-bottom: 5px;
    border: 1px solid #000;
}
form#gpioConfig label, form#networkConfig label, form#timeConfig label, form#moduleConfig label {
    font-size: 0.9rem;
    width: 140px;
    text-align: left;
    display:inline-block;
    margin-bottom: 5px;
}
)rawliteral";

const char* httpIndex PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="config">
        <a href="/settings">&#x03C0</a>
    </div>
    <h2>Louver control</h2>
    %MODULE_NAME%
    <div class="rect">
    <button class="button" id="shortUp" onclick="buttonPressed(this)">&#9650;</button>
    <button class="button" id="shortDown" onclick="buttonPressed(this)">&#9660;</button>
    <br />
    <button class="button" id="fullOpen" onclick="buttonPressed(this)">&#9650;&#9650;</button>
    <button class="button" id="fullClose" onclick="buttonPressed(this)">&#9660;&#9660;</button>
    <br />
    <button class="button" id="fullCloseAndOpenLamellas" onclick="buttonPressed(this)">&#9660;&#9650;</button>
    <br />
    <button class="button" id="stop" onclick="buttonPressed(this)">Stop</button>
    </div>
    <div id="footer">%VERSION%</div>
    
<script>
function buttonPressed(element) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/command?button=" + element.id, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

const char* httpModuleConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%
    <h3>Module config</h3>
    <div class="rect">
        <form action="/moduleConfigSave" method="post" id="moduleConfig">
            <label for="name">Module name:</label>
            <input type="text" name="name" id="name" value="%MODULE_NAME%">
            <br />
            <button class="button" type="submit" form="moduleConfig" value="Submit">Save</button>                                       
        </form>
    </div>
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral";

const char* httpGpioConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%
    <h3>GPIO config</h3>
    <div class="rect">
        <form action="/gpioConfigSave" method="post" id="gpioConfig">
            <label for="downGpio">Key down GPIO:</label>
            <input type="text" name="downGpio" id = "downGpio" value="%KEY_DOWN_GPIO%">
            <select id="downInvert" name="downInvert">
                <option value="0" %SELECTED_DOWN_INVERTED_NO%>Active high</option>
                <option value="1" %SELECTED_DOWN_INVERTED_YES%>Active low</option>
            </select>
            <br />
            <label for="upGpio">Key up GPIO:</label>
            <input type="text" name="upGpio" id = "upGpio" value="%KEY_UP_GPIO%">
            <select id="upInvert" name="upInvert">
                <option value="0" %SELECTED_UP_INVERTED_NO%>Active high</option>
                <option value="1" %SELECTED_UP_INVERTED_YES%>Active low</option>
            </select>
            <br />
            <label for="rdownGpio">Relay down GPIO:</label>
            <input type="text" name="rdownGpio" id = "rdownGpio" value="%RELAY_DOWN_GPIO%">
            <select id="rdownInvert" name="rdownInvert">
                <option value="0" %SELECTED_RDOWN_INVERTED_NO%>Active high</option>
                <option value="1" %SELECTED_RDOWN_INVERTED_YES%>Active low</option>
            </select>
            <br />            
            <label for="rupGpio">Relay up GPIO:</label>
            <input type="text" name="rupGpio" id = "rupGpio" value="%RELAY_UP_GPIO%">
            <select id="rupInvert" name="rupInvert">
                <option value="0" %SELECTED_RUP_INVERTED_NO%>Active high</option>
                <option value="1" %SELECTED_RUP_INVERTED_YES%>Active low</option>
            </select>
            <br />
            <button class="button" type="submit" form="gpioConfig" value="Submit">Save</button>                                       
        </form>
    </div>
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
 </body>
</html>
)rawliteral";

const char* httpTimingConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%
    <h3>Timing config</h3>
    <div class="rect">
        <form action="/timingConfigSave" method="post" id="timeConfig">
            <label for="timeFullOpen">Full open [s]:</label>
            <input type="text" name="timeFullOpen" id="timeFullOpen" value="%TIME_FULL_OPEN%">
            <br />
            <label for="timeFullClose">Full close [s]:</label>
            <input type="text" name="timeFullClose" id="timeFullClose" value="%TIME_FULL_CLOSE%">
            <br />
            <label for="timeShort">Short movement [s]:</label>
            <input type="text" name="timeShort" id="timeShort" value="%TIME_SHORT%">
            <br />
            <label for="timeOpenLamellas">Open lamellas [s]:</label>
            <input type="text" name="timeOpenLamellas" id="timeOpenLamellas" value="%TIME_OPEN_LAMELLAS%">
            <br />
            <button class="button" type="submit" form="timeConfig" value="Submit">Save</button>                                       
        </form>
    </div>
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral";

const char* httpNetworkConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%
    <h3>Network config</h3>
    <div class="rect">
        <form action="/networkConfigSave" method="post" id="networkConfig">
            <label for="wifiMode">WIFI mode:</label>
            <select id="wifiMode" name="wifiMode">
                <option value="AP" %SELECTED_WIFI_MODE_AP%>AP</option>
                <option value="client" %SELECTED_WIFI_MODE_CLIENT%>Client</option>
            </select>
            <br />
            <label for="apSSID">AP SSID:</label>
            <input type="text" name="apSSID" id = "apSSID" value="%NETWORK_SSID_AP%">
            <br />
            <label for="apPass">AP password:</label>
            <input type="password" name="apPass" id="apPass" value="%NETWORK_PASS_AP%">
            <br />
            <label for="clientSSID">Client SSID:</label>
            <input type="text" name="clientSSID" id="clientSSID" value="%NETWORK_SSID%">
            <br />
            <label for="clientPass">Client password:</label>
            <input type="password" name="clientPass" id="clientPass" value="%NETWORK_PASS%">
            <br />
            <label for="mDNSHost">mDNS host:</label>
            <input type="text" name="mDNSHost" id="mDNSHost" value="%NETWORK_MDNS_HOST%">
            <br />
            <button class="button" type="submit" form="networkConfig" value="Submit">Save</button>                                       
        </form>
    </div>
    <br />
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral";  

const char* httpSettings PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%
    <h3>Settings</h3>
    <div class="rect">
        <form action="/moduleConfig" id="moduleConfig">
            <button class="button" type="submit" form="moduleConfig" value="Submit">Module config</button>                                       
        </form>    
        <form action="/timingConfig" id="timingConfig">
            <button class="button" type="submit" form="timingConfig" value="Submit">Timing config</button>                                       
        </form>    
        <form action="/gpioConfig" id="gpioConfig">
            <button class="button" type="submit" form="gpioConfig" value="Submit">GPIO config</button>                                       
        </form>    
        <form action="/networkConfig" id="networkConfig">
            <button class="button" type="submit" form="networkConfig" value="Submit">Network config</button>                                       
        </form>    
        <form action="/update" id="firmwareUpdate">
            <button class="button" type="submit" form="firmwareUpdate" value="Submit">Firmware update (OTA)</button>                                       
        </form>    
    </div>
    <br />
    <form action="/" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to main page</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral";  

const char* httpNetworkConfigSaved PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%<br /><br />
    <strong>Network config saved - rebooting.</strong>
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral";  

const char* httpConfigSaved PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <h2>Louver control config</h2>
    %MODULE_NAME%<br /><br />
    <strong>Config saved.</strong>
    <form action="/settings" id="back">
        <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
    </form>
    <div id="footer">%VERSION%</div>
</body>
</html>
)rawliteral"; 

const char* getHttpDefaultCss()
{
    return httpDefaultCss;
}

const char* getHttpIndex()
{
    return httpIndex;
}

const char* getHttpModuleConfig()
{
    return httpModuleConfig;
}


const char* getHttpGpioConfig()
{
    return httpGpioConfig;
}

const char* getHttpTimingConfig()
{
    return httpTimingConfig;
}

const char* getHttpNetworkConfig()
{
    return httpNetworkConfig;
}

const char* getHttpSettings()
{
    return httpSettings;
}

const char* getHttpNetworkConfigSaved()
{
    return httpNetworkConfigSaved;
}

const char* getHttpConfigSaved()
{
    return httpConfigSaved;
}