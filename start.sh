# !/bin/bash

cd src
#gcc DNS_client.c -o DNS_client
#gcc DNS_server_local.c -o DNS_server_local
#gcc DNS_root_server.c -o DNS_root_server
gcc DNStestUDP.c -o DNStestUDP
gcc DNS_2LD_edu.c -o DNS_2LD_edu

#mv DNS_root_server ../bin/DNS_root_server
mv DNStestUDP  ../bin/DNStestUDP
mv DNS_2LD_edu  ../bin/DNS_2LD_edu
#mv DNS_client ../bin/DNS_client
#mv DNS_server_local ../bin/DNS_server_local

#cd ../bin
#./DNS_server_local 4001
#./DNS_client sdafadffa sadsadd
