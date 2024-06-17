#!/bin/bash
ifconfig_output=$(ifconfig)

ip_line=$(echo "$ifconfig_output" | grep -oE 'inet (addr:)?([0-9]*\.){3}[0-9]*')

ip_address=$(echo "$ip_line" | grep -oE '([0-9]*\.){3}[0-9]*')

echo "IP地址: $ip_address"
