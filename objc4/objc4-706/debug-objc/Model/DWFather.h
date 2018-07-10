//
//  DWFather.h
//  debug-objc
//
//  Created by 王启启 on 2018/7/10.
//

#import <Foundation/Foundation.h>

@interface DWFather : NSObject

@property (nonatomic, copy) NSString *name;

- (void)dw_inputMyName;

@end

@interface DWSon: DWFather

@end


@interface NSObject (RuntimeMethod)

+ (void)dw_foo;

@end

