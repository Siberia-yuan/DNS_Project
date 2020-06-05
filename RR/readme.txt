127.0.0.2: local server
127.0.0.3: root server
127.0.0.4: cn & us domain
127.0.0.5: microsoft.com & ietf.org domain
127.0.0.6: edu.cn domain
127.0.0.7: gov.us domain

格式：
domain name, TTL, Class, Type, Value

ip地址必须是符合规范的，不然没法发送
每一行必须以逗号结尾
mx的ip必须在对应mx的域名的前面，不然会找不到