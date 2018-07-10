//
//  DWFather.m
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import "DWFather.h"

@implementation DWFather

- (void)dw_inputMyName {
    NSLog(@"my name's %@",self.name);
}

@end

@implementation DWSon

// 问题1：下面代码会输出什么？
- (instancetype)init {
    if (self = [super init]) {
        NSLog(@"%@",NSStringFromClass([self class]));
        NSLog(@"%@",NSStringFromClass([super class]));
    }
    return self;
}

@end

@implementation NSObject (RuntimeMethod)

+ (void)dw_foo {
    NSLog(@"call dw_foo");
}

@end
