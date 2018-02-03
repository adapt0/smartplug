/////////////////////////////////////////////////////////////////////////////
/** @file
Encapsulates an HTTP connection

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__HTTP_CONNECTION
#define INCLUDED__HTTP_CONNECTION

//- includes
#include "net_conn.h"

/////////////////////////////////////////////////////////////////////////////
/// Encapsulates an HTTP connection
///
/// \code
/// 
/// struct ip_addr addr;
/// IP4_ADDR(&addr, 192, 168, 1, 1);
/// 
/// HttpConnection http{ &addr, 80 };
/// if (ERR_OK != http.get("/index.html")) {
///     printf("Connection failed\n");
///     return false;
/// }
/// 
/// if (200 != http.statusCode()) {
///     printf("Unexpected status code %d\n", http.statusCode());
///     return false;
/// }
/// 
/// while (true) {
///     const auto available = http.available();
///     if (available <= 0) break;
/// 
///     const auto* bytes = http.data();
/// 
///     // do_something_with( bytes, available );
/// 
///     if (ERR_OK != http.next()) {
///         printf("Failed to read\n");
///         return false;
///     }
/// }
/// 
/// \endcode
class HttpConnection {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    /// @param addr IP address to connect to
    /// @param port TCP port to connect on
    HttpConnection(const ip_addr_t* addr, uint16_t port)
    : tcp_(NETCONN_TCP)
    , addr_(*addr)
    , port_(port)
    { }

    /// @private noncopyable
    HttpConnection(const HttpConnection&) = delete;
    /// @private noncopyassignable
    HttpConnection& operator=(const HttpConnection&) = delete;

    /// move constructor
    HttpConnection(HttpConnection&&) = default;

    err_t get(const char* url);
    err_t next();

    /////////////////////////////////////////////////////////////////////////
    /// Retrieve size of the current block of partial body content
    /// @returns size
    size_t available() const { return pkt_.size() - pkt_ofs_; }
    /// Pointer to current block of partial body content
    /// @returns block of body content
    const char* data() const { return pkt_.data() + pkt_ofs_; }

    /////////////////////////////////////////////////////////////////////////
    /// Expected content length indicated by HTTP headers
    /// @returns size (in butes)
    size_t contentLength() const { return contentLength_; }
    /// HTTP status code from HTTP response
    /// @returns HTTP status code
    int statusCode() const { return statusCode_; }

private:
    /// HTTP connection states
    enum class State {
        // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
        HEADER_STATUS_HTTP,
        HEADER_STATUS_CODE,
        HEADER_STATUS_REASON,
        HEADER,
        HEADER_FIELD,
        HEADER_VALUE,
        CONTENT,
        FINISH,
    };

    err_t retrieveHeaders_();

    NetConn             tcp_;           ///< connection
    const ip_addr_t     addr_;          ///< IP address to connect to
    const uint16_t      port_{0};       ///< port to connect to

    NetConn::Packet     pkt_;           ///< last received packet
    size_t              pkt_ofs_ = 0;   ///< offset of body in pkt_ data

    State       state_ = State::HEADER_STATUS_HTTP;    ///< state
    int         statusCode_ = -1;       ///< status code
    size_t      contentLength_ = 0;     ///< received content length
    size_t      contentOfs_ = 0;        ///< content offset
};

#endif // INCLUDED__HTTP_CONNECTION
