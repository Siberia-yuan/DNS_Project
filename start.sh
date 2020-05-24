# !/bin/bash

cd src
gcc DNS_client.c -o DNS_client
gcc DNS_server_local.c -o DNS_server_local

mv DNS_client ../bin/DNS_client
mv DNS_server_local ../bin/DNS_server_local

cd ../bin
#./DNS_server_local 127.0.0.2 4001
#./DNS_client sdafadffa sadsadd
