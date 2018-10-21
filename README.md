
<h1>Openwrt - VPN Configurator</h1>

<h2>Description</h2>
<p> This package provides a solution for automatic  vpn configurations. The package  downloads the configuration files frm the given URL and prepares them to use with  openvpn</p>

<h2> Installation </h2>
1 - Add this git repository in  your feeeds file in build environment and run the followings

./scripts/feeds update -a
./scripts/feeds install -a 

2 - Run sudo make V=s

3 - once the package is compiled, copy it to the  your device 's /tmp folder

4 - install the packe with opkg install <package Name>
  
5 - Configure the  URL for VPN Configurations

5-  Run the package with various options to download the vpn files, select random 3 vpn files, try in sequence the vpn files.

6- in each case package tries to connect with the selected vpn configurtion files, if  no configuration is successfull, package tries to select other files based on the configuration.

7 - once successfull, package connects to the selected VPN server

