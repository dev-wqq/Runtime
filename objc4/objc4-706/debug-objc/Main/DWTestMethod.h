//
//  DWTestMethod.h
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import <Foundation/Foundation.h>
#import "DWFather.h"

#pragma mark - 经典Runtime四个问题
/**
 问题1：下面代码会输出什么？
 - (instancetype)init {
    if (self = [super init]) {
        NSLog(@"%@",NSStringFromClass([self class]));
        NSLog(@"%@",NSStringFromClass([super class]));
    }
    return self;
 }
 */
extern void testSelfAndSuperDiff(void);

/**
 问题2：下面的代码的结果？
 BOOL res1 = [(id)[NSObject class] isKindOfClass:[NSObject class]];
 BOOL res2 = [(id)[NSObject class] isMemberOfClass:[NSObject class]];
 BOOL res3 = [(id)[DWSon class] isKindOfClass:[DWSon class]];
 BOOL res4 = [(id)[DWSon class] isMemberOfClass:[DWSon class]];
 */
extern void testKindOfClassAndMemberOfClassDiff(void);

/**
 问题3：下面的代码会？Compile Error/ Runtime Crash/ NSLog...?
 interface NSObject (RuntimeMethod)
 + (void)dw_foo;
 @end
 
 @implementation NSObject (RuntimeMethod)
 + (void)dw_foo {
    NSLog(@"call %@",NSStringFromSelector(_cmd));
 }
 [NSObject dw_foo];
 [[NSObject new] dw_foo];
 @end
 */
extern void testCallInstanceAndClassMethodDiff(void);

/**
 问题4:下面的代码会？Compile Error/ Runtime Crash/ NSLog...?
 id cls = [DWSon class];
 void *obj = &cls;
 [(__bridge id)obj dw_inputMyName];
 */
extern void testPointerCallMethod(void);

@interface DWTestMethod : NSObject

@end
