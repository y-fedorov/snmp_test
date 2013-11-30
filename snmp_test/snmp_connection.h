//
//  snmp_connection.h
//
//
//  Code Auhtor: Moshe Levi
//  Code Autor blog: http://blog-moshe.blogspot.ru/2012/10/boost-asio-with-net-snmp.html
//
//  Reproduced by Yaroslav Fedorov on 26/11/2013.
//  Copyright (c) 2013 Yaroslav Fedorov. All rights reserved.
//

#ifndef snmp_connection_h
#define snmp_connection_h

#include <string.h>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

using namespace std;
using namespace boost::asio::ip;

typedef boost::function<void(string)> SNMPHandler;


class snmp_connection
{
public:                                  
    snmp_connection(boost::asio::io_service& io_service)
    : snmp_socket_(io_service),
    snmp_handle_(NULL) {};
    ~snmp_connection() {
        snmp_sess_close(this->snmp_handle_); 
    }
    void connect(long version, const string& peername, const string& community);
    void async_snmp_get(const string& snmp_oid, SNMPHandler handler);
    void async_snmp_set(const string& snmp_oid, const int type, const string& value, SNMPHandler handler);
private:
    void handle_snmp_req(const boost::system::error_code &ec, std::size_t bytes_transferred, 
                         struct snmp_pdu *pdu, SNMPHandler handler);
    void handle_snmp_res(const boost::system::error_code &ec, std::size_t bytes_transferred, 
                         SNMPHandler handler);
    
    udp::socket snmp_socket_;
    void *snmp_handle_;
};

#endif
