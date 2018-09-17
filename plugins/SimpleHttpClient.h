// Developer by Andrey Bakhmutov, Sep 2018

#ifndef _SIMPLE_HTTP_CLIENT_H_
#define _SIMPLE_HTTP_CLIENT_H_

#include <cstdint>
#include <string>
#include <vector>

#include "NetworkClient.h"

class SimpleHttpClient
{
public:
    enum class Status : uint8_t
    {
        OK,
        ConnectionError,
        HttpError,
        DataError,
        Timeout
    };

    SimpleHttpClient( uint32_t timeout_sec = 30 );
    ~SimpleHttpClient( );

    bool open( const char* hostname, uint16_t port );
    void close( );

    Status send_get_request( const std::string& path, size_t& content_length, uint32_t& http_code );

    Status recv_payload_chunk( char* buf, size_t bsize, size_t& bytes_read );

    static const char* status2str( Status status );

private:
    Status parse_server_response( size_t& content_length, uint32_t& http_code );

private:
    NetworkClient m_connection;
    char m_buf[4096];
    size_t m_size;
    int m_payload_pos;
    size_t m_content_length;
    uint32_t m_timeout_sec;
};

#endif
