# pgen
A password generator utility:
```bash
  pgen 10
  X7+})KoO/q
```
# BUILD and INSTALL
You can easily *build*, *install*, *clean* and *uninstall* the binary by calling make with the respective step name:
```bash
  make [*step*]
```

# TODO:
- Could be interesting to build a binary without dependencies
- Fuzz the binary
- Polish code
- ~~Take advantage of multicore CPUs systems~~
- ~~Hardcode and dinamically build the alphabet~~
- ~~Extract a buffer of n bytes from /dev/random and work with it~~
- ~~Better flag parsing~~
- ~~Add O(1) exclude filter~~
- ~~Fix infinite loop if custom alphabet and exclusion alphabet are equal~~
- ~~improve build allphabet logic to create an alphabet made out of already filtered values and custom alphabet. (NOTE: *this allow to perform a check on the alphabet len, if 0 return error*)~~
