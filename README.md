# Snake
🐍 Snake, Yet Another Mach-O Unused ObjC Selector/Class/Protocol Detector.

ObjC Metadata
* Classes ✔
* Protocols ✔
* Methods ✔
* Categories ✔
* Binding Info ✔
* ObjC-Specific Sections
	* `__objc_selrefs` ✔
	* `__objc_superrefs` ✔
	* `__objc_classrefs` ✔
	* `__objc_classlist` ✔
	* `__objc_catlist` ✔
	* `__objc_protolist` ✔

See [SnakeKit][1]
## Features
- [x] Parse Mach-O directly, no Symbols option required in Strip style of Build Settings, __no depends on otool__.
- [x] Unused selectors.
- [x] Unsued classes.
- [x] Unused protocols.
- [x] Duplicate selectors.
- [x] Selector/Classes/Protocols sort by library, and selector size, if Linkmap file provided.
- [x] Fast, a 460.6M binary and a 134.3M linkmap file costs 1.62s(3.7 GHz 6-Core Intel Core i5;40 GB 2667 MHz DDR4).

## Homebrew
brew tap flexih/tap && brew install snake

## How To Use
```
Usage:
  snake [-dscp] [-l path] path/to/binary ...

  -s, --selector     Unused selectors
  -c, --class        Unused classes
  -p, --protocol     Unused protocoles
  -d, --duplicate    Duplicate selectors
  -l, --linkmap arg  Linkmap file, which has selector size, library name
  -j, --json         Output json format
      --help         Print help
```

snake -l path/to/linkmap path/to/binary [-dscp]

## Example
bin/snake  -l demo/release/demo-LinkMap-normal-x86\_64.txt demo/release/demo.app/demo -c
```
Total Lib Count: 1
Total Unused Class Count: 3

# demo

SceneDelegate
UnusedClass
ViewController
```
bin/snake -l demo/release/demo-LinkMap-normal-x86\_64.txt demo/release/demo.app/demo -s
```
Total Lib Count: 1
Total Class Count: 2
Total Unused Selector: 2

# demo

@ UnusedClass
-[UnusedClass unusedMethOfUnusedClass]  6

@ UsedClass
-[UsedClass unusedMeth] 6
```

## Storyboard
Used selectors
```
python2 storyboard.py path/to/x.storyboard
```

## Source

git clone --recursive https://github.com/flexih/Snake.git

## Details

[Objective-C二进制瘦身][2]

## Credits

* [cxxopts][3]

[1]:	https://github.com/flexih/SnakeKit
[2]:	https://www.jianshu.com/p/e3cf048c67aa
[3]:	https://github.com/jarro2783/cxxopts