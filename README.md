# imu_sample
Raspberry Pi Pico で LSM9DS1 を SPIでアクセスするサンプル
のバージョン２です。（バージョン1はimu_test）

## ビルド手順

念の為

`export PICO_SDK_PATH=../../pico-sdk`


pico-sdkディレクトリがある場所で

```
git clone https://github.com/kouhei1970/imu_sample.git
cd imu_test
mkdir build
cd build
cmake ..
make
```

## 接続

Picoとの接続は以下の様にするものとします。

|Pico側|LSM9DS1ボード側|備考|
|---|---|---|
|GPIO 1 (pin 2) SPI0 CSn|CSAG|加速度・ジャイロ選択|
|GPIO 4 (pin 6) MISO/SPI0 RX|SDO（２ピンとも）|センサからデータ出力|
|GPIO 5 (pin 7) SPI00 CSn|CSM|地磁気計選択|
|GPIO 6 (pin 9) SCK/SPI0 SCK|SCL|クロック|
|GPIO 7 (pin 10) MOSI/SPI0 TX|SDA|センサへデータ出力|

## C++からCの関数を使用する

ここではCの関数として用意された９DOFセンサのライブラリをC++で使用する。
imu_testはCで書いたサンプルだったので、問題なかったが、C++のメインからCのライブラリを使う際に若干問題が生じる。

センサのライブラリはプラットフォームに依存するSPIやI2Cの通信はユーザーが作成し関数のポインタをライブラリ側に渡すことで
ライブラリがどの様なプラットフォームでも機能する様にしている。

どうやら、C++のソースコードで通信用の関数を書くと、おそらくCのライブラリがC++の関数を使用する事となり
不具合が生じる。

C＋＋からCの関数を使うのは割と簡単で、作法が決められているが、逆は色々問題がありそう。

ということで、通信用の関数もCの関数として書いてリンクするのが正しいやり方です。
