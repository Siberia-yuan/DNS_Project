# !/bin/bash

cd src
gcc DNS_client.c -o DNS_client
gcc DNS_server_local.c -o DNS_server_local
gcc DNS_root_server.c -o DNS_root_server
gcc DNStestUDP.c -o DNStestUDP


mv DNS_root_server ../bin/DNS_root_server
mv DNStestUDP  ../bin/DNStestUDP
mv DNS_client ../bin/DNS_client
mv DNS_server_local ../bin/DNS_server_local

#cd ../bin
#./DNS_server_local 4001
#./DNS_client sdafadffa sadsadd
