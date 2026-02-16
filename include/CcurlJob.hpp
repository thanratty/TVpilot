#pragma once

#include "config.h"

//--

#include <wininet.h>
#include <string>
#include <vector>

#include "curl/curl.h"




class cCurlJob
{
public:
    cCurlJob(const std::string _url) : m_url(_url)
    {
    }

    void Reset()
    {
        xmlErrorCode = 0;
        xmlErrorDomain = 0;
        xmlErrorLine = 0;
        xmlErrorCol = 0;
        xmlErrorMsg.clear();

        m_html_page.clear();
        m_curl_result   = CURLE_FAILED_INIT;
        m_http_response = HTTP_STATUS_DENIED;
    }

    bool downloadShow();

    // Getter/Setter functions

    inline const std::string& Url() const
    {
        return m_url;
    }

    inline void AppendHtml(const char* pdata, size_t length)
    {
        m_html_page += std::string(pdata, length);
    }

    inline const char* HtmlData() const
    {
        return m_html_page.data();
    }

    inline size_t HtmlLength() const
    {
        return m_html_page.length();
    }


public:
    CURLcode            m_curl_result{ CURLE_FAILED_INIT };
    long                m_http_response{ HTTP_STATUS_DENIED };
    //
    int                 xmlErrorCode{ 0 };
    int                 xmlErrorDomain { 0};
    int                 xmlErrorLine{ 0 };
    int                 xmlErrorCol{ 0 };
    std::string         xmlErrorMsg;

private:
    std::string         m_url;
    std::string         m_html_page;

    bool fetchUrl();

};


