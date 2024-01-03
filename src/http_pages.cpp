#include "http_pages.h"
#include <Arduino.h>

const char* httpDefaultCss PROGMEM = R"rawliteral(
@import url(https://fonts.googleapis.com/css?family=Oswald|Roboto);
html {
  margin: 0;
  padding: 0;
}

body {
  font-family: Roboto, sans-serif;
  color: #515a6e;
  background-color: #d5eafc;
  width: 100%;
  margin: 0;
  padding: 0;
  overflow: hidden;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  display: -webkit-box;
  display: -ms-flexbox;
  display: flex;
  -webkit-box-align: center;
  -ms-flex-align: center;
  align-items: center;
  -webkit-box-pack: center;
  -ms-flex-pack: center;
  justify-content: center;
  overflow:visible;
}

a {
  cursor: pointer;
  color: #eee;
  text-decoration: none;
  -moz-transition: all 0.3s;
  -o-transition: all 0.3s;
  -webkit-transition: all 0.3s;
  transition: all 0.3s;
}
a:hover {
  color: #ff5050;
}

.header {
    height: 100px;
    width: 100%;
    float: left;
    position: absolute;
    top: 0;
    text-align: center;
    background-color: #000000;
}

h1 {
    color: #ffffff;
}

.container {
  text-align: center;
  position: relative;
  background-color: #fafafe;
  border-radius: 10px;
  margin: 140px 0 0 0;
  padding: 25px 20px 10px;
  box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
  width: 500px;
  box-sizing: border-box;
}

.container_control {
    text-align: center;
    position: relative;
    background-color: #fafafe;
    border-radius: 10px;
    margin: 140px 0 0 0;
    padding: 25px 20px 10px;
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
    width: 320px;
    box-sizing: border-box;
  }

.container:before, .container_control::before {
  content: "";
  position: absolute;
  left: 0;
  bottom: 0;
  right: 0;
  height: 60%;
  background-color: #fafafe;
  border-radius: 10px;
  z-index: 2;
}

.card {
  position: relative;
  z-index: 2;
}

.card_title {
  font-size: 24px;
  margin: 0;
}

.card_title-info {
  font-size: 14px;
  margin: 7px 0 10px;
  font-weight: bold;
}

.module_title-info {
    font-size: 14px;
    margin: 7px 0 10px;
    color: #fff;
}

div.config {
    text-align: right;
}

div.version {
    text-align: right;
    font-size: 10px;
}

button {
  border-radius: 4px;
  border: none;
  outline: none;
  padding: 0 15px;
  font-size: 18px;
  line-height: 36px;
  font-weight: 500;
  margin: 10px 10px 10px;
  color: #fff;
  background: linear-gradient(#ff6100, #ff5050);
  box-shadow: 0 2px 12px -3px #ff5050;
  -moz-animation: btn 6s 3s infinite ease-in-out;
  -webkit-animation: btn 6s 3s infinite ease-in-out;
  animation: btn 6s 3s infinite ease-in-out;
  opacity: 0.9;
  -moz-transition: all 0.3s;
  -o-transition: all 0.3s;
  -webkit-transition: all 0.3s;
  transition: all 0.3s;
}
button.movement {
    width: 100px;
    height: 50px;
}
button:hover {
  opacity: 1;
  box-shadow: 0 2px 2px -3px #ff5050;
}

.card_info {
  font-size: 14px;
}

.input {
  display: flex;
  flex-direction: column-reverse;
  position: relative;
  padding-top: 10px;
}

.input + .input {
  margin-top: 25px;
}

.input_label {
  color: #8597a3;
  position: absolute;
  top: 20px;
  -moz-transition: all 0.3s;
  -o-transition: all 0.3s;
  -webkit-transition: all 0.3s;
  transition: all 0.3s;
}

.input_select_label {
    color: #8597a3;
    position: absolute;
    top: -10px;
  }

.input_field, .select_field {
  border: 0;
  padding: 0;
  z-index: 1;
  background-color: transparent;
  border-bottom: 2px solid #eee;
  font: inherit;
  font-size: 14px;
  line-height: 30px;
}
.input_field:focus, .input_field:valid {
  outline: 0;
  border-bottom-color: #665856;
}
.input_field:focus + .input_label, .input_field:valid + .input_label,  .select_field:focus + .select_label, .select_field:valid + .select_label{
  color: #665856;
  -moz-transform: translateY(-25px);
  -ms-transform: translateY(-25px);
  -webkit-transform: translateY(-25px);
  transform: translateY(-25px);
}

.link {
  position: absolute;
  bottom: 20px;
  right: 20px;
  z-index: 3;
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
  <div class="header">
    <h1>Louver control</h1>
    <p class="module_title-info">%MODULE_NAME%</p>
  </div>
  <div class="container_control">
    <div class="card">
      <button class="movement" id="shortUp" onclick="buttonPressed(this);">&#9650;</button>
      <button class="movement" id="shortDown" onclick="buttonPressed(this);">&#9660;</button>
      <br />
      <button class="movement" id="fullOpen" onclick="buttonPressed(this);">&#9650;&#9650;</button>
      <button class="movement" id="fullClose" onclick="buttonPressed(this);">&#9660;&#9660;</button>
      <br />
      <button class="movement" id="fullCloseAndOpenLamellas" onclick="buttonPressed(this);">&#9660;&#9650;</button>
      <br />
      <button class="movement" id="stop" onclick="buttonPressed(this);">Stop</button>
      <p class="card_title-info">
        <div class="version">
         v%VERSION%
        </div>
      </p>
      <div class="config">
        <a href="/settings">&#x03C0</a>
      </div>
    </div>
  </div>
<script>
function buttonPressed(element) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/command?button=" + element.id, true);
  xhr.send();
}
</script>
</body>
</html>)rawliteral";

const char* httpModuleConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">Module Configuration</p>
            <form class="card_form" action="/moduleConfigSave" method="post" id="moduleConfig">
                <div class="input">
                    <input type="text" class="input_field" required name="name" id = "name" value="%MODULE_NAME%"/>
                    <label class="input_label" for="name">Module name</label>
                </div>
                <button class="button" type="submit" form="gpioConfig" value="Submit">Save</button>
            </form>
            <form action="/settings" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                 v%VERSION%
                </div>
            </p>
        </div>
    </div>
</body>
</html>)rawliteral";

const char* httpGpioConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">GPIO Configuration</p>
            <form class="card_form" action="/gpioConfigSave" method="post" id="gpioConfig">
                <div class="input">
                    <input type="text" class="input_field" required name="downGpio" id = "downGpio" value="%KEY_DOWN_GPIO%"/>
                    <label class="input_label" for="downGpio">Key down GPIO</label>
                </div>
                <div class="input">
                    <select class="select_field" id="downInvert" name="downInvert">
                        <option value="0" %SELECTED_DOWN_INVERTED_NO%>Active high</option>
                        <option value="1" %SELECTED_DOWN_INVERTED_YES%>Active low</option>
                    </select>
                    <label class="input_select_label" for="downInvert">Key down GPIO active level</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="upGpio" id = "upGpio" value="%KEY_UP_GPIO%"/>
                    <label class="input_label" for="upGpio">Key up GPIO</label>
                </div>
                <div class="input">
                    <select class="select_field" id="upInvert" name="upInvert">
                        <option value="0" %SELECTED_UP_INVERTED_NO%>Active high</option>
                        <option value="1" %SELECTED_UP_INVERTED_YES%>Active low</option>
                    </select>
                    <label class="input_select_label" for="upInvert">Key up GPIO active level</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="rdownGpio" id = "rdownGpio" value="%RELAY_DOWN_GPIO%"/>
                    <label class="input_label" for="rdownGpio">Relay down GPIO</label>
                </div>
                <div class="input">
                    <select class="select_field" id="rdownInvert" name="rdownInvert">
                        <option value="0" %SELECTED_RDOWN_INVERTED_NO%>Active high</option>
                        <option value="1" %SELECTED_RDOWN_INVERTED_YES%>Active low</option>
                    </select>
                    <label class="input_select_label" for="rdownInvert">Relay down GPIO active level</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="rupGpio" id = "rupGpio" value="%RELAY_UP_GPIO%"/>
                    <label class="input_label" for="rupGpio">Relay up GPIO</label>
                </div>
                <div class="input">
                    <select class="select_field" id="rupInvert" name="rupInvert">
                        <option value="0" %SELECTED_RUP_INVERTED_NO%>Active high</option>
                        <option value="1" %SELECTED_RUP_INVERTED_YES%>Active low</option>
                    </select>
                    <label class="input_select_label" for="rupInvert">Relay up GPIO active level</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="resetGpio" id = "resetGpio" value="%KEY_RESET_GPIO%"/>
                    <label class="input_label" for="resetGpio">Reset key GPIO</label>
                </div>
                <div class="input">
                    <select class="select_field" id="resetInvert" name="resetInvert">
                        <option value="0" %SELECTED_KEY_RESET_INVERTED_NO%>Active high</option>
                        <option value="1" %SELECTED_KEY_RESET_INVERTED_YES%>Active low</option>
                    </select>
                    <label class="input_select_label" for="resetInvert">Reset key GPIO active level</label>
                </div>
                <button class="button" type="submit" form="gpioConfig" value="Submit">Save</button>
            </form>
            <form action="/settings" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                 v%VERSION%
                </div>
            </p>
        <div>
    </div>
 </body>
</html>)rawliteral";

const char* httpTimingConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">Timing Configuration</p>
            <form class="card_form" action="/timingConfigSave" method="post" id="timeConfig">
                <div class="input">
                    <input type="text" class="input_field" required name="timeFullOpen" id = "timeFullOpen" value="%TIME_FULL_OPEN%"/>
                    <label class="input_label" for="timeFullOpen">Full open [s]</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="timeFullClose" id = "timeFullClose" value="%TIME_FULL_CLOSE%"/>
                    <label class="input_label" for="timeFullClose">Full close [s]</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="timeShort" id = "timeShort" value="%TIME_SHORT%"/>
                    <label class="input_label" for="timeShort">Short movement [s]</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="timeOpenLamellas" id = "timeOpenLamellas" value="%TIME_OPEN_LAMELLAS%"/>
                    <label class="input_label" for="timeOpenLamellas">Open lamellas [s]</label>
                </div>
                <button class="button" type="submit" form="timeConfig" value="Submit">Save</button>
            </form>
            <form action="/settings" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                 v%VERSION%
                </div>
            </p>
        </div>
    </div>
</body>
</html>)rawliteral";

const char* httpNetworkConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">MQTT Configuration</p>
            <form class="card_form" action="/networkConfigSave" method="post" id="networkConfig">
                <div class="input">
                    <select class="select_field" id="wifiMode" name="wifiMode">
                        <option value="ap" %SELECTED_WIFI_MODE_AP%>AP</option>
                        <option value="client" %SELECTED_WIFI_MODE_CLIENT%>Client</option>
                    </select>
                    <label class="input_select_label" for="wifiMode">WIFI mode</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="apSSID" id = "apSSID" value="%NETWORK_SSID_AP%"/>
                    <label class="input_label" for="apSSID">AP SSID:</label>
                </div>
                <div class="input">
                    <input type="password" class="input_field" name="apPass" id = "apPass" value="%NETWORK_PASS_AP%"/>
                    <label class="input_label" for="apPass">AP password</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="clientSSID" id = "clientSSID" value="%NETWORK_SSID%"/>
                    <label class="input_label" for="clientSSID">Client SSID:</label>
                </div>
                <div class="input">
                    <input type="password" class="input_field" name="clientPass" id = "clientPass" value="%NETWORK_PASS%"/>
                    <label class="input_label" for="clientPass">Client password</label>
                </div>
                <div class="input">
                    <select class="select_field" id="clientBehavior" name="clientBehavior">
                        <option value="0" %SELECTED_CLIENT_BEHAVIOR_0%>1m in client, 5mins in AP</option>
                        <option value="1" %SELECTED_CLIENT_BEHAVIOR_1%>Permanent client</option>
                    </select>
                    <label class="input_select_label" for="clientBehavior">Client behavior when cannot connect</label>
                </div>
                <button class="button" type="submit" form="networkConfig" value="Submit">Save</button>
            </form>
            <form action="/settings" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                 v%VERSION%
                </div>
            </p>
        </div>
    </div>
</body>
</html>)rawliteral";

const char* httpMqttConfig PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">MQTT Configuration</p>
            <form class="card_form" action="/mqttConfigSave" method="post" id="mqttConfig">
                <div class="input">
                    <input type="text" class="input_field" required name="clientId" id = "clientId" value="%MQTT_CLIENT_ID%"/>
                    <label class="input_label" for="clientId">Client ID</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="brokerIp" id = "brokerIp" value="%MQTT_BROKER_IP%"/>
                    <label class="input_label" for="brokerIp">Broker IP</label>
                </div>
                <div class="input">
                    <input type="text" class="input_field" required name="brokerPort" id = "brokerPort" value="%MQTT_BROKER_PORT%"/>
                    <label class="input_label" for="brokerPort">Broker port</label>
                </div>
                <button class="button" type="submit" form="mqttConfig" value="Submit">Save</button>
            </form>
            <form action="/settings" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                 v%VERSION%
                </div>
            </p>
        </div>
    </div>
</body>
</html>)rawliteral";

const char* httpSettings PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
    <div class="header">
        <h1>Louver control config</h1>
        <p class="module_title-info">%MODULE_NAME%</p>
    </div>
    <div class="container">
        <div class="card">
            <p class="card_title-info">Settings</p>
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
            <form action="/mqttConfig" id="mqttConfig">
                <button class="button" type="submit" form="mqttConfig" value="Submit">MQTT config</button>                                       
            </form>    
            <form action="/update" id="firmwareUpdate">
                <button class="button" type="submit" form="firmwareUpdate" value="Submit">Firmware update (OTA)</button>                                       
            </form>        
            <form action="/" id="back">
                <button class="button" type="submit" form="back" value="Submit">Back to main page</button>                                       
            </form>
            <p class="card_title-info">
                <div class="version">
                    v%VERSION%
                </div>
            </p>
        </div>
    </div>
</body>
</html>)rawliteral";

const char* httpNetworkConfigSaved PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
  <div class="header">
    <h1>Louver control config</h1>
    <p class="module_title-info">%MODULE_NAME%</p>
 </div>
 <div class="container">
  <div class="card">
    <strong>Network config saved - rebooting.</strong>
    <form action="/settings" id="back">
      <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
  </form>
  <p class="card_title-info">
    <div class="version">
     v%VERSION%
    </div>
  </p>
  </div>
 </div>
</body>
</html>)rawliteral";

const char* httpConfigSaved PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Louver control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="default.css">
</head>
<body>
  <div class="header">
    <h1>Louver control config</h1>
    <p class="module_title-info">%MODULE_NAME%</p>
 </div>
 <div class="container">
  <div class="card">
    <strong>Config saved.</strong>
    <form action="/settings" id="back">
      <button class="button" type="submit" form="back" value="Submit">Back to settings</button>                                       
  </form>
  <p class="card_title-info">
    <div class="version">
     v%VERSION%
    </div>
  </p>
  </div>
 </div>
</body>
</html>)rawliteral";

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

const char* getHttpMqttConfig()
{
    return httpMqttConfig;
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

