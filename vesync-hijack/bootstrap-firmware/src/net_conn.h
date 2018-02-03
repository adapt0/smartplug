/////////////////////////////////////////////////////////////////////////////
/** @file
lwip Netconn C++ wrapper

Encapsulates a Netconn socket with lifetime management

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__NET_CONN
#define INCLUDED__NET_CONN

//- includes
#undef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 0 ///< get rid of macros (bind, connect, etc)
#include <lwip/api.h>

/////////////////////////////////////////////////////////////////////////////
/// netconn C++ wrapper
class NetConn {
    struct UseRecv { };
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    /// @param type Type of netconn connection (NETCONN_TCP, NETCONN_UDP)
    explicit NetConn(netconn_type type) {
        conn_ = netconn_new(type);
    }
    /// destructor
    ~NetConn() {
        netconn_delete(conn_);
        conn_ = nullptr;
    }

    /// @private noncopyable
    NetConn(const NetConn&) = delete;
    /// @private noncopyassignable
    NetConn& operator=(const NetConn&) = delete;

    /////////////////////////////////////////////////////////////////////////
    /// Bind a netconn to a specific local IP address and port
    /// @param addr IP address
    /// @param port IP port
    /// @returns ERR_OK on success (Netconn error code)
    err_t bind(const ip_addr_t* addr, uint16_t port) {
        return netconn_bind(conn_, const_cast<ip_addr_t*>(addr), port);
    }
    /// Connect to a specific remote IP address and port
    /// @param addr IP address to connect to
    /// @param port IP port to connect on
    /// @returns ERR_OK on success (Netconn error code)
    err_t connect(const ip_addr_t* addr, uint16_t port) {
        return netconn_connect(conn_, const_cast<ip_addr_t*>(addr), port);
    }
    /// Join multicast groups for UDP netconns
    /// @param multiaddr Multicast IP address group to leave
    /// @param netif_addr Associated network interface IP (may be nullptr)
    /// @param join_or_leave NETCONN_JOIN/NETCONN_LEAVE
    /// @returns ERR_OK on success (Netconn error code)
    err_t joinLeaveGroup(const ip_addr_t* multiaddr, const ip_addr_t* netif_addr, netconn_igmp join_or_leave) {
        return netconn_join_leave_group(conn_, const_cast<ip_addr_t*>(multiaddr), const_cast<ip_addr_t*>(netif_addr), join_or_leave);
    }
    /// set receive timeout
    /// @param timeout_ms milliseconds
    /// @returns ERR_OK on success (Netconn error code)
    void setRecvTimeout(int timeout_ms) {
        netconn_set_recvtimeout(conn_, timeout_ms);
    }
    /// Send data over a TCP connection
    /// @param data Data to send
    /// @param size Amount of data to send
    /// @param apiflags netconn write flags
    /// @returns ERR_OK on success (Netconn error code)
    err_t write(const void* data, size_t size, uint8_t apiflags) {
        return netconn_write(conn_, data, size, apiflags);
    }

    /////////////////////////////////////////////////////////////////////////
    /// NetConn received packet wrapper
    /// @sa NetConn::recv
    class Packet {
    public:
        /////////////////////////////////////////////////////////////////////
        /// default constructor
        Packet() = default;
        /// @private constructor
        Packet(UseRecv, netbuf* nb, err_t result, const char* data = nullptr, uint16_t size = 0)
        : nb_(nb), result_(result), data_(data), size_(size)
        { }
        /// destructor
        ~Packet() {
            if (nb_) netbuf_delete(nb_);
        }

        /////////////////////////////////////////////////////////////////////
        /// move constructor
        Packet(Packet&& rhs)
        : nb_(rhs.nb_), result_(rhs.result_), data_(rhs.data_), size_(rhs.size_)
        {
            rhs.nb_ = nullptr;
        }
        /// move assignment
        Packet& operator=(Packet&& rhs) {
            if (nb_) netbuf_delete(nb_);
            nb_ = rhs.nb_;
            rhs.nb_ = nullptr;
            result_ = rhs.result_;
            rhs.result_ = ERR_VAL;
            data_ = rhs.data_;
            size_ = rhs.size_;
            return *this;
        }

        /// @private noncopyable
        Packet(const Packet&) = delete;
        /// @private noncopyassignable
        Packet& operator=(const Packet&) = delete;

        /////////////////////////////////////////////////////////////////////
        /// associated result
        /// @returns ERR_OK if recv was successful
        err_t result() const { return result_; }
        /// valid packet?
        /// @returns true if packet is invalid
        bool operator!() const {
            return ERR_OK != result_;
        }

        /////////////////////////////////////////////////////////////////////
        /// packet data
        /// @returns bytes
        const char* data() const { return data_; }
        /// amount of packet data
        /// @returns size in bytes
        size_t size() const { return size_; }

        /// remote address
        /// @returns ip address
        const ip_addr_t* fromAddr() const {
            return netbuf_fromaddr(nb_);
        }

    private:
        netbuf*     nb_ = nullptr;      ///< underlying netconn buffer
        err_t       result_  = ERR_VAL; ///< cached read error code
        const char* data_ = nullptr;    ///< packet data (points into netbuf)
        size_t      size_ = 0;          ///< size of data_
    };

    /////////////////////////////////////////////////////////////////////////
    /// Retrieve next available packet of data
    /// @returns Packet
    /// @sa NetConn::Packet
    Packet recv() {
        netbuf* nb;
        const auto res = netconn_recv(conn_, &nb);
        if (ERR_OK == res) {
            const char* data;
            uint16_t len;
            netbuf_data(nb, (void**)&data, &len);
            return Packet{ UseRecv{}, nb, res, data, len };
        } else {
            return Packet{ UseRecv{}, nb, res };
        }
    }

private:
    netconn*    conn_ = nullptr;    ///< underlying handle
};

#endif // INCLUDED__NET_CONN
