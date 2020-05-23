# !/bin/bash

cd src
gcc DNS_client.c -o DNS_client.o
gcc DNS_server_local.c -o DNS_server_local.o




mv DNS_client.o ../bin/DNS_client.o
mv DNS_server_local.c ../bin/DNS_server_local.o

cd ../bin
./DNS_server_local.o
./DNS_client.o