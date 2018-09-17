// Developed by Andrey Bakhmutov, Sep 2018

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "SimpleHttpClient.h"

SimpleHttpClient::SimpleHttpClient( uint32_t timeout_sec )
    : m_size( 0 )
    , m_payload_pos( -1 )
    , m_content_length( 0 )
    , m_timeout_sec( timeout_sec )
{
}

SimpleHttpClient::~SimpleHttpClient( )
{
    close( );
}

bool SimpleHttpClient::open( const char* hostname, uint16_t port )
{
    if ( m_connection.connect( hostname, port ) < 0 )
    {
        return false;
    }
    return true;
}

void SimpleHttpClient::close( )
{
    if ( m_connection.is_connected( ) )
        m_connection.close_connection( );
}

SimpleHttpClient::Status SimpleHttpClient::send_get_request( const std::string& path,
                                                             size_t& content_length,
                                                             uint32_t& http_code )
{
    std::stringstream s;

    content_length = 0;
    http_code = 0;

    m_size = 0;
    m_payload_pos = -1;

    if ( !m_connection.is_connected( ) )
        return Status::ConnectionError;

    s << "GET " << path << " HTTP/1.1\r\n";
    s << "Host: "
      << "localhost.localdomain\r\n";
    s << "\r\n";

    const std::string& str = s.str( );

    if ( m_connection.send_buffer( str.c_str( ), str.length( ) ) == -1 )
    {
        return Status::ConnectionError;
    }

    bool was_timeout;

    memset( m_buf, 0, sizeof( m_buf ) );

    // assume that all HTTP headers are in the first 4kb of the response

    int ret = m_connection.recv_buffer( m_buf, sizeof( m_buf ) - 1, m_timeout_sec, was_timeout );

    if ( ret < 0 )
    {
        return was_timeout ? Status::Timeout : Status::ConnectionError;
    }

    m_size = static_cast< size_t >( ret );

    return parse_server_response( content_length, http_code );
}

SimpleHttpClient::Status
SimpleHttpClient::parse_server_response( size_t& content_length, uint32_t& http_code )
{
    if ( m_size < 15 )
        return Status::DataError;

    if ( strncasecmp( m_buf, "HTTP/1.1 ", 9 ) )
        return Status::DataError;

    http_code = std::strtoul( m_buf + 9, 0, 10 );

    if ( http_code != 200 )
        return Status::HttpError;

    const char* body_start = strstr( m_buf, "\r\n\r\n" );
    if ( !body_start )
        return Status::DataError;
    body_start += 4;

    const char* cl = strcasestr( m_buf, "Content-Length:" );
    if ( cl && cl < body_start )
    {
        cl += 15;
        while ( *cl == ' ' || *cl == '\t' )
            cl++;
        content_length = std::strtoul( cl, 0, 10 );
    }
    else
    {
        return Status::DataError;
    }
    m_payload_pos = body_start - m_buf;

    return Status::OK;
}

SimpleHttpClient::Status
SimpleHttpClient::recv_payload_chunk( char* buf, size_t bsize, size_t& bytes_read )
{
    bytes_read = 0;

    if ( !m_size )
        return Status::ConnectionError;
    if ( m_payload_pos == -1 )
        Status::DataError;

    if ( m_payload_pos < m_size )
    {
        size_t size = std::min( bsize, m_size - m_payload_pos );
        memcpy( buf, m_buf + m_payload_pos, size );
        m_payload_pos += size;
        bytes_read = size;

        return Status::OK;
    }

    bool was_timeout;

    int ret = m_connection.recv_buffer( buf + bytes_read, bsize - bytes_read, m_timeout_sec,
                                        was_timeout );

    if ( ret < 0 )
    {
        return was_timeout ? Status::Timeout : Status::ConnectionError;
    }

    bytes_read = +ret;

    return Status::OK;
}

const char* SimpleHttpClient::status2str( Status status )
{
    switch ( status )
    {
    case Status::OK:
        return "OK";
    case Status::ConnectionError:
        return "ConnectionError";
    case Status::HttpError:
        return "HttpError";
    case Status::DataError:
        return "DataError";
    case Status::Timeout:
        return "Timeout";
    default:
        return "Unknown";
    };
}
