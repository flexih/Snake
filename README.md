# Snake
🐍 Snake, Yet Another Mach-O Unused ObjC Selector/Class/Protocol Detector.

## Features
- [x] No Symbols option required in Strip style of Build Settings
- [x] Unused selectors
- [x] Unsued classes
- [x] Unused protocols
- [x] Selector/Classes/Protocols sort by library, and selector size,  if  Linkmap file provided
- [x] Performance

## How To Use
```
Usage:
  snake [-scp] [-l path] mach-o ...

  -s, --selector     Unused selectors
  -c, --class        Unused classes
  -p, --protocol     Unused protocoles
  -l, --linkmap arg  Linkmap file, which has selector size, library name
  -j, --json         Output json format
      --help         Print help
```

## Example
bin/snake  -l demo/release/demo-LinkMap-normal-x86_64.txt demo/release/demo.app/demo -c
```
Total Lib Count: 1
Total Unused Class Count: 3

# demo

SceneDelegate
UnusedClass
ViewController
```
bin/snake -l demo/release/demo-LinkMap-normal-x86_64.txt demo/release/demo.app/demo -s
```
Total Lib Count: 1
Total Class Count: 2
Total Unused Selector: 2

# demo

@ UnusedClass
-[UnusedClass unusedMethOfUnusedClass]	6

@ UsedClass
-[UsedClass unusedMeth]	6
```

## Source

git clone --recursive https://github.com/flexih/Snake.git

## Credits

* [cxxopts](https://github.com/jarro2783/cxxopts)

