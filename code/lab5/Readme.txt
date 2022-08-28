Lab5 具有二级索引的文件系统
2021.10.31

Nachos系统原有的文件系统只支持单级索引，最大能存取NumDirect * SectorSize = 30 * 128 = 3840字节大小的文件。本实验将在理解原文件系统的组织结构基础上扩展原有的文件系统，设计并实现具有二级索引的文件系统。

本目录下的可执行程序n5为Lab5的一个参考实现。
其实现的一些具体细节如下：

一级索引块共有索引项数目：
    (SectorSize - 3 * sizeof(int)) / sizeof(int)。对于128字节的扇区，int为32比特时，为29项索引

二级索引块共有索引项数目：
    (SectorSize / sizeof(int))。对于128字节的扇区，int为32比特时，为32项索引

实现的二级索引的文件系统，单个文件最大可达(2 * NumDirect + 1) * SectorSize = 61 * 128 = 7808字节

在FileHeader类的析构函数~FileHeader中，判断文件长度改变时自动写回文件头


可依次输入下列命令，进行测试：
./nachos -f
./nachos -cp test/huge huge
./nachos -D
./nachos -ap test/huge huge
./nachos -D
./nachos -ap test/huge huge
./nachos -D
