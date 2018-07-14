  ### Build Runtime
  提供了编译成功的 objc4-706 和 objc4-723（with AppleSources 依赖库，这里是为了方便查找一些在runtime中只声明没有实现的函数）.
 
 1.编译 runtime 的好处
  * 可以直接通过 debug 调试 runtime, 有助于理解 runtime 的底层实现原理；
 
 2.最近一周通过动手去调试 [孙源大佬的神经病院objc runtime入院考试](https://blog.sunnyxx.com/2014/11/06/runtime-nuts/)，收获不小，对runtime的理解更加深刻，后续规划 debug 调试：
  * 调试消息转发&消息传递；
  * runtime 建立 对象-类-元类 三者之间关系时机；
  * main 函数之前 runtime 做了什么；
  * load 方法的加载；
  * 分类添加方法、属性、协议得处理；
  * 等等；
  




  



 
