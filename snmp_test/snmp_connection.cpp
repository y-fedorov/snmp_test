//
//  snmp_connection.cpp
//
//
//  Code Auhtor: Moshe Levi
//  Code Autor blog: http://blog-moshe.blogspot.ru/2012/10/boost-asio-with-net-snmp.html
//
//  Reproduced by Yaroslav Fedorov on 26/11/2013.
//  Copyright (c) 2013 Yaroslav Fedorov. All rights reserved.
//

#include <iostream>

#include "snmp_connection.h"

int asynch_response(int operation, struct snmp_session *sp, int reqid,
                    struct snmp_pdu *pdu, void *magic)
{
    if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
        char buf[1024];
        
        if (pdu->errstat == SNMP_ERR_NOERROR) {
            if(pdu->variables) {
                print_variable(pdu->variables->name, pdu->variables->name_length,  pdu->variables);
                snprint_variable(buf, sizeof(buf), pdu->variables->name, pdu->variables->name_length, pdu->variables);
                //  snmp_connection::response_ = buf;
            }
        }
        
    } else if (operation == NETSNMP_CALLBACK_OP_TIMED_OUT) {
        cout << "TIMED_OUT" << endl;
    } else if (operation == NETSNMP_CALLBACK_OP_SEND_FAILED){
        cout << "SEND_FAILED" << endl;
    } else if (operation == NETSNMP_CALLBACK_OP_CONNECT){
        cout << "CONNECT" << endl;
    } else if (operation == NETSNMP_CALLBACK_OP_DISCONNECT){
        cout << "DISCONNECT" << endl;
    }
    
    return 1;
}

void snmp_connection::connect(long version, const string& peername, const string& community){
    
    struct snmp_session sess;
    snmp_sess_init(&sess);   /* initialize session */
    sess.version = version;
    sess.peername = strdup(peername.c_str());
    sess.community = reinterpret_cast<u_char*>(strdup(community.c_str()));
    sess.community_len = community.size();
    sess.callback = asynch_response;
    snmp_handle_ = snmp_sess_open(&sess);
    free(sess.peername);
    free(sess.community);
    netsnmp_transport *transport  = snmp_sess_transport( snmp_handle_ );
    snmp_socket_.assign(boost::asio::ip::udp::v4(), transport->sock);
    // Put the socket into non-blocking mode.
    udp::socket::non_blocking_io non_blocking_io(true);
    snmp_socket_.io_control(non_blocking_io);    
}

void snmp_connection::async_snmp_get(const string& snmp_oid, SNMPHandler handler) {
    
    struct snmp_pdu *pdu = NULL;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;
    
    pdu = snmp_pdu_create(SNMP_MSG_GET);   
    if (!snmp_parse_oid(snmp_oid.c_str(), anOID, &anOID_len)) {
        snmp_perror(snmp_oid.c_str());
        cout << "error snmp_parse_oid" << endl;
    }    
    
    snmp_add_null_var(pdu, anOID, anOID_len);
    snmp_socket_.async_send(
                            boost::asio::null_buffers(),
                            boost::bind(&snmp_connection::handle_snmp_req,
                                        this, boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred, pdu, handler));
}

void snmp_connection::handle_snmp_req(const boost::system::error_code &ec, std::size_t bytes_transferred,
                                      struct snmp_pdu *pdu, SNMPHandler handler)
{
    if (this->snmp_socket_.is_open()){
        if (ec) {
            cout << "error snmp_connection::handle_snmp_req" << endl;
        } else {
            // Notify net-snmp library that it can perform a write. 
            snmp_sess_send(snmp_handle_, pdu);
            snmp_socket_.async_receive(  boost::asio::null_buffers(), 
                                       boost::bind(&snmp_connection::handle_snmp_res, this, 
                                                   boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred, handler));
        }
    }    
}

void snmp_connection::handle_snmp_res(const boost::system::error_code &ec, std::size_t bytes_transferred, 
                                      SNMPHandler handler)


{
    if (this->snmp_socket_.is_open()) {
        if (ec) {
            cout << "error snmp_connection::handle_snmp_res" << endl;
        } else {      
            fd_set snmp_fds;
            FD_ZERO(&snmp_fds);
            FD_SET(snmp_socket_.native(), &snmp_fds);
            snmp_sess_read(snmp_handle_, &snmp_fds);
        }
    }
}
