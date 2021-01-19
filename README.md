[![license](https://img.shields.io/badge/license-isc-blue.svg)](https://github.com/hello2al/gamma-ray/blob/master/LICENSE)

# Gamma Ray

**WARNING: The author of this software will not be responsible for your losses on the market, use at your own discretion.**

## Introduction

Gamma Ray is a high frequnecy market maker trading bot for crypto currency markets written in C++. It is able to consistantly update maker orders under 100 micro seconds.

This program is aimes to be a starting point for developing your own market maker algorithms, and it comes with 3 existing strategies for you to get started:

- Top, which tries to post 1 tick better than the existing best bid and ask prices in the orderbook, given a minimum spread contraint.
- Mid, which post bid/ask orders with a fixed spread.
- Avellaneda Stoikov, which is a high frequency market maker framework with a proper model, for more information you may read the paper [here](https://www.researchgate.net/publication/24086205_High_Frequency_Trading_in_a_Limit_Order_Book). A jupyter notebook `doc/AS model calibration.ipynb` is provided giving a sample method to calibrate model parameters.

As of now this program only supports one exchange: Bitmex, more exchanges may be added in the future depend on user request.

## Build Instructions
This program is tested on Ubuntu 18, other Linux distributions may differ from this instruction.

### Install dependencies
``` bash
# install dependent packages
sudo apt-get update
sudo apt-get -y install build-essential doxygen graphviz ninja-build libboost-all-dev libssl-dev libcpprest-dev

# install cmake 3.18
wget https://github.com/Kitware/CMake/releases/download/v3.18.0-rc1/cmake-3.18.0-rc1.tar.gz && tar -xzvf cmake-3.18.0-rc1.tar.gz && cd cmake-3.18.0-rc1
./bootstrap && make
sudo make install
cd .. && rm -rf cmake-3.18.0-rc1
source

# install poco
git clone -b master https://github.com/pocoproject/poco.git
cd poco
mkdir cmake-build && cd cmake-build
cmake ..
sudo cmake --build . --target install
cd ../.. && rm -rf poco
```

### Build

```bash
git clone --recurse-submodules https://github.com/hello2all/gamma-ray
cd gamma-ray

mkdir release
cd release

cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

## Configuration

A sample configuration `sample-config.json` is provided, after editing according to your parameters this config file is required to be renamed to `gamma-ray.json` and copied into the same directory as the executable.

```bash
cp ./sample-config.json ./release/gamma-ray.json
```

A brief explaination of configurable parameteres:

- **websocketUrl**: Websocket API end point 
- **httpUrl**: REST API end point
- **apiKey**: API key
- **apiSecret**: API secret
- **contract**: trading pair, currently supports only perpetual contracts such as `BTCUSD`, `ETHUSD` etc.
- **quotingMode**: quoting strategy, you can choose from `Mid`, `Top` and `AvellanedaStoikov`.
- **size**: order size/quantity
- **quotingModeParam**: quoting strategy specific parameters
- **pairs**: how many price levels of bid ask pairs will be posted
- **priceInterval**: price difference between price levels
- **sizeIncrement**: size difference between price levels
- **skewFactor**: for each executed order size, mid price will move one tick against the executed side
- **tradesPerMinute**: safety feature, maximum orders can be executed before stop quoting, not yet implemented

## Run

With `gamma-ray.json` config file in the same directory with the executable

```bash
./release/main
```

## Deployment

For the best latency, this bot should be deployed on AWS `eu-west-1c`.

## Donations❤️

This project is still evolving and need your help to grow. If you find this project useful, please consider support the development of this project by donating using the following addresses. Testing HFT strategies replies on trading on live markets due to many technical constrains, which can be costly. Your contribution will keep this project free and open to the public.

BTC: 3BMEXLjaNku2Sh73s2i1DZVTgKpDrzTSwk

ETH: 0x9DFe1C3E906Ec27121e7921953CEdA5AaB558306

Donators will have early access to new features and your feature requests will be prioritised.

## Join the community

If you have questions or would like to contribute to the project, join our Discord channel. It's gonna be a sausage party, I promise :p

https://discord.gg/ZAG6aCXfCs
