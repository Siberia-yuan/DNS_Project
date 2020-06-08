# !/bin/bash

cd src
gcc DNS_client.c -o DNS_client -std=c99
gcc DNS_server_local.c -o DNS_server_local -std=c99
gcc DNS_root_server.c -o DNS_root_server -std=c99
gcc DNS_2LD_edu.c -o DNS_2LD_edu -std=c99
gcc DNS_2LD_gov.c -o DNS_2LD_gov -std=c99
gcc DNS_TLD_cn_us.c -o DNS_TLD_cn_us -std=c99
gcc DNS_TLD_com_org.c -o DNS_TLD_com_org -std=c99


mv DNS_root_server ../bin/DNS_root_server
mv DNS_2LD_edu  ../bin/DNS_2LD_edu
mv DNS_2LD_gov ../bin/DNS_2LD_gov
mv DNS_TLD_cn_us ../bin/DNS_TLD_cn_us
mv DNS_TLD_com_org ../bin/DNS_TLD_com_org
mv DNS_client ../bin/DNS_client
mv DNS_server_local ../bin/DNS_server_local
