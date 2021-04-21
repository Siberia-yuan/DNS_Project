# DNS_Project
北京邮电大学 互联网应用大作业 DNS系统设计与实现</br>
Coursework project, a DNS system which is capable of execute iterative query.

## build
```
DNS_Project user$ ./compile.sh
```
## activate DNS server
```
DNS_Project user$ cd bin
bin user$ ./DNS_server_local
bin user$ ./DNS_root_server
bin user$ ./DNS_TLD_cn_us
bin user$ ./DNS_TLD_com_org
bin user$ ./DNS_2LD_gov
bin user$ ./DNS_2LD_edu
```
## send client query
```
bin user$ ./DNS_client <queryType> <domainName>
```


