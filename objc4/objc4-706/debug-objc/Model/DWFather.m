//
//  DWFather.m
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import "DWFather.h"
#import <objc/runtime.h>
#import <objc/message.h>
#import "message.h"

@implementation DWFather

- (void)dw_inputMyName {
    NSLog(@"my name's %@, %p",self.name, &self->_name);
}

@end

@implementation DWSon

/**
 clang -rewrite-objc DWFather.m 转换成 C++
 static instancetype _I_DWSon_init(DWSon * self, SEL _cmd) {
    if (self = ((DWSon *(*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("DWSon"))}, sel_registerName("init"))) {
        Class selfClass = ((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("class"));
        Class superClass = ((Class (*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("DWSon"))}, sel_registerName("class"));
        NSLog((NSString *)&__NSConstantStringImpl__var_folders_n9_wgmqtq757m308yt5wphqp6_hn7jykz_T_DWFather_299b8e_mi_1,NSStringFromClass(selfClass));
        NSLog((NSString *)&__NSConstantStringImpl__var_folders_n9_wgmqtq757m308yt5wphqp6_hn7jykz_T_DWFather_299b8e_mi_2,NSStringFromClass(superClass));
    }
    return self;
 }
 
 Class selfClass = ((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("class"));
 Class superClass = ((Class (*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("DWSon"))}, sel_registerName("class"));
 */
- (instancetype)init {
    if (self = [super init]) {
        Class selfClass = [self class];
        Class superClass = [super class];
        NSLog(@"%@",NSStringFromClass(selfClass));
        NSLog(@"%@",NSStringFromClass(superClass));

        // [self class]
        selfClass = objc_msgSend(self, sel_registerName("class"));
        /**
         [super class]
         
         super_class is the first class to search.
         通过实践可知，objc_msgSendSuper 的工作原理是：从 objc_super 中的 super_class 指向的父类方法列表开始查找 Selector，
         找到方法以后，使用 objc_super 中的 receiver 去调用父类中的这个 Selector.
         调用者是 objc_super->receiver。
         */
        struct objc_super superStruct = {self, class_getSuperclass(objc_getClass("DWSon"))};
        superClass = objc_msgSendSuper(&superStruct, sel_registerName("class"));
        
        NSLog(@"---> %@",NSStringFromClass(selfClass));
        NSLog(@"---> %@",NSStringFromClass(superClass));
    }
    return self;
}

@end

@implementation NSObject (RuntimeMethod)

+ (void)dw_foo {
    NSLog(@"call dw_foo");
}

@end
