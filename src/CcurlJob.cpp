
#pragma warning( disable : 26812 )
#include "pch.h"

//--

#include <vector>

#include "curl/curl.h"

#include "common.hpp"

#include "CcurlJob.hpp"






/**
 * Callback for Curl to append data to the received data buffer
 */
STATIC size_t
WriteMemoryCallback( const char*    contents,      // ptr to received data
                     size_t         size,          // size of each element (always 1 according to documentation)
                     size_t         nmemb,         // number of elements
                     void*          userp )        // The cCurlJob this pointer
{

    size_t real_size    = size * nmemb;
    cCurlJob* pCurlJob  = static_cast<cCurlJob*>(userp);

    pCurlJob->AppendHtml(contents, real_size);

    return real_size;
}



/**
 * Encapsulate a call into Curl API to retrieve a URL
 * Any error checks & retries are handled by the caller.
 * 
 * Returns  true or false
 *          m_http_response   set appropriately
 *          m_curl_result     set appropriately
 * 
 * Private function.
 */
bool cCurlJob::fetchUrl()
{
    CURL* curl_handle;
    struct curl_slist* headers = nullptr;

    // If we can't init a simple session, return an init error.
    if ((curl_handle = curl_easy_init()) == nullptr)
    {
        m_curl_result = CURLE_FAILED_INIT;
        return false;
    }


    headers = curl_slist_append(headers, u8"Accept: */*");
    headers = curl_slist_append(headers, u8"Host: epguides.com:443");
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
        
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, -1);        // Needed for multi-threaded use
    curl_easy_setopt(curl_handle, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYSTATUS, false);
    curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, CURLUSESSL_TRY );
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, CURL_REPLY_TIMEOUT);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, u8"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:126.0) Gecko/20100101 Firefox/126.0");

    m_curl_result = curl_easy_perform(curl_handle);
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &m_http_response);

    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(headers);
   
    return (m_curl_result == CURLE_OK);
}





bool cCurlJob::downloadShow()
{
    unsigned attempt_number = 1;
    bool     curl_ok;
    bool     success = false;

    // Download the webpage from the show URL with retries
    do
    {
        // Reset error flags etc & (re)try the download
        Reset();
        curl_ok = fetchUrl();

        // All good? Break out.
        if ((curl_ok) && (m_http_response == HTTP_STATUS_OK))
        {
            success = true;
            break;
        }

        LOG_PRINT(eLogFlags::CURL, L"Curl fail, error %d : HTTp response %d\n", m_curl_result, m_http_response);

        // Delay before retrying if any retries left
        if (attempt_number++ < CURL_MAX_TRIES) {
            Sleep(CURL_RETRY_DELAY);
        }
    } while (attempt_number <= CURL_MAX_TRIES);

    return success;
}