# cpp-swoole


Install swoole
====
```shell
git clone https://github.com/swoole/swoole-src.git
phpize
./configure
cmake .
#cmake -DCMAKE_INSTALL_PREFIX=/opt/swoole .
sudo make install
```

Build
====
```shell
mkdir build
cd build
cmake ..
make
```

Run
===
```shell
./bin/server
```
