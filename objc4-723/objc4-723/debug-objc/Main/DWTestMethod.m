//
//  DWTestMethod.m
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import "DWTestMethod.h"
#import <objc/message.h>
#import <objc/runtime.h>

#pragma mark - 经典Runtime四个问题

@implementation DWTestMethod

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
+ (void)testSelfAndSuperDiff {
    DWSon *son = [[DWSon alloc] init];
    son.name = @"xxxyyy";
}

 /**
 + (Class)class {
    return self;
 }
 
 - (Class)class {
    return object_getClass(self);
 }
 
 Class object_getClass(id obj)
 {
    if (obj) return obj->getIsa();
    else return Nil;
 }
 
 inline Class
 objc_object::getIsa()
 {
    if (!isTaggedPointer()) return ISA();
 }
 
 inline Class
 objc_object::ISA()
 {
    assert(!isTaggedPointer());
    return (Class)(isa.bits & ISA_MASK);
 }
 
 + (BOOL)isMemberOfClass:(Class)cls {
    return object_getClass((id)self) == cls;
 }
 
 - (BOOL)isMemberOfClass:(Class)cls {
    return [self class] == cls;
 }
 
 + (BOOL)isKindOfClass:(Class)cls {
    for (Class tcls = object_getClass((id)self); tcls; tcls = tcls->superclass) {
        if (tcls == cls) return YES;
    }
    return NO;
 }
 
 - (BOOL)isKindOfClass:(Class)cls {
    for (Class tcls = [self class]; tcls; tcls = tcls->superclass) {
        if (tcls == cls) return YES;
    }
    return NO;
 }
 */
/**
 问题2：下面的代码的结果？
 */
+ (void)testKindOfClassAndMemberOfClassDiff {
    BOOL res1 = [(id)[NSObject class] isKindOfClass:[NSObject class]];
    BOOL res2 = [(id)[NSObject class] isMemberOfClass:[NSObject class]];
    BOOL res3 = [(id)[DWFather class] isKindOfClass:[DWFather class]];
    BOOL res4 = [(id)[DWFather class] isMemberOfClass:[DWFather class]];
    
    NSLog(@"[(id)[NSObject class] isKindOfClass:[NSObject class]] ------> %@",@(res1));
    NSLog(@"[(id)[NSObject class] isMemberOfClass:[NSObject class]] ----> %@",@(res2));
    NSLog(@"[(id)[DWFather class] isKindOfClass:[DWFather class]] ------> %@",@(res3));
    NSLog(@"[(id)[DWFather class] isMemberOfClass:[DWFather class]] ----> %@",@(res4));
    // 1 0 0 0
}

/**
 问题3：下面的代码会？Compile Error/ Runtime Crash/ NSLog...?
 interface NSObject (RuntimeMethod)
 + (void)dw_foo;
 @end
 
 @implementation NSObject (RuntimeMethod)
 + (void)dw_foo {
 NSLog(@"call %@",NSStringFromSelector(_cmd));
 }
 @end
 */
+ (void)testCallInstanceAndClassMethodDiff {
    [NSObject dw_foo];
    //    [[NSObject new] dw_foo];
}

/**
 问题4:下面的代码会？Compile Error/ Runtime Crash/ NSLog...?
 */
+ (void)testPointerCallMethod {
    id cls = [DWSon class];
    void *obj = &cls;
    [(__bridge id)obj dw_inputMyName];
}

- (void)testPointerCallMethod {
    id cls = [DWSon class];
    void *obj = &cls;
    [(__bridge id)obj dw_inputMyName];
}

/**
 Objc 中的对象是一个指向 ClassObject 地址的变量，即 obj = &ClassObject，而对象的实例变量 void *ivar = &obj + offset(N)
 变量入栈顺序从高到低
 每个指针占8个字节
 */
- (instancetype)initTestMemoryAddress {
    NSLog(@"self = %p  _cmd = %p", &self, &_cmd);
    struct objc_super superStruct = {self, class_getSuperclass(objc_getClass("DWTestMethod"))};
    NSLog(@"superStruct.receiver = %p  superStruct.super_class = %p", &superStruct.receiver, &superStruct.super_class);
//    if (self = [super init]) {
    
        id cls = [DWFather class];
        NSLog(@"DWFather Class = %@, 地址 = %p", cls, &cls);
        void *obj = &cls;                  // 指向DWSon class的指针
        NSLog(@"Void *obj = %@, 地址 = %p", obj, &obj);
        [(__bridge id)obj dw_inputMyName]; //（__bridge id）使得 obj 转换成了 objc_object 的类型
    
        NSLog(@"\n");
        DWFather *father = [[DWFather alloc] init];
        NSLog(@"father instance = %@, 地址 = %p", father, &father);
        [father dw_inputMyName];
//    }
    return self;
}

@end
