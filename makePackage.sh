qmake
make -j4
rm -rf ./linux/deb_package
mkdir -p ./linux/deb_package/usr/local/bin
mkdir -p ./linux/deb_package/usr/local/etc
mkdir -p ./linux/deb_package/usr/local/share/applications
mkdir -p ./linux/deb_package/usr/local/share/icons
mkdir -p ./linux/package
cp -pR ./res/DEBIAN/ ./linux/deb_package
cp ./res/tarpuqJIGsw.json ./linux/deb_package/usr/local/etc/tarpuqJIGsw.json
cp ./res/tarpuqJIGsw.desktop ./linux/deb_package/usr/local/share/applications
cp ./src/icons/tq.png ./linux/deb_package/usr/local/share/icons
cp ./linux/release/bin/tarpuqJIGsw ./linux/deb_package/usr/local/bin
chmod -R 755 ./linux/deb_package
dpkg-deb --build ./linux/deb_package ./linux/package/tarpuqJIGsw.deb