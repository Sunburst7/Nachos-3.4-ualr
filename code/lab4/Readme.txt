Lab4 基本文件系统扩展
2021.10.31

本目录下的可执行程序n4a、n4b、n4c、n4d为Lab4的4个略微不同的参考实现。

n4a：
  在OpenFile类中，增加WriteBack方法，手动调用此方法写回文件头

n4b：
  在OpenFile类析构函数~OpenFile中自动写回文件头(不判断文件长度是否已改变)

n4c：
  在OpenFile类的析构函数~OpenFile中，判断文件长度改变时自动写回文件头

n4d：
  在FileHeader类的析构函数~FileHeader中，判断文件长度改变时自动写回文件头


可依次输入下列命令，进行测试：
./nachos -f
./nachos -cp test/big big
./nachos -D
./nachos -ap test/small big
./nachos -D
./nachos -hap test/small big
./nachos -D
