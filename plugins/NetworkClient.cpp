// Developer by Andrey Bakhmutov, Sep 2018

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "NetworkClient.h"

NetworkClient::NetworkClient( )
    : m_sd( -1 )
{
}

NetworkClient::~NetworkClient( )
{
    close_connection( );
}

void NetworkClient::close_connection( )
{
    if ( m_sd >= 0 )
    {
        close( m_sd );
        m_sd = -1;
    }
}

bool NetworkClient::is_connected( ) const
{
    return m_sd != -1;
}

int NetworkClient::connect( const char* host, uint16_t port )
{
    hostent *shost = 0, hem;
    char buf[2048];
    int err = 0;

    if ( gethostbyname_r( host, &hem, buf, sizeof( buf ), &shost, &err ) || !shost )
    {
        return -1;
    }

    uint32_t ip;
    memcpy( &ip, shost->h_addr_list[0], sizeof( ip ) );

    return connect( ip, port );
}

int NetworkClient::connect( uint32_t ip, uint16_t port )
{
    m_sd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if ( m_sd == -1 )
        return -1;

    sockaddr_in addr;

    memset( ( char* )&addr, 0, sizeof( addr ) );

    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = ip;

    if (::connect( m_sd, ( sockaddr* )&addr, sizeof( addr ) ) == -1 )
    {
        close( m_sd );
        m_sd = -1;
    }

    return m_sd;
}

int NetworkClient::recv_buffer( char* buf, size_t bsize, int timeout_sec, bool& was_timeout )
{
    fd_set readfds;
    FD_ZERO( &readfds );
    FD_SET( m_sd, &readfds );

    timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    was_timeout = false;
    int ret;

    do
    {
        ret = ::select( m_sd + 1, &readfds, 0, 0, &timeout );
    } while ( ret == -1 && errno == EINTR );

    if ( !ret )
    {
        was_timeout = true;
        return -1;
    }

    if ( ret == -1 )
        return -1;

    return ::read( m_sd, buf, bsize );
}

int NetworkClient::send_buffer( const char* buf, size_t size )
{
    return ::write( m_sd, buf, size );
}
