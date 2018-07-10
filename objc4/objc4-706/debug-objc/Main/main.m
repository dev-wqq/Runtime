//
//  main.m
//  debug-objc
//
//  Created by wangqiqi on 2017/12/21.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#import "DWTestObject.h"
#import "DWTestMethod.h"

extern void instrumentObjcMessageSends(BOOL);
/**
    备忘快捷搜索：
    类的定义
    typedef struct objc_class *Class;
 */

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");

        instrumentObjcMessageSends(YES);
        DWSubTestObject *subObject = [[DWSubTestObject alloc] init];
        [subObject testMethod];
        [subObject testMethod];
        
#pragma mark - 直接调方法
        // 不用走消息转发机制
        void (*dw_directCallMethod)(id, SEL);
        dw_directCallMethod = (void (*)(id,SEL))[subObject methodForSelector:@selector(dw_test)];
        dw_directCallMethod(subObject, @selector(dw_test));
        
//        _objc_autoreleasePoolPrint();
//        _objc_rootRetainCount(subObject);
#pragma mark - 经典Runtime四个问题
        // self & super 的区别
        testSelfAndSuperDiff();
        
        // isKindOfClass: & isMemberOfClass 的区别
        testKindOfClassAndMemberOfClassDiff();
        
        // 实例方法 & 类方法 调用的区别
        testCallInstanceAndClassMethodDiff();
        
//        testPointerCallMethod();
    }
    return 0;
}



