//
//  main.cpp
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

void snmp_print(string result, int num){
    cout << "result " << result << endl;
    cout << "num " << num << endl;
}

int main(int argc, const char * argv[])
{
    boost::asio::io_service io_service;
    snmp_connection snmp_conn(io_service);
    snmp_conn.connect(SNMP_VERSION_2c,string("192.168.1.1"), string("public"));
    snmp_conn.async_snmp_get(string(".1.3.6.1.2.1.1.4.0"), boost::bind(snmp_print,_1,3));
    io_service.run();
    
    return 0;
}
