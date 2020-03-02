rm -rf ./deb_package
mkdir -p ./deb_package/usr/local/bin
mkdir -p ./deb_package/usr/local/etc
mkdir -p ./deb_package/usr/local/share/applications
mkdir -p ./deb_package/usr/local/share/icons
cp -pR ./res/DEBIAN/ ./deb_package
cp ./res/tarpuqJIGsw.json ./deb_package/usr/local/etc/tarpuqJIGsw.json
cp ./res/tarpuqJIGsw.desktop ./deb_package/usr/local/share/applications
cp ./src/icons/tq.png ./deb_package/usr/local/share/icons
cp ./linux/release/bin/tarpuqJIGsw ./deb_package/usr/local/bin
chmod -R 755 ./deb_package
dpkg-deb --build ./deb_package tarpuqJIGsw.deb
mkdir -p ./linux/package
mv ./tarpuqJIGsw.deb ./linux/package
