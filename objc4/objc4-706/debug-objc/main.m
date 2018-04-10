//
//  main.m
//  debug-objc
//
//  Created by wangqiqi on 2017/12/21.
//

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>

@interface NSObject (Test)

- (void)foo;
//+ (void)foo;

@end

@implementation NSObject (Test)

- (void)foo {
    NSLog(@"call instance foo");
}

@end

@interface DWRuntimeTest: NSObject

@property (nonatomic, copy) NSString *name;
- (void)say;
//+ (void)test;

@end

@implementation DWRuntimeTest

- (void)say {
    NSLog(@"name is %@", self.name);
}
//
//+ (void)test {
//    NSLog(@"class method test.");
//}

@end

@interface DWSubRuntimeTest: DWRuntimeTest

@end

@implementation DWSubRuntimeTest

- (instancetype)init {
    if (self = [super init]) {
        NSLog(@"call [self class] %@", NSStringFromClass([self class]));
        NSLog(@"call [super class] %@", NSStringFromClass([super class]));
    }
    return self;
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        BOOL res1 = [(id)[NSObject class] isKindOfClass:[NSObject class]];
        BOOL res2 = [(id)[NSObject class] isMemberOfClass:[NSObject class]];
        
        BOOL res3 = [(id)[DWRuntimeTest class] isKindOfClass:[DWRuntimeTest class]];
        BOOL res4 = [(id)[DWRuntimeTest class] isMemberOfClass:[DWRuntimeTest class]];
        
        NSLog(@"res1 = %d, res2 = %d, res3 = %d, res4 = %d",res1,res2,res3,res4);
        
        
        NSObject *obj = [[NSObject alloc] init];
        
        id __weak obj1 = obj;
        NSLog(@"1 %@",obj1);
        NSLog(@"2 %@",obj1);
        NSLog(@"3 %@",obj1);
        NSLog(@"4 %@",obj1);
        NSLog(@"5 %@",obj1);
        NSLog(@"6 %@",obj1);
        NSLog(@"7 %@",obj1);
        
        _objc_autoreleasePoolPrint();
        
        NSLog(@"%d",_objc_rootRetainCount(obj));
        
        
        // 如果方法实现在NSObject 的子类会crash
        // +[DWSubRuntimeTest foo]: unrecognized selector sent to class 0x1000013a0
        // [DWSubRuntimeTest foo];
        
        // 如果实现在NSObject 中是不会carsh
        // [DWSubRuntimeTest foo];
        
//        self = ((DWSubRuntimeTest *(*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("DWSubRuntimeTest"))}, sel_registerName("init")))
        
        
        id cls = [DWRuntimeTest class];
        
//        void *obj = &cls;
//        [(__bridge id)obj say];
        
    }
    return 0;
}



