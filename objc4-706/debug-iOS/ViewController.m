//
//  ViewController.m
//  debug-iOS
//
//  Created by 王启启 on 2018/7/11.
//

#import "ViewController.h"
#import <objc/message.h>
#import <objc/runtime.h>

@interface DWMemoryTestObject: NSObject

@property (nonatomic, copy) NSString *name;

- (void)dw_inputName;

@end

@implementation DWMemoryTestObject

- (void)dw_inputName {
    NSLog(@"input name %@ %p", self.name, self.name);
}

@end



@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    NSLog(@"self  = %@, address = %p", self, &self);
    [super viewDidLoad];
    
    NSLog(@"\nself = %p\n_cmd = %p\n", &self, &_cmd);
    
    NSLog(@"self  = %@, address = %p", self, &self);
    id cls = [DWMemoryTestObject class];
    NSLog(@"Class = %@, address = %p", cls, &cls);
    void *obj = &cls;                  // 指向DWSon class的指针
    NSLog(@"Void  = %@, address = %p", obj, &obj);
    [(__bridge id)obj dw_inputName];   //（__bridge id）使得 obj 转换成了 objc_object 的类型
    
    // Objc 中的对象是一个指向 ClassObject 地址的变量，即 obj = &ClassObject，而对象的实例变量 void *ivar = &obj + offset(N)
    
    
    NSLog(@"\n");
    DWMemoryTestObject *testObject = [[DWMemoryTestObject alloc] init];
    NSLog(@"father instance = %@, 地址 = %p", testObject, &testObject);
    [testObject dw_inputName];
    
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
