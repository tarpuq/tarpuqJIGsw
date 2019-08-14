rm -rf ./deb_package
mkdir -p ./deb_package/usr/local/bin
mkdir -p ./deb_package/usr/local/etc
mkdir -p ./deb_package/usr/local/share/applications
mkdir -p ./deb_package/usr/local/share/icons
chmod -R 755 ./deb_package
cp -pR ./res/DEBIAN/ ./deb_package
cp ./res/tarpuqJIGsw.json ./deb_package/usr/local/etc/tarpuqJIGsw.json
cp ./res/tarpuqJIGsw.desktop ./deb_package/usr/local/share/applications
cp ./res/tq.png ./deb_package/usr/local/share/icons
cp ./linux/release/tarpuqJIGsw ./deb_package/usr/local/bin
dpkg-deb --build ./deb_package tarpuqJIGsw.deb
