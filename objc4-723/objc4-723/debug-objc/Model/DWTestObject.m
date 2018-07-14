//
//  DWTestObject.m
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import "DWTestObject.h"

@implementation NSObject (Test)

- (void)dw_test {
//    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

@end



@implementation DWTestObject

- (void)testMethod {
    //    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

- (void)testMethod:(NSString *)str {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

+ (void)testMethod {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

+ (void)testMethod:(NSString *)str {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

@end


@implementation DWSubTestObject

- (void)subTestMethod {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

- (void)subTestMethod:(NSString *)str {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

+ (void)subTestMethod {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

+ (void)subTestMethod:(NSString *)str {
    NSLog(@"call %@", NSStringFromSelector(_cmd));
}

@end
