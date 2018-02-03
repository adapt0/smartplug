/////////////////////////////////////////////////////////////////////////////
/** @file
Encapsulates an HTTP connection

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "http_connection.h"
#include <algorithm>
#include <cctype>
#include <cstring>

/////////////////////////////////////////////////////////////////////////////
/// start a GET request
/// @param url URL to request (e.g., /index.html)
/// @returns ERR_OK on success (Netconn error code)
err_t HttpConnection::get(const char* url) {
//TODO: handle multiple GET requests
    auto res = tcp_.connect(&addr_, port_);
    if (ERR_OK != res) return res;

    //
    char buf[128];
    snprintf(
        buf, sizeof(buf),
        "GET %s HTTP/1.0\r\n\r\n",
        url
    );
    res = tcp_.write(buf, strlen(buf), NETCONN_COPY);
    if (ERR_OK != res) return res;

    // retrieve & process headers
    return retrieveHeaders_();
}

/////////////////////////////////////////////////////////////////////////////
/// retrieve HTTP headers
err_t HttpConnection::retrieveHeaders_() {
    char headerField[128];
    char headerValue[128];
    size_t headerOfs = 0;
    int statusCode = 0;

    while (1) {
        pkt_ = tcp_.recv();
        if (!pkt_) return pkt_.result();

        const auto* pktData = pkt_.data();
        for (uint16_t pktOfs = 0; pktOfs < pkt_.size(); ) {
            // printf("'%c' %d %d\r\n", pktData[pktOfs], pktOfs, state_);
            switch (state_) {
            case State::HEADER_STATUS_HTTP:
                if (' ' != pktData[pktOfs++]) continue;
                state_ = State::HEADER_STATUS_CODE;
                continue;
            case State::HEADER_STATUS_CODE:
            {
                const char ch = pktData[pktOfs++];
                if (' ' != ch) {
                    if (isdigit(ch)) statusCode = statusCode * 10 + (ch - '0');
                    continue;
                }
                statusCode_ = statusCode;
                state_ = State::HEADER_STATUS_REASON;
                continue;
            }
            case State::HEADER_STATUS_REASON:
                if ('\n' == pktData[pktOfs++]) state_ = State::HEADER;
                continue;
            case State::HEADER:
                if ('\r' == pktData[pktOfs] || '\n' == pktData[pktOfs]) {
                    if ('\n' == pktData[pktOfs]) {
                        if (0 == headerOfs) state_ = State::CONTENT;
                        headerOfs = 0;
                    }
                    ++pktOfs;
                    continue;
                }
                headerOfs = 0;
                state_ = State::HEADER_FIELD;
                continue;
            case State::HEADER_FIELD:
            {
                const char ch = pktData[pktOfs++];
                if (':' != ch && headerOfs < sizeof(headerField)) {
                    headerField[headerOfs++] = ch;
                    continue;
                }
                headerField[headerOfs] = 0;
                headerOfs = 0;
                state_ = State::HEADER_VALUE;
                continue;
            }
            case State::HEADER_VALUE:
            {
                if ('\r' == pktData[pktOfs] || '\n' == pktData[pktOfs]) {
                    state_ = State::HEADER;
                    headerValue[headerOfs++] = 0;
printf("%s:%s\r\n", headerField, headerValue);

                    if (0 == strcasecmp(headerField, "Content-Length")) {
                        contentLength_ = atoi(headerValue);
                    }
                    continue;
                }

                const char ch = pktData[pktOfs++];
                if (headerOfs || ' ' != ch) {
                    if (headerOfs < (sizeof(headerValue) - 1)) {
                        headerValue[headerOfs++] = ch;
                    }
                }
                continue;
            }
            case State::CONTENT:
                if (pktOfs >= pkt_.size()) continue; // need more data
                pkt_ofs_ = pktOfs;
                return ERR_OK;
            case State::FINISH:
            default:
                pkt_ofs_ = pkt_.size();
                return ERR_USE;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// retrieve next block of the body content
/// @returns ERR_OK on success (Netconn error code)
err_t HttpConnection::next() {
    if (State::CONTENT != state_) {
        pkt_ofs_ = pkt_.size();
        return ERR_USE;
    }

    // consumed all content?
    contentOfs_ += available();
    if (contentOfs_ >= contentLength_) {
        state_ = State::FINISH;
        pkt_ofs_ = pkt_.size();
        return ERR_OK;
    }

    // receive next packet
    pkt_ofs_ = 0;
    pkt_ = tcp_.recv();
    return pkt_.result();
}
