//
//  DWTestObject.h
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import <Foundation/Foundation.h>

@interface NSObject (Test)

- (void)dw_test;

@end

@interface DWTestObject : NSObject

- (void)testMethod;

- (void)testMethod:(NSString *)str;

+ (void)testMethod;

+ (void)testMethod:(NSString *)str;

@end

@interface DWSubTestObject: DWTestObject

- (void)subTestMethod;

- (void)subTestMethod:(NSString *)str;

+ (void)subTestMethod;

+ (void)subTestMethod:(NSString *)str;

@end
