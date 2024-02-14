#ifndef ESP_HTTP_UPDATE_SERVER_H
#define ESP_HTTP_UPDATE_SERVER_H

#include <ESPAsyncWebSrv.h>

class UpdateServer {

  public:
    UpdateServer();
    void setup(AsyncWebServer *server, const String& path, const String& username, const String& password, const char* html, AwsTemplateProcessor processor = nullptr);

  private:
    AsyncWebServer *m_server;

    String    m_username;
    String    m_password;
    bool      m_authenticated;
    String    m_updaterError;
    const char* m_html;
    AwsTemplateProcessor m_pageProcessor;
    size_t m_lastReportedSize;
    size_t m_totalSize;

    void _setUpdaterError();
};


#endif