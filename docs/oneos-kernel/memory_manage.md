# 内存管理

---

在计算机系统中，应用程序需要的内存大小需要在程序运行过程中根据实际情况确定，因此系统需要提供对内存空间进行动态管理的能力。OneOS提供了内存堆和内存池两种方式管理内存。

内存堆管理主要实现动态内存的管理，主要包括内存的申请和释放操作。用户可以根据实际需求申请不同大小的内存，当用户不再使用时，可以将申请的内存释放回内存堆中。

内存池适用于分配大量大小相同的内存块的场景，它能够快速的分配和释放内存，且能够避免内存碎片化问题。