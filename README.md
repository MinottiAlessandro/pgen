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
- Start optimize the logic, some of the techniques might be:
    - Hardcode and dinamically build the alphabet
    - Extract a buffer of n bytes from /dev/random and work with it
    - ~~Better flag parsing~~
    - ~~Add O(1) exclude filter~~
    - Take advantage of multicore CPUs systems
    - Could be interesting to build a binary without dependencies
    - Add tests
    - Fuzz the binary
