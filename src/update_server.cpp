#include "update_server.h"
#include <StreamString.h>
#if defined(ESP32) 
#include <Update.h>
#endif
#include "log.h"
#include "module.h"

UpdateServer::UpdateServer() {
    m_server = nullptr;
    m_username = emptyString;
    m_password = emptyString;
    m_authenticated = false;
    m_totalSize = 0;
    m_pageProcessor = nullptr;
}

void UpdateServer::setup(AsyncWebServer *server, const String& path, const String& username, const String& password, const char* html, AwsTemplateProcessor processor) {
    m_server = server;
    m_username = username;
    m_password = password;
    m_html = html;
    m_totalSize = 0;
    m_pageProcessor = processor;

    Log::info("OTA", "Configuring OTA, path=\"%s\"", path.c_str());

    // handler for the /update form page
    this->m_server->on(path.c_str(), HTTP_GET, [&](AsyncWebServerRequest *request){
      Log::debug("OTA", "GET request, /update");
      if(this->m_username != emptyString && this->m_password != emptyString && request->authenticate(this->m_username.c_str(), this->m_password.c_str()))
        return request->requestAuthentication();
        
      if (m_pageProcessor)
        request->send_P(200, PSTR("text/html"), m_html, m_pageProcessor);
    });

    this->m_server->on(String(path + "Progress").c_str(), HTTP_GET, [&](AsyncWebServerRequest *request){
      Log::debug("OTA", "GET request, /updateProgress");
      String json = "{\"bytes\":" + String(m_totalSize) + "}";
      request->send(200, "text/json", json);
    });

    // handler for the /update form POST (once file upload finishes)
    this->m_server->on(path.c_str(), HTTP_POST, [&](AsyncWebServerRequest *request){
      Log::debug("OTA", "POST request, /update");
      if(!this->m_authenticated)
        return request->requestAuthentication();
      
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");        
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");         
      request->send(response);
      Module::reboot();
    },[&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      if(!index) 
      {
        Log::debug("OTA", "File upload request, filename=\"%s\"", filename.c_str());
        this->m_updaterError.clear();

        this->m_authenticated = (this->m_username == emptyString || this->m_password == emptyString || request->authenticate(this->m_username.c_str(), this->m_password.c_str()));
        
        if(!this->m_authenticated) {
          Log::error("OTA", "Unauthenticated Update");
          return;
        }
        else {
          Log::debug("OTA", "Authenticated Update");
        }
#if defined(ESP8266)
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        Log::debug("OTA", "Starting, max sketch size %d bytes", maxSketchSpace);
        m_lastReportedSize = 0;
        m_totalSize = 0;
        Update.runAsync(true); 
        if (!Update.begin(maxSketchSpace, U_FLASH)) 
        {
#else
        Log::debug("OTA", "Starting");
        m_lastReportedSize = 0;
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH))
        {
#endif
          this->_setUpdaterError();
        }
        Log::debug("OTA", "Started");
      }
      
      if(this->m_authenticated && len){
        if(Update.write(data, len) != len){
          this->_setUpdaterError();
        }
        else
        {
          m_totalSize += len;
          if (m_totalSize - m_lastReportedSize >= 10 * 1024)
          {
            m_lastReportedSize = m_totalSize;
            Log::debug("OTA", "%dK bytes written", m_totalSize / 1024);
          }
        }
      } 
      
      if(final){
        if(Update.end(true)){ //true to set the size to the current progress
          Log::info("OTA", "Firmware update success (%d bytes), rebooting", m_totalSize);
        } else {
          this->_setUpdaterError();
        }
      } 
      delay(0);
      return;
   });
}

void UpdateServer::_setUpdaterError() {

  //Update.printError(Serial);
  //this->_updaterError = str.c_str();
}