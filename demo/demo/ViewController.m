//
//  ViewController.m
//  demo
//
//  Created by flexih on 2020/2/2.
//  Copyright © 2020 flexih. All rights reserved.
//

#import "ViewController.h"

@protocol UsedProtocol <NSObject>

- (void)protoMethUsed;

@end

@interface UsedClass : NSObject<UsedProtocol>

@end

@implementation UsedClass

- (void)protoMethUsed {
    
}

- (void)usedMeth {
    
}

- (void)unusedMeth {
    
}

@end

@interface UnusedClass : NSObject

@end

@implementation UnusedClass

- (void)unusedMethOfUnusedClass {
    
}

@end

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    UsedClass *uc = [UsedClass new];
    [uc usedMeth];
}


@end
