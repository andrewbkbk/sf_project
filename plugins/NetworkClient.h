// Developed by Andrey Bakhmutov, Sep 2018

#ifndef _NETWORK_CLIENT_H_
#define _NETWORK_CLIENT_H_

#include <stdint.h>
#include <unistd.h>

class NetworkClient
{
public:
    NetworkClient( );
    ~NetworkClient( );

    int connect( const char* host, uint16_t port );
    int connect( uint32_t ip, uint16_t port );

    bool is_connected( ) const;
    void close_connection( );

    int send_buffer( const char* buf, size_t len );
    int recv_buffer( char* buf, size_t bsize, int timeout_sec, bool& was_timeout );

private:
    int m_sd; // socket descriptor
};

#endif //_NETWORK_CLIENT_H_
