# Albinos

Raven-OS' configuration unification service and library.

## Building & installing

```bash
mkdir -p build/<build_type>
cd build/<build_type>
cmake ../.. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=<BuildType> # cmake wants a CamelCase value, so "Debug" for debug, "Release" for release etc.
make
sudo make install
cd -
```
The compilation may take a while.

## Running

To run the service, use:
```bash
./service/service
```

## Config editor installation

First you need to install nimble.

You can use **choosenim** for installing nim environment:

```bash
curl https://nim-lang.org/choosenim/init.sh -sSf | sh
choosenim update devel #use development nim version
choosenim devel #switch to devel version
```

Then you should:
```bash
nimble build
nimble install
```

## Running the config editor

```bash
albinos_editor
```

## Documentation

See [our doc](https://docs.raven-os.org/p/albinos/master/index.html)!
