#!/bin/bash

#-------------------------------
#
# Openvpn-Testconfig-Generator
#
# 
#-------------------------------

if [ $# -eq 0 ]
then
    echo "
    Usage: ./openvpn_test_config_gen.sh 255 key!
    The Number is the number of the configured tunnels
    and \"key\" is the flag to produce the keyfiles.
    It only works if openvpn is installed on the machine!
     "
    exit 0
fi

n=`echo $1`
if [ $1 -gt 255 ]
then
    echo " ERROR: only numbers to 255 are allowed!!!"
    exit 0
fi

if [ -n "$2" -a "$2" = "key" ]
then
    key=true
else
    key=false
fi


 echo "n: " $n
 echo "OPT_OPENVPN='yes'
  
OPENVPN_EXPERT='no'
  
  
OPENVPN_WEBGUI='yes' 
  
OPENVPN_DEFAULT_START='always'
OPENVPN_DEFAULT_COMPRESS='yes'
OPENVPN_DEFAULT_OPEN_OVPNPORT='yes'
OPENVPN_DEFAULT_CIPHER='AES-256-CBC'
OPENVPN_DEFAULT_DIGEST='SHA1'
  
OPENVPN_N='${n}'" >> openvpn.txt
  
  

i=1
while [ "$i" -le "$n" ]
do
   y=`expr ${i} + 1024`
echo "#-------------------------------------
#
#     Tunnel Name_${i}
#
#-------------------------------------
OPENVPN_${i}_NAME='name_${i}'
OPENVPN_${i}_LOCAL_PORT='${y}'
OPENVPN_${i}_REMOTE_PORT='${y}'" >> openvpn.txt
  if [ "$key" = "true" ]
  then
      openvpn --genkey --secret test_${i}.key
       echo "OPENVPN_${i}_SECRET='test_${i}.key'" >> openvpn.txt
  else
       echo "OPENVPN_${i}_SECRET='test.key'" >> openvpn.txt
  fi  
echo "OPENVPN_${i}_SECRET='test_${i}.key'
OPENVPN_${i}_TYPE='tunnel'
OPENVPN_${i}_REMOTE_VPN_IP='10.0.0.${i}'
OPENVPN_${i}_LOCAL_VPN_IP='10.0.1.${i}'

OPENVPN_${i}_ROUTE_N='1'
OPENVPN_${i}_ROUTE_1='192.168.${i}.0/24

OPENVPN_${i}_PF_INPUT_N='1'
OPENVPN_${i}_PF_INPUT_1='if:VPNDEV:any ACCEPT BIDIRECTIONAL'

OPENVPN_${i}_PF_FORWARD_N='2'
OPENVPN_${i}_PF_FORWARD_1='state:ESTABLISHED,RELATED ACCEPT'
OPENVPN_${i}_PF_FORWARD_2='if:VPNDEV:any ACCEPT BIDIRECTIONAL'

OPENVPN_${i}_PF_POSTROUTING_N='0'


"  >> openvpn.txt
  i=`expr $i + 1`
done
            