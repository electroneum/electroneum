#!/bin/bash
# Configure the SoapUI file with the correct values

wallet_file_name="MyWallet"
rpc_address="localhost"
rpc_port="18083"
subaddr_count="15000000"
send_amount="10000"         # atomic units

sed -i.bak "s/_wallet_file_name_/$wallet_file_name/g" ./SoapUI-SubAddress-Stress-Testing.xml
sed -i.bak "s/_rpc_address_/$rpc_address/g" ./SoapUI-SubAddress-Stress-Testing.xml
sed -i.bak "s/_rpc_port_/$rpc_port/g" ./SoapUI-SubAddress-Stress-Testing.xml
sed -i.bak "s/_subaddr_count_/$subaddr_count/g" ./SoapUI-SubAddress-Stress-Testing.xml
sed -i.bak "s/_send_amount_/$send_amount/g" ./SoapUI-SubAddress-Stress-Testing.xml