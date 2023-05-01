# 构建你自己的插件框架
## 一个跨平台的插件框架 for C/C++
### 谁需要插件？

如果您想开发成功且动态的，插件是必经之路系统。基于插件的可扩展性是当前扩展的最佳实践和以安全的方式发展系统。插件让第三方开发人员添加对系统的价值，并让内部开发人员添加无风险的功能破坏核心功能的稳定。插件促进分离关注，保证实施细节隐藏，孤立的测试以及许多其他最佳实践。Eclipse之类的平台实际上是所有的框架框架功能由插件提供。日食本身（包括UI和Java开发环境）只是一组插件进入核心框架。

### 为什么c++ ?

当涉及到插件时，c++是出了名的不随和。c++是
非常特定于平台和编译器。c++标准
没有指定应用程序二进制接口(ABI)，这意味着
来自不同编译器甚至不同版本的c++库
编译器不兼容。再加上c++没有概念的事实
动态加载和每个平台提供自己的解决方案(不兼容)
和其他人一起)，你就知道了。有一些重量级的解决方案
它试图解决的不仅仅是插件，而是依赖于一些额外的
运行时支持。尽管如此，C/ c++通常是唯一实用的选择
谈到高性能系统。

基本的解决方案都没有解决与之相关的无数问题
创建一个工业强度的基于插件的系统，如错误处理，数据
类型、版本、框架代码和应用程序之间的分离
代码。在深入研究解决方案之前，让我们先了解一下问题。

### 二进制兼容性问题
同样，没有标准的c++ ABI。不同的编译器(甚至
同一编译器的不同版本)产生不同的目标文件和
库。这个问题最明显的表现就是差异
由不同编译器实现的名称混淆算法。这意味着
一般来说，你只能链接c++对象文件和库
使用完全相同的编译器(品牌和版本)编译。许多
编译器甚至没有实现c++ 98的标准c++特性
对于这个问题有一些局部的解决办法。例如，如果您访问
一个c++对象只能通过虚指针，并且只能调用它的虚方法
你回避了名字混淆的问题。然而，不能保证
甚至在不同的编译器之间，内存中的虚拟表布局也是相同的，
虽然它更稳定。
如果您尝试动态加载c++代码，您将面临另一个问题——没有
的动态库中加载和实例化c++类的直接方法
Linux或Mac OS X (Windows下支持Visual c++)。
这个问题的解决方案是使用带有C链接的函数(不是名称)
被编译器破坏)作为一个工厂函数返回一个不透明的
调用者的句柄。然后调用者将句柄强制转换为适当的类
(通常是纯抽象基类)。这需要一些协调
当然，并且仅在使用编译库和应用程序时才有效
在内存中具有匹配虚函数表布局的编译器。
兼容性的终极目标是忘掉c++，公开一个纯的
C API。实际上，C在所有编译器实现之间是兼容的。
稍后我将展示如何在C之上实现c++编程模型
兼容性。

### 基于插件的系统架构
基于插件的系统可以分为三个部分:
- 特定于领域的系统
- 一个插件管理器
- 插件

特定于领域的系统通过加载插件并创建插件对象
插件管理器。一旦一个插件对象被创建，主系统有
一些指向它的指针/引用，它可以像任何其他对象一样使用。
通常，我们会有一些特殊的销毁/清理要求
看到的。
插件管理器是一段相当通用的代码。它管理插件的生命周期并将它们公开给主系统。它可以找到
加载插件，初始化它们，注册工厂函数，并能够卸载
插件。它还应该让主系统迭代加载的插件或
注册插件对象。
插件本身应该遵守插件管理器协议和
提供符合主要系统期望的对象。
实际上，在基于c++的插件中很少看到如此清晰的分离
系统)。插件管理器通常与插件管理器紧密耦合
领域特定的系统。这是有充分理由的。插件管理
需要提供特定类型的插件对象的最终实例。
此外，插件的初始化通常需要传递域特定信息和/或回调函数/服务。这是不可能的
使用一个通用的插件管理器很容易。

插件部署模型
插件通常作为动态库部署。动态库允许
插件的许多优点，如热插拔(重新加载新的
实现无需关闭系统)，可由第三方安全扩展
开发人员(不修改核心系统的附加功能)，以及
更短的链接时间。然而，在某些情况下，静态库是
插件的最佳载体。例如，有些系统根本不支持
动态库(许多嵌入式系统)。在其他情况下，安全
关注点不允许加载外部代码。有时是核心系统
它预装了一些插件，并且静态地更健壮
将它们链接到主可执行文件(这样用户就不会意外地删除它们)。
底线是一个好的插件系统应该同时支持这两种动态
静态插件。这使您可以部署相同的插件系统
不同的环境有不同的约束。

插件编程接口
插件都是关于接口的。基于插件的系统的基本概念是
有一些中央系统加载插件，但它对插件一无所知
并通过定义良好的接口和协议与它们进行通信。
简单的方法是定义一组函数作为接口
插件导出(动态或静态库)。这种方法在技术上
有可能，但在概念上是有缺陷的。原因是有两种
一个插件应该支持并且只能是一组接口
从插件导出的函数。这意味着这两种接口
会混在一起。
第一个接口(和协议)是通用插件接口。它让
中央系统初始化插件，并让插件注册到
中央系统的各种功能，创建和销毁对象以及
作为全局清理函数。通用插件接口不是特定于域的，可以指定和实现为可重用库。的
第二个接口是插件实现的功能接口
对象。该接口是特定于领域的，必须仔细设计
并由实际的插件实现。中央系统应该意识到了
并通过它与插件对象进行交互。
清单1是指定通用插件接口的头文件。
在不深入研究细节和解释任何事情的情况下，我们只是
看看它能提供什么。

```
#ifndef PF_PLUGIN_H
#define PF_PLUGIN_H

#include <apr-1/apr_general.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PF_ProgrammingLanguage
{
    PF_ProgrammingLanguage_C,
    PF_ProgrammingLanguage_CPP,
    PF_ProgrammingLanguage;
}PF_ProgrammingLanguage;

struct PF_PlatformServices;
typedef struct PF_ObjectParams
{
    const apr_byte_t* objectType;
    const struct PF_PlatformServices_ * platformServices;
}PF_ObjectParams;

typedef struct PF_PluginAPI_Version
{
    apr_int32_t major;
    apr_int32_t minor;
}PF_PluginAPI_Version;

typedef void* (*PF_CreateFunc)(PF_ObjectParams*);
typedef apr_int32_t (*PF_DestroyFunc)(void*);

typedef struct PF_RegisterParams
{
    PF_PluginAPI_Version version;
    PF_CreateFunc createFunc;
    PF_DestroyFunc destroyFunc;
    PF_ProgrammingLanguage programmingLanguage;
}PF_RegisterParams;

typedef apr_int32_t (*PF_RegisterFunc)(const apr_byte_t * nodeType,const PF_RegisterParams* params);
typedef apr_int32_t (*PF_InvokeServiceFunc)(const apr_byte_t* serviceName,void* serviceParams);

typedef struct PF_PlatformServices
{
    PF_PluginAPI_Version version;
    PF_RegisterFunc registerObject;
    PF_InvokeServiceFunc invokeService;
}PF_PlatformServices;

typedef apr_int32_t (*PF_ExitFunc)();
typedef PF_ExitFunc (*PF_InitFunc)(const PF_PlatformServices*);

#ifndef PLUGIN_API
#ifdef WIN32
#define PLUGIN_API __declspec(dllimport)
#else
#define PLUGIN_API
#endif
#endif

extern 
#ifdef __cplusplus "C"{
#endif

PLUGIN_API PF_ExitFunc PF_initPlugin(const PF_PlatformServices* params);

#ifdef __cplusplus
}
#endif

#endif 


#ifdef __cplusplus "C"{
}  /* PF_PLUGIN_H */
#endif

```

您应该注意到的第一件事是它是一个C文件。这允许插件
框架编译和纯C系统使用，并编写纯C
插件。但是，它并不局限于C语言，实际上是为大多数情况设计的
从c++。
PF_ProgrammingLanguage枚举允许插件声明
插件管理器，如果它们是用C或c++实现的。
PF_ObjectParams是一个抽象结构体，传递给创建的插件
对象。
PF_PluginAPI_Version用于协商版本并确保
插件管理器只加载兼容版本的插件。
函数指针
定义PF_CreateFunc和PF_DestroyFunc(由
插件)允许插件管理器创建和销毁插件对象(每个
Plugin向插件管理器注册这些函数)

PF_RegisterParams结构体包含插件所需要的所有信息
必须在初始化时提供给插件管理器(版本，
创建/销毁函数和编程语言)。
PF_RegisterFunc(由插件管理器实现)允许每个
插件为每个对象类型注册一个PF_RegisterParams结构体
支持。注意这个方案允许一个插件注册不同的版本
对象和多个对象类型的。
PF_InvokeService函数指针定义是一个泛型函数
插件可以用来调用主系统的服务，如日志记录，事件
通知和错误报告。签名包括服务名称和
指向形参结构体的不透明指针。插件应该知道
可用的服务以及如何调用它们(或者您可以实现服务)
如果您愿意，可以使用PF_InvokeService)。
PF_PlatformServices结构体聚合了我刚才提到的所有服务
提到平台提供给插件(版本、注册对象)
以及调用服务功能)。这个结构体被传递给每个插件
初始化时间。
PF_ExitFunc是插件退出函数指针的定义
(由插件实现)。
PF_InitFunc是插件初始化函数的定义
指针。
PF_initPlugin是插件初始化的实际签名
功能动态插件(插件部署在动态链接
库/共享库)。它是按名称从动态插件导出的，所以
插件管理器将能够在加载插件时调用它。它接受
一个指向PF_PlatformServices结构体的指针，所以所有的服务都是
在初始化时立即可用(这是注册的正确时间)
对象)，并返回一个指向退出函数的指针。
注意静态插件(在静态库中实现的插件)
直接到主可执行文件)应该用C实现init函数
链接，但千万不要命名为PF_initPlugin。原因是
有多个静态插件，它们都将具有相同的功能
名字和你的编译器会讨厌它。
静态插件初始化是不同的。它们必须显式初始化
由主可执行程序调用它们的初始化函数
pf_initfuncc签名。这是不幸的，因为它意味着主要
当一个新的静态插件出现时，可执行文件需要被修改

添加/删除以及各种init函数的名称必须是
协调。
有一种叫做“自动注册”的技术试图解决这个问题
问题。自动注册由静态对象中的全局对象完成
图书馆。这个对象应该在main()之前被构造
就开始了。这个全局对象可以请求插件管理器初始化
静态插件(传递插件的init()函数指针)。不幸的是,这
scheme在Visual c++中不起作用。

### 编写插件
写一个插件意味着什么?插件框架是非常通用的
并且不提供应用程序可以与之交互的任何有形对象。
您必须在插件之上构建应用程序对象模型
框架。这意味着您的应用程序(加载插件)和
插件本身必须同意并协调它们的交互
模型。通常这意味着应用程序期望插件提供的功能
公开某些特定API的特定类型的对象。插件
框架将提供注册所需的所有基础设施，
枚举并加载这些对象。例1是c++的定义
接口称为IActor。它有两个操作——getInitialInfo()和play()。
注意，这个接口是不够的，因为getInitialInfo()需要一个
指向结构体ActorInfo的指针，而play()需要一个指向yet的指针
另一个叫做itturn的界面。这通常是情况，您必须进行设计
并指定整个对象模型。

```
struct IActor
{
    virtual ~IActor() {}
    virtual void getIntialInfo(ActorInfo* info) = 0;
    virtual void play(ITurn* turnInfo) = 0;
};
```

每个插件都可以注册多个实现IActor接口的类型。
当应用程序决定实例化一个由插件注册的对象时，
它调用由插件实现的注册的PF_CreateFunc。的
插件负责创建一个相应的对象并将其返回给
应用程序。返回类型是void *，因为对象创建操作
是通用插件框架的一部分，不知道什么
特定的IActor接口。然后应用程序将void *强制转换为
一个IActor *，并且可以通过接口与它一起工作，就好像它是一个普通的
对象。当应用程序用完IActor对象后，它调用
注册的PF_DestroyFunc由插件和插件实现
销毁参与者对象。不要注意后面的虚拟析构函数
窗帘。

编程语言支持
在二进制兼容性部分，我解释了可以使用c++
如果您使用具有匹配虚表布局的编译器，则可以使用虚表级兼容性
应用程序和插件，或者您可以使用c级兼容性和
然后你可以使用不同的编译器来构建应用程序和插件，
但你仅限于C交互。应用程序对象模型必须是
基于c的。

### 纯 C
在纯C编程模型中，您只需用C开发插件
你实现了PF_CreateFunc函数，你返回一个C对象
与应用程序C对象模型中的其他C对象进行交互。是什么
所有这些都是关于C对象和C对象模型的。大家都知道C是a
过程语言，没有对象的概念。这是正确的和静止的
C有足够的抽象机制来实现对象，包括
多态性(在这种情况下是必要的)和支持面向对象
编程风格。事实上，最初的c++编译器实际上是C编译器的前端。它从后来的c++代码中生成C代码
使用普通C编译器编译。它的名字很能说明问题。
问题是使用包含函数指针的结构体。的签名
每个函数都应该接受自己的结构体作为第一个参数。该结构可以
还包含其他数据成员。这个简单的习语对应于c++
类并提供封装(状态和行为在一个地方)，
继承(通过使用基结构的第一个数据成员)，以及
多态性(通过设置不同的函数指针)。
C语言不支持析构函数、函数和操作符重载和
名称空间，以便在定义接口时选择更少。这可能
因祸得福，因为接口应该被其他人使用
可能掌握c++语言的不同子集的人。减少
接口中语言结构的作用域可以提高接口的简单性和易用性
界面的可用性。

如果你快速看一下，你会发现它甚至支持一种形式
普通对象之外的集合和迭代器。

```
#ifndef C_OBJECT_MODEL
#define C_OBJECT_MODEL
#include <apr-1/apr.h>
#define MAX_STR 64 /* max string length of string fields */
typedef struct C_ActorInfo_
{
    apr_uint32_t id;
    apr_byte_t name[MAX_STR];
    apr_uint32_t location_x;
    apr_uint32_t location_y;
    apr_uint32_t health;
    apr_uint32_t attack;
    apr_uint32_t defense;
    apr_uint32_t damage;
    apr_uint32_t movement;
} C_ActorInfo;

typedef struct C_ActorInfoIteratorHandle_ { 
    char c;
}* C_ActorInfoIteratorHandle;

typedef struct C_ActorInfoIterator_
{
    void (*reset)(C_ActorInfoIteratorHandle handle);
    C_ActorInfo* (*next) (C_ActorInfoIteratorHandle handle);
    C_ActorInfoIteratorHandle handle;
}C_ActorInfoIterator;

typedef struct C_TurnHandle_{
    char c;
}*C_TurnHandle;

typedef struct C_Turn_
{
    C_ActorInfo* (*getSelfInfo) (C_TurnHandle handle);
    C_ActorInfoIterator * (*getFriends)(C_TurnHandle handle);
    C_ActorInfoIterator* (*getFoes)(C_TurnHandle handle);
    void (*move)(C_TurnHandle handle,apr_uint32_t x,apr_uint32_t y);
    void (*attack)(C_TurnHandle handle,apr_uint32_t id);
    C_TurnHandle handle;
}C_Turn;

typedef struct C_ActorHandle_ {
    char c;
}*C_ActorHandle;

typedef struct C_Actor_
{
    void(*getInitialInfo) (C_ActorHandle handle,C_ActorInfo* info);
    void(*play) (C_ActorHandle handle,C_Turn* turn);
    C_ActorHandle handle;
}C_Actor;

#endif
```

### 纯 C++
在纯c++编程模型中，您只需用c++开发插件。
插件编程接口函数可以作为静态实现
成员函数或普通的静态/全局函数(c++主要是一个
毕竟是C的超集)。对象模型可以是各种各样的c++
对象模型。

```
#ifndef OBJECT_MODEL
#define OBJECT_MODEL
#include "c_object_model.h"
typedef C_ActorInfo ActorInfo;
struct IActorInfoIterator
{
    virtual void reset() = 0;
    virtual ActorInfo* next() = 0;
};

struct ITurn
{
    virtual ActorInfo * getSelfInfo() = 0;
    virtual IActorInfoIterator* getFriends() = 0;
    virtual IActorInfoIterator* getFoes() = 0;
    virtual void move(apr_uint32_t x,apr_uint32_t y) = 0;
    virtual void attack(apr_uint32_t id) = 0;
};

struct IActor
{
virtual ~IActor(){}
virtual void getIntialInfo(ActorInfo* info) = 0;
virtual void play(ITurn* turnInfo)  = 0;
};

```

### 双C / c++
在双重C/ c++编程模型中，您可以使用
C或c++。当你注册你的对象时，你指定它们是C还是
c++对象。如果您创建了一个平台，并且希望提供
第三方开发人员最终自由选择他们的编程
语言和编程模型，以及混合和匹配C和c++插件。
插件框架支持它，但真正的工作是设计一个dual
C/ c++对象模型。每个对象类型都需要
同时实现C接口和c++接口。这意味着你会的
有一个c++类的标准虚函数表和一堆函数
对应于虚表方法的指针。机制
并非微不足道，我将在示例游戏的背景下进行演示。
请注意，从插件开发人员的角度来看，双重C/ c++
模型不会引入任何额外的复杂性。插件开发人员
总是使用C接口或c++开发C或c++插件
接口。

### 混合C / c++
在C/ c++混合编程模型中，您可以使用c++开发插件
在幕后使用的是C对象模型。这涉及到创建c++
实现c++对象模型和包装的包装器类
对应的C对象。插件开发人员针对这一层进行编程
它转换每个调用，参数和返回值
C和c++。这需要在实现
应用程序对象模型，但通常非常直接。好处是
为具有完整C级的插件开发人员提供了一个很好的c++编程模型
兼容性。我不会在样本游戏中进行演示。

探索了各种解决方案，并介绍了插件框架。在这个
在第一部分中，我描述了一个基于插件的系统的体系结构和设计
基于插件框架，插件的生命周期，以及
通用插件框架。小心!一些代码可能会在这里和
在那里。
基于插件的系统架构

基于插件的系统可以大致分为三个部分
耦合:主系统或应用程序与其对象模型，即插件
管理器和插件本身。插件遵循插件
管理器的接口和协议，还实现对象模型
接口。让我们用一个具体的例子来说明它。主要系统是a
回合制游戏。游戏发生在一个包含各种各样的战场上
怪物。英雄与怪物战斗，直到他自己或所有的怪物死亡。
非常基本，但令人满意。清单1是Hero类的定义

```
#ifndef HERO_H
#define HERO_H
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include "object_model/object_model.h"
class Hero : public IActor
{
    public:
    Hero();
    ~Hero();

    // IActor methods
    virtual void getInitialInfo(ActorInfo* info) = 0;
    virtual void play(ITurn* turnInfo);
};
#endif
```

BattleManager是驱动游戏的引擎。它照顾到
实例化英雄和怪物并填充战场。然后
在每个回合中，它会召唤每个角色(英雄或怪物)去做最坏的事情
play()方法。
英雄和怪物实现了IActor接口。英雄是一个
内置游戏对象与预定义的行为。另一边是怪物
手被实现为插件对象。这让游戏变得有趣
扩展新的怪物和解耦新的开发
主要由怪兽引擎开发的游戏。
PluginManager的工作是抽象掉怪物的事实
从插件中生成，并将它们作为角色呈现给BattleManager
像英雄一样。这种方案还允许游戏自带一些功能
怪物是静态链接的，没有在插件中实现。
理想情况下，BattleManager甚至不应该意识到存在这样的东西
插件。它应该只在c++对象级别上操作。这使它更容易
也可以测试，因为您可以在测试代码中创建模拟怪物
必须编写一个完整的插件。
PluginManager本身可以是通用的也可以是专门化的。一个通用的
插件管理器不知道特定的底层对象模型。当一个
c++ PluginManager实例化一个在插件中实现的新对象
必须返回一个泛型接口，调用者必须将其强制转换为实际的
接口从对象模型。这有点难看，但是必要的。一个
自定义PluginManager知道你的对象模型，并可以在其中操作
基础对象模型的术语。例如，a
我们游戏的自定义PluginManager可以有一个CreateMonster()函数
返回一个IActor接口。我展示的PluginManager是一个通用的
一个，但我将演示放置特定于对象模型是多么简单
层在上面。这是标准做法，因为你不希望你的
处理显式强制转换的应用程序代码

### 插件系统生命周期
是时候弄清楚插件系统的生命周期了。的
应用程序，PluginManager和插件本身参与一个
复杂的舞蹈要按照严格的规程进行。好消息是
一般的插件框架大多可以编排这个过程。的
当应用程序需要插件时，它可以访问插件
需要实现一些将在适当时候调用的函数。
注册静态插件
静态插件是部署在静态库中并被链接的插件
静态地插入到应用程序中。注册可以自动完成，如果
库定义了一个全局注册器对象，其构造函数被调用
自动。不幸的是，它并不适用于所有平台(例如，
Windows)。另一种方法是显式地告诉PluginManager
通过传递一个专用的init函数来初始化静态插件。因为，所有的
静态插件静态链接到主可执行文件init()函数
(它必须有PF_InitPlugin的签名)的每个插件必须有
唯一的名称。一个好的约定类似于<Plugin
名称> _InitPlugin()。下面是静态对象的init()函数的原型
名为“StaticPlugin”的插件:

```
extern "C" PF_ExitFunc
StaticPlugin_InitPlugin(const PF_PlatfromServices* params)
```

显式初始化在主程序之间创建了紧密耦合
应用程序和静态插件因为主应用程序需要
在编译时“知道”什么插件链接到它，以便初始化
他们。如果所有的过程都是静态的，那么这个过程可以作为构建的一部分自动化
插件遵循一些约定，构建过程可以使用这些约定来找到它们
并生成代码来实时初始化它们中的每一个。
一旦静态插件的init()函数被调用，它将注册它的所有对象
类型与PluginManager。
加载动态插件
动态插件是比较常见的。它们都应该被部署
在专用目录中。应用程序应该调用
PluginManager的loadAll()方法并传递专用目录
路径。PluginManager扫描该目录下的所有文件并加载每个文件
动态库。应用程序也可以调用load()方法，
如果它想要细粒度控制什么，它会加载单个插件
插件完全加载。
插件初始化
一旦动态库被成功加载，PluginManager就会被加载
寻找一个著名的函数入口点，称为PF_initPlugin。如果这样的
找到一个入口点，PluginManager通过调用
此函数并传递PF_PlatformServices结构体。
这个结构体包含PF_PluginAPI_Version，它允许插件

执行一些版本协商，并确定它是否可以正常工作。如果
应用程序的版本不合适，插件可能会决定失败
初始化。PluginManager记录插件不在的事实
正确初始化并继续加载下一个插件。这不是致命的
如果插件加载失败，从PluginManager的角度来看会出现错误
或初始化。应用程序可以通过枚举执行额外的检查
加载插件，并确认没有关键插件丢失。
清单2包含c++插件(和)的PF_initPlugin函数
它的退出函数)。

```
#include "cpp_plugin.h"
#include "plugin_framework/plugin.h"
#include "KillerBunny.h"
#include "StationaryStatan.h"
extern "C" PLUGIN_API apr_int32_t ExitFunc()
{
    return 0;
}

extern "C" PLUGIN_API PF_ExitFunc PF_initPlugin(const PF_PlatformServices* params)
{
    int res = 0;
    PF_RegisterParams rp;
    rp.version.major = 1;
    rp.version.minor = 0;
    rp.programmingLanguage = PF_ProgrammingLanguage_CPP;

    // Register KillerBunny
    rp.createFunc = KillerBunny::create;
    rp.destroyFunc = KillerBunny::destroy;
    res = params->registerObject(const apr_byte_t* )"KillerBunny",&rp);
    if(res < 0) 
      return nullptr;

    // Register StationarySatan
    rp.createFunc = StationarySatan::create;
    rp.destroyFunc = StationarySatan::destroy;
    res = params->registerObject((const apr_byte_t * )"StationarySatan",&rp);
    if(res < 0)
      return nullptr;

    return ExitFunc;
}
```

### 对象注册
球现在是在插件本身的手中(内部)
PF_initPlugin代码)。如果版本协商顺利，插件
应该在插件管理器中注册它支持的所有对象类型。的
注册的目的是为应用程序提供功能
比如PF_CreateFunc和PF_DestroyFunc，它可以稍后使用它们来创建
并销毁插件对象。这种安排允许插件控制
对象的实际创建和销毁，包括它们的任何资源
管理(类似于内存)，但让应用程序控制对象的数量
以及他们的一生。当然，插件可以通过always实现单例
返回相同的对象实例。
注册是通过准备每个对象类型注册记录来完成的
(PF_RegisterParams)和调用registerObject()函数指针
在作为参数传递的PF_PlatformServices结构中提供
PF_initPlugin)。registerOBject()函数接受一个字符串
唯一标识对象类型或通配符“*”和
PF_RegisterParams结构体。我将解释类型字符串的用途
以及如何在下一节中使用它。需要类型字符串的原因
是因为不同的插件可能支持多种类型的对象。


在清单2中可以看到，c++插件注册了两个怪物类型
——“杀人兔”和“文具撒旦”。
现在，情况完全不同了。一旦插件
调用registerObject()控件返回到PluginManager。
PF_RegisterParams还包含一个版本和一个编程
语言领域。version字段让PluginManager确保它可以
使用此对象类型。如果存在版本不匹配，则不会注册
对象。这不是致命的错误。这允许相当灵活的谈判，
在哪里插件试图注册同一对象类型的多个版本
为了利用新接口(如果有的话)并回退到旧接口
接口。程序设计语言领域将很快解释。如果
插件管理器对PF_RegisterParams结构体很满意，它只是存储
它位于将对象类型映射到的内部数据结构中
PF_RegisterParams结构体。
在插件注册了所有的对象类型之后，它返回一个函数指针
PF_ExitFunc。这个函数在插件卸载之前被调用
让插件清理在其生命周期内获得的所有全局资源。
如果插件决定它不能正常工作(不能分配一些
资源，关键对象类型注册失败，版本不匹配)
在自身之后进行清理并返回NULL。这向PluginManager发出信号

插件初始化失败。PluginManager也会删除所有
由失败的插件执行的注册。
由应用程序创建插件对象
在这一点上，所有的动态插件已经加载，静态和
动态插件已经初始化并注册了所有的对象类型
他们的支持。应用程序现在可以通过调用来创建对象实例
PluginManager的createObject()方法。此方法接受
对象类型字符串和IObjectAdapter接口。我将讨论对象
下一节将讨论适配，所以让我们关注对象类型字符串。的
应用程序需要知道支持什么对象类型。这方面的知识
可以硬编码到应用程序或它可以查询插件的管理器
注册映射，并在运行时找出当前的对象类型
注册。
如果您还记得的话，类型字符串可以是唯一类型标识符或
通配符“*”。当应用程序使用类型字符串调用createBbject()时
(“*”是一个无效的类型字符串)PluginManager查找精确匹配
在其注册图中。如果找到匹配项，它将调用
注册的PF_CreateFunc并将结果返回给应用程序(可能
适应后)。如果找不到匹配，它将遍历所有的外卡
注册(以“*”类型字符串注册的插件)并让他们尝试通过
调用他们注册的PF_CreateFunc。如果任何插件返回一个非NULL结果，它将返回给应用程序。

通配符注册的目的是什么?它允许插件创建
他们在注册时不知道的对象。怎么啦？是的。在Numenta,
我们用它来支持Python插件。注册的单个c++插件
"*"类型字符串。如果应用程序请求一个Python类(类型为
Python类的实际限定导入路径)，然后是拥有的c++插件
嵌入式Python解释器创建了一个特殊对象，该对象包含一个
Python类的实例，并将插件请求转发到其内部
Python对象(通过Python C API)。在应用程序中，它显示为
标准c++对象。这允许极大的灵活性，因为它可以
只需在正确的位置放置一个Python类，即使在系统运行时也是如此
Python对象立即可用。
基于c语言对象的自动适应
同样，插件框架同时支持C和c++插件。C和c++
插件对象实现不同的接口。我提出的主要创新
下一期将介绍如何设计和实现C/ c++双重对象
模型。统一的对象模型可以透明地访问和使用

由C和c++对象操作。但是，如果应用程序必须这样做
处理每个插件使用其本地接口，这将是高度
不方便。应用程序代码将充斥着if语句
每个参数都必须转换成合适的数据类型，
这也是非常低效的。插件框架使用两种技术来
克服这些障碍。
- 首先，对象模型由实现C和c++。
- 其次，C对象被一个特殊的适配器包装，该适配器暴露了一个实现相应c++接口的c++ facade。
的最终的结果是，应用程序可以幸灾乐祸地忽略这个事实
根本就没有C语言插件它可以将所有插件对象视为c++
对象，因为它们都将实现c++接口。
实际的适配是使用对象适配器完成的。这是一个物体
的专门化，由应用程序提供
插件框架提供的ObjectAdapter模板)
实现IObjectAdapter接口。
清单3包含IObjectAdapter接口和
ObjectAdapter模板

```
#ifndef OBJECT_ADAPTER_H
#define OBJECT_ADAPTER_H
#include "plugin_framwork/plugin.h"
// this interface is used to adapt C plugin objects to C++ plugin objects.
// it must be passed to the pluginmanger::createObject() function.
struct IObjectAdatper
{
    virtual ~IObjectAdapter(){}
    virtual void* adapt(void* object,PF_DestroyFunc df ) = 0;
};
// this template should be used if the object model implements the dual c/c++ object desin pattern. otherwise you need to provide
// your own object adapter class that implements IObjectAdatper
template<tyepname T,typename U>
struct ObjectAdapter : public IObjectAdapter
{
    virtual void* adapt(void* object, PF_DestroyFunc df){
        reurn new T((U*) object,df);
    }
};
#endif // OBJECT_ADAPTER_H
```

PluginManager使用它来将C对象适配为c++对象。我解释
当我详细介绍各种通用组件的过程时
插件框架。
重要的是，插件框架提供了所有功能
将C对象转换为c++对象所必需的基础结构;
但它需要应用程序的帮助，因为它不知道的类型
它需要适应的对象。
应用程序和插件对象之间的交互
应用程序只是在c++接口上调用c++成员函数
它创建的插件对象(可能改编为C对象)。除了…之外
忠实地从插件对象的成员函数返回结果
也可以通过PF_InvokeService函数调用回调函数
PF_PlatformServices结构的。这些服务可以用于多种用途
诸如日志记录、错误报告、长时间运行的进度通知等目的
操作和事件传播。这些回调函数是
协议之间的应用程序和插件，必须设计为一部分
对整个应用程序的接口和对象模型进行设计

应用程序销毁插件对象
管理对象生命周期的最佳实践是，创建者也是对象的创建者
驱逐舰。这在像c++这样的语言中尤其重要
负责内存分配和回收。有很多方法
分配和释放内存:malloc/free, new/delete, array new/delete，
特定于操作系统的api，从不同的堆中分配/释放，等等。它经常是
使用相应的解分配方法进行解分配非常重要
分配方法。创造者最了解如何去做
分配的资源。在插件框架中，每个对象类型都是
注册了一个创建函数和一个销毁函数
(PF_CreateFunc和PF_DestroyFunc)。创建插件对象
应该使用PF_DestroyFunc销毁。
每个插件都负责正确地实现所有资源
都清理干净了。该插件可以自由地实现任何内存
它想要的计划。所有插件对象都可以静态分配
和PF_DestroyFunc可能什么都不做，或者可能有一个预先创建的实例池，PF_DestroyFunc可能只是返回一个对象到
池。应用程序只使用PF_CreateFunc和创建对象
当使用PF_DestroyFunc完成时释放它们。的
c++插件对象的析构函数做正确的事情，所以应用程序
不需要处理直接调用PF_DestroyFunc的问题，并且可以进行dispose处理
使用标准删除操作符的插件对象。这适用于适应环境
对象，因为对象适配器确保调用
在它的析构函数中适当的pf_destroyfunction。

当应用程序关闭时插件系统清理
当应用程序退出时，它需要销毁它创建的所有插件对象
并通知所有插件(包括静态和动态)是时候清理了。
应用程序通过调用
PluginManager的shutdown()会依次调用
每个插件的PF_ExitFunc(从PF_initPlugin函数返回)
如果成功)并卸载所有动态插件。打电话是很重要的
退出函数，即使应用程序即将退出和所有的内存
保留的插件将自动回收。原因是有
不自动回收的其他类型的资源
因为插件可能有一些缓冲状态，他们需要
通过网络提交/刷新/发送等。应用程序很幸运
PluginManager负责这个。
在某些情况下，应用程序也可能选择只卸载一个
插件。在这种情况下，也必须调用exit函数，即插件本身

卸载(如果它是一个动态插件)，并从所有
PluginManager的内部数据结构。
插件系统组件
本节描述了通用插件的主要组件
框架和它们的作用。你可以在
源代码的plugin_framework子目录。
DynamicLibrary
DynamicLibrary组件是一个简单的跨平台c++类。它
在UNIX(包括Mac OS)上使用dlopen/dlclose/dlsym系统调用。
X)和LoadLibrary/FreeLibrary/GetProcAddress API调用
窗户
清单4是DynamicLibrary的头文件

```
#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H
#include <string>
class DynamicLibrary
{
    public:

    static DynamicLibrary* load(const std::string& path, std::string &errorString);
    ~DynamicLibrary();
    void *getSymbol(const std::string& name);

    private:

    DynamicLibrary();
    DynamicLibrary(void* handle);
    DynamicLibrary(const DynamicLibrary&);

    private:

    void* handle_;
};
#endif
```

的实例表示每个动态库
DynamicLibrary类。加载动态库涉及调用
static load()方法，如果一切正常，则返回一个DynamicLibrary指针
为fine，如果失败则为NULL。errorString输出参数包含
错误消息(如果有)。动态库将存储特定于平台的
句柄用于表示加载的库，因此可用于获取
符号和卸载稍后。
getSymbol()方法用于从加载的库中获取符号
析构函数卸载库(只是删除指针)。
加载动态库有不同的方法。为
简单，DynamicLibrary在每个平台上只选择一个选项。它是
可以扩展它，但由于平台的差异，接口将不会
简单了。

### PluginManager
PluginManager是插件框架中的大牛。每一件事
所有与插件有关的信息都要通过PluginManager。清单5
包含PluginManager的头文件。

```
#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H
#include <vector>
#include <map>
#include <apr-1/apr.h>
#include <boost/shared_ptr.hpp>
#include "plugin_framework/plugin.h"

class DynamicLibrary;
struct IObjectAdapter;
class PluginManager
{
    typedef std::map<std::string,boost::shared_ptr<DynamicLibrary> > DyanamicLibraryMap;
    typedef std::vector<PF_ExitFunc> ExitFuncVec;
    typedef std::vector<PF_RegisterParams> RegistrationVec;
    
    public:
    typedef std::map<std::string,PF_RegisterParams> RegistrationMap;
    static PluginManager& getInstance();
    static apr_int32_t initializePlugin(PF_InitFunc initFunc);
    apr_int32_t loadAll(const std::string & pluginDirectory,PF_InvokeServiceFunc func = nullptr);
    apr_int32_t loadByPath(const std::string &path);
    void* createObject(const std::string & objectType,IObjectAdapter& adapter);
    apr_int32_t shutdown();
    static apr_int32_t registerObject(const apr_int32_t * nodeType,const PF_RegisterParams* params);
    const RegistrationMap & getRegistrationMap();
    
    private:
    ~PluginManger();
    PluginManager();
    PluginManager(const PluginManager& );

    DynamicLibrary* loadLibrary(const std::string& path,std::string& errorString);
    
    private:
    bool inIntializePlugin_;
    PF_PlatformServices platformServices_;
    DynamicLibraryMap dynamicLibraryMap_;
    ExitFuncVec exitFuncVec_;

    RegistrationMap tempExactMatchMap_;   // register exact-match object types
    RegistrationVec tempWildCardVec_;   // wild card ('*') object types
    RegistrationMap exactMatchMap_; // register exact-match object types
    RegistrationVec wildCardVec_; // wild card ('*') object types
};
#endif
```

应用程序启动插件的加载
调用PluginManager::loadAll()，传递包含插件的目录
插件。PluginManager加载所有动态插件并进行初始化
他们。它将每个动态插件库存储在dynamicLibraryMap_中，
exitFuncVec_中的每个退出函数(动态和静态插件)
以及exactMatchMap_。通配符注册
都存储在wildCardVec_中。PluginManager现在已经准备好了
创建插件对象。如果有静态插件，他们也注册(要么
通过申请或自动注册)

在插件初始化期间，PluginManager保留所有注册
合并的临时数据结构
如果初始化成功，则转换为exactMatchMap_和wildCardVec_
如果失败了就丢弃。这种事务行为保证了所有的存储
注册来自于仍然加载的成功初始化的插件
到内存中。
当应用程序需要创建一个新的插件对象(动态的或静态的)时
static)它调用PluginManager::createObject()并传递一个对象
型号和适配器。PluginManager使用
注册PF_CreateFunc，并将其从C转换为c++(如果它是C对象)
(基于PF_ProgrammingLanguage成员的注册
结构体)。
在这一点上，PluginManager退出了图片。应用程序
直接与插件对象交互，并最终销毁它。
PluginManager很幸运地忽略了所有这些交互。它是。
应用程序在插件之前销毁插件对象的责任
被卸载(或者至少在插件被卸载后不调用它的方法)。
PluginManager也负责卸载插件。
PluginManager::shutdown()方法应该在应用程序中调用
在它销毁了它创建的所有插件对象之后。shutdown()调用
退出功能的所有插件(静态和动态)，卸载所有的
动态插件并清除所有内部数据结构。这是可能的
通过调用它的loadAll()方法来“重启”PluginManager。的
应用程序也可以通过调用来“重启”静态插件
每个插件的PluginManager::initializePlugin()。静态插件
旧的自动注册将永远消失。
如果应用程序忘记调用shutdown()， PluginManager将调用它
在析构函数中。

ObjectAdapter
ObjectAdapter的工作是将C插件对象适配为c++插件
对象作为对象创建的一部分。IObjectAdapter接口是
简单。它定义了一个方法(除了强制的virtual
析构函数)——适应。adapt()方法接受一个void指针，它将
是一个C对象和pf_destroyfunction函数指针，它可以销毁C
对象。它应该返回一个包装C对象的c++对象。它是。
应用程序提供适当包装器对象的责任。的
插件框架无法做到这一点，因为这项任务需要知识
应用程序对象模型。然而，插件框架提供了

ObjectAdapter模板，它只执行C .对象的static_cast
对象转换为应用程序提供的包装器对象(参见清单3)。
结果对象可以传递给任何需要c++的上下文
接口。这将是本系列下一篇文章的重点，所以
如果现在听起来有点晦涩，不要担心。这里的重点是
是ObjectAdapter模板提供的实现吗
IObjectAdapter接口，应用程序可以将其专门化到自己的接口
C插件对象及其c++包装器。
清单6包含了ActorFactory，它继承了一个专门化
ObjectAdapter模板。它的功能是作为C对象的适配器
实现C_Actor到ActorAdapter的c++对象
IActor接口。ActorFactory还提供了
静态createActor()函数调用
使用PluginManager的createObject()将自己作为适配器并强制转换
导致指向IActor指针的空指针。这允许应用程序调用
友好的createActor()静态函数，只有一个角色类型，没有混乱
使用适配器和强制转换。

```
#ifndef ACTOR_FACTORY_H
#define ACTOR_FACTORY_H
#include "plugin_framework/PluginManager.h"
#include "plugin_framework/ObjectAdapter.h"
#include "object_model/ActorAdapter.h"
struct ActorFactory : public ObjectAdapter<ActorAdapter, C_Actor>
{
 static ActorFactory & getInstance()
 {
 static ActorFactory instance;
 return instance;
 }
 static IActor * createActor(const std::string & objectType)
 {
 void * actor = PluginManager::getInstance().createObject(objectType, 
getInstance());
 return (IActor *)actor;
 }
};
#endif // ACTOR_FACTORY_H

```

PluginRegistrar
PluginRegistrar允许静态插件自动注册它们的对象
使用PluginManager，而不需要应用程序显式地
初始化它们。它的工作方式(当它工作时)是插件定义一个
PluginRegistrar的全局实例，并将其初始化传递给它
函数(具有与PF_InitFunc匹配的签名)。
PluginRegistrar只是调用
PluginManager::initializePlugin()方法，用于点燃静态插件
初始化就像加载动态插件后的动态一样

```
#ifndef PLUGIN_REGISTRAR_H
#define PLUGIN_REGISTRAR_H
#include "plugin_framework/PluginMnager.h"
struct PluginRegister
{
    PluginRegistrar(PF_InitFunc initFunc)
    {
        PluginManager::initializePlugin(initFunc);
    }
};
#endif // PLUGIN_REGISTRAR_H
```

这是“建立你自己的”系列文章中的第三篇
插件框架”，它是关于在c++中开发跨平台插件的。
第一篇文章详细描述了这个问题，探讨了各种解决方案
并介绍了插件框架。第二篇文章探讨了
架构和设计了一个基于插件的基于插件的系统
框架，插件的生命周期和通用插件的内部
框架。本文涵盖了跨平台开发、杂项
主题如平台服务提供给系统插件，错误
处理，设计和实现了一个双C/ c++对象模型

跨平台开发
用C/ c++进行跨平台开发很困难。真的很难。有数据
类型差异、编译器差异和操作系统API差异。关键在于
跨平台开发就是将平台差异封装起来
主应用程序代码可以专注于应用程序的逻辑。如果你的
应用程序代码受困于特定于平台的代码，并且有很多
#ifdef OS_THIS和#ifdef OS_THAT，这是一个明确的信号，你需要一些
重构。一个好的做法是完全隔离所有特定于平台的内容
将代码放入单独的库或库集中。理想情况是，如果你需要的话
支持一个全新的平台，您只需要修改的代码
平台支持库。
了解你的目标平台
当瞄准多个平台时，首先要做的是
理解并意识到差异。如果您的目标是32位和64位
平台，你需要了解其后果。如果你的目标是Windows，
您需要了解ANSI/MBCS与Unicode/Wide字符串的区别。
如果你的目标是带有精简操作系统的移动设备，你需要知道什么
子集是可用的。
使用一个好的跨平台库
下一步是选择一个好的跨平台库。在那里
有几个很好的图书馆。他们中的大多数都专注于UI。我选择使用
插件框架的Apache Portable Runtime (APR)。APR是
Apache web服务器的基础，subversion服务器和其他一些
项目。
但是APR可能不适合你。它有完整的文档，但是
文档不是一流的。没有一个大的繁荣的社区。有
没有书籍和相对较少的项目使用它。最重要的是，它是
您可能不喜欢它的命名约定。然而，它是
非常可移植和健壮(至少Apache和Subversion使用的部分)
你知道它可以用来实现高性能的系统。

考虑为您的跨平台库编写一个自定义包装器
使用的部件)。这种方法有几个好处:
你可以修改接口以完全符合你的需要
命名约定将与你代码的其余部分匹配
这将使它更容易切换到不同的图书馆，甚至
升级到同一库的新版本。
也有一个缺点:
你必须在写作和维护上投入时间和资源
你的包装
调试可能会有点困难，因为你必须离开
通过另一层(更不用说您的包装器可能是)
车)。

我选择为APR编写包装器，因为它是一个C库，需要这样做
您显式地释放资源，高效地处理内存池
内存分配，我不喜欢命名约定和整个
的感觉。我还使用了一个相对较小的子集(只有目录和文件api)，但是它
这仍然是一项重要的工作，测试和调试它。你可以看到路径
plugin_framework子目录下的Directory类。注意，我用
基本类型的APR类型没有包装在我自己的
类型定义。这只是出于懒惰，不推荐这样做。你
可以在结构体和接口中看到APR类型吗

数据类型差异
c++从C继承的整型是跨平台的危险。int,
Long和friends在不同的平台上有不同的大小(32位和64位)
在今天的系统上是位，以后可能是128位)。对于某些应用程序可能是这样
似乎无关紧要，因为它们从未接近32位限制(或者更确切地说，是31位)
如果您使用无符号整数，则为Bit，但如果您在64位上序列化您的对象
在32位系统上进行系统和反序列化可能会让您不愉快
惊讶。同样，这里也没有轻松的休息。你需要理解这些问题
做正确的事，就是确保发送/保存端和
接收端/加载端对每个值的字节数达成一致。文件
格式或网络协议设计者必须很聪明。
APR为可能不同的基本类型提供了一组类型
不同的平台。这些类型提供了保证的大小，并避免了
模糊内置类型。然而，对于某些应用程序(主要是数字应用程序)，它是
有时使用本机字长很重要(通常是
int代表什么)来实现最大的性能。

在跨平台包装器中包装平台特定的组件
有时您必须编写更大的特定于平台的代码块。为
例如，APR对动态库的支持对于插件来说是不够的
框架的需求。我只是实现了DynamicLibrary类，它
一个跨平台抽象的动态库的概念是否具有中立性
接口。要做到这一点而不失去表现力或连贯性是不可能的
的性能。您需要做出自己的权衡，并暴露于
申请你必须申请的。在DynamicLibrary的情况下，我更喜欢
最小的接口，不允许应用程序指定任何标志
我在Unix上使用了更简单的LoadLibrary()
代替更灵活的LoadLibraryEx()

组织所有第三方依赖项
如今，重用第三方代码是一种常见的做法。有很多好的
拥有自由许可证的图书馆。您的项目也可能使用其中的一些。
在跨平台环境中，选择第三方是很重要的
明智地使用图书馆。除了标准的选择标准，比如
健壮性、性能、易用性、文档和支持，都是您所需要的
注意图书馆的发展和维护方式。一些
库还没有以跨平台的方式开发
开始。经常看到库主要在一个上使用
平台和移植作为一个事后考虑到其他平台。如果代码库是
不统一，是一个危险信号。如果有少量的用户对某一特定
平台是一个危险信号。如果核心开发人员或维护者不这样做，那就不要
将库打包到所有平台，这是一个危险信号。如果是新版本
出来后，有些平台落后了，这是一个危险信号。而红旗则不然
意思是你不应该用图书馆。它只是意味着所有其他的东西
同样，你应该更喜欢一个没有危险信号的图书馆。

投资构建系统
一个好的自动化构建系统对于非琐碎的开发是至关重要的
软件系统。如果你在你的系统中加入了几种风格(标准的，
专业版，企业版)，几个平台(Windows, Linux, Mac OS X)和一个
很少的构建变体(调试，发布)你会得到指数级的爆炸
工件。构建系统必须是完全自动化的，并且支持完整的
构建生命周期——从源代码控制系统获取源代码，做任何事情
预处理，编译，链接，运行单元测试和集成测试，打包
分发，可能运行完整的系统测试，并将结果报告给所有的
利益相关者。
我对构建系统和自动化有点狂热，但你不会
很抱歉投资了你的构建系统。

平台服务
平台服务是由您的系统或系统向插件提供的服务
应用程序。我称它们为“平台服务”，因为它们是通用插件
框架可以作为基于插件的系统的平台。
PF_PlatformServices结构体包含版本、
registerObject函数指针和invokeService函数指针;

```
typedef apr_int32_t (*PF_RegisterFunc)(const apr_int32_t* nodeType,const PF_RegisterParams* params);
typedef apr_int32_t (*PF_InvokeServiceFunc)(const apr_byte_t* serviceName,void* serviceParams);
typedef sstruct PF_PlatformServices
{
    PF_PluginAPI_Version version;
    PF_RegisterFUnc registerObject;
    PF_InvokeServiceFunc invokeService;
}PF_PlatformServices;

```

版本让插件知道它所托管的PluginManager的版本
在。它允许插件做出特定于版本的决定，比如注册不同
对象的类型取决于主机的版本。
registerObject()函数是一个PluginManager服务
插件注册它的对象(没有它就没有插件系统)。
invokeService函数用于特定于应用程序的服务。正常的
应用程序和插件之间的交互(一旦插件对象)
已经创建)是应用程序调用插件对象方法
通过对象模型接口(例如，IActor::play())。但是，它经常是
插件需要从应用程序请求一些服务。它
当应用程序提供一些托管执行时，这种情况经常发生
对象在其中记录进度、报告错误或分配内存的环境
一种集中的方式。应用程序通常提供一个标准的记录器，
错误报告功能，以及内存分配器和所有对象使用它们。
这些服务对象通常是单例或静态函数和方法。
动态插件不能直接访问它们。不像
的registerObject服务中，特定于应用程序的不能在
通用的PF_PlatformServices结构，因为它们是未知的
一般的插件框架和它们会因不同而不同
应用程序。应用程序可以包装所有这些服务访问点和
定义一个包含所有插件的大结构体，并将其传递给每个插件对象
通过对象模型接口方法(例如，initObject()
插件对象必须实现。这在在场的情况下并不总是方便的
C插件。服务对象很可能在c++和c++中实现
接受并返回c++对象作为参数，也许它们是模板化的
也许它们会抛出异常。提供一个C兼容的是可能的
每一个的包装。然而，通常将所有服务集中起来更容易
对象的所有交互都通过单个接收器进行处理
插件。

这就是invokeService()的全部内容。签名很简单——a
服务名称的字符串和指向任意结构体的空指针。这是一个
弱类型接口，但它提供了绝对的灵活性。插件和
应用程序必须协调可用的服务和参数
结构应该是。例2演示了一个接受
文件名、行号、消息和日志。

```
LogServiceParams.h
==================
typedef struct LogServiceParams
{
 const apr_byte_t * filename;
 apr_uint32_t line;
 const apr_byte_t * message;
} LogServiceParams;

Some Application File...
========================
#include "LogServiceParams.h"

apr_int32_t InvokeService(const apr_byte_t * serviceName, 
 void * serviceParams)
{
 if (::strcmp(serviceName, "log") == 0)
 {
LogServiceParams * lsp = (LogServiceParams *)serviceParams;
Logger::log(lsp->filename, lsp->line, lsp->message);
 }
}

```

LogServiceParams结构体是在头文件中定义的
插件和应用#include。这提供了日志记录协议
他们之间。该插件将当前文件名、行号和日志打包
消息，并以“log”作为参数调用invokeService()函数
服务名称和指向该结构的指针(以void *的形式)。的
在应用程序端调用invokeService()函数的实现
指向该结构体的指针作为void指针将其强制转换为LogServiceParams
结构，然后用该信息调用Logger::log()方法。如果
插件没有发送一个正确的LogServiceParams结构，行为是
未定义(但绝对不好)。invokeService()可以用来处理
多个服务请求，并且可以通过返回-1让插件知道它
失败了。如果应用程序需要向插件返回一个结果，输出
变量可以添加到服务参数结构中。每个服务可能有
它自己的参数结构。
例如，如果应用程序想要控制内存分配，因为它
使用自定义内存分配方案，它可以提供一个“分配”和
“释放”服务。当插件需要分配内存时
如果自己执行malloc或new，它将调用“allocate”服务
AllocateServiceParams结构体将包含请求的大小和参数
为分配的缓冲区输出void *(或char *)。

错误处理
错误处理在基于c++插件的系统中略有不同。你不能
只需在插件中抛出异常，并期望它们由
应用程序。这是我在中讨论的二进制兼容性问题的一部分
本系列的第一篇文章。如果插件是使用
与应用程序完全相同的编译器，但这不是我的限制
愿意强迫插件开发者。你可以随时降身成c风格
错误返回代码，但这违背了c++插件的原则
框架。插件框架的主要设计目标之一是
允许插件开发人员和应用程序开发人员进行编程
即使在c++的掩护下，它们也可以用C风格的函数进行通信
动态库边界。
所以，我们需要的是一种拦截在插件中抛出的异常的方法，
以安全和编译器不可知的方式将它们跨动态库边界传输到应用程序，然后再次抛出异常
应用程序端。
我使用的解决方案是包装(在插件方面)每个方法在一个尝试除了块。当在插件端抛出异常时，我提取一些
信息，并将其报告给应用程序
特殊调用invokeService()。在应用程序端，当
在存储错误信息时调用reportterror服务
当前插件对象方法返回，抛出存储异常。
这种延迟序列化的异常抛出机制不是很有效
，但它实现了常规c++异常的语义
插件方法在调用后不做任何其他事情
reportterror服务。

实现双C/ c++对象模型
这部分可能是最复杂和最具创新性的部分
c++插件框架。双对象模型允许C
和c++插件共存，并由同一应用程序托管
允许应用程序本身完全不知道二元性并处理所有
对象作为c++对象。不幸的是，通用插件框架不能
自动为你做。我将介绍设计模式并深入研究
双对象模型的样本游戏，你一定要为你的
应用程序对象模型。
双对象模型的基本思想是每个对象都应该是可用的
通过C接口和c++接口的应用程序。这些
除了语言差异之外，接口应该几乎相同。的
游戏的对象模型包括:
- ActorInfo
- ActorInfoContainer
- Turn
- Actor

ActorInfo是最简单的，因为它只是一个被动结构体，包含
演员信息;参见例3。使用了完全相同的结构
在C和c++中，它是在c_object_model.h文件中完全定义的。另一个
对象不是那么简单。

```
typedef struct C_ActorInfo_
{
 apr_uint32_t id;
 apr_byte_t name[MAX_STR];
 apr_uint32_t location_x;
 apr_uint32_t location_y;
 apr_uint32_t health;
 apr_uint32_t attack;
 apr_uint32_t defense;
 apr_uint32_t damage;
 apr_uint32_t movement;
} C_ActorInfo;

```

ActorInfoContainer是一个成熟的C/ c++双对象;请参见清单
一个。

```
#ifndef ACTOR_INFO_CONTAINER_H
#define ACTOR_INFO_CONTAINER_H
#include "object_model.h"
#include <vector>
struct ActorInfoContainer : IActorInfoIterator,C_ActorInfoIterator
{
    static void reset_(C_ActorInfoIteratorHandle handle)
    {
        ActorInfoContainer *aic = reinterpret_cast<ActorInfoContainer*>(handle);
        aic->reset();
    }
    static C_ActorInfo* next_(C_ActorInfoIteratorHandle handle)
    {
        ActorInfoContainer* aic = reinterpret_cast<ActorInfoContainer*>(handle);
        return aic->next();
    }
    ActorInfoContainer() : index(0)
    {
        C_ActorInfoIterator::handle = (C_ActorInfoIteratorHandle)this;
        C_ActorInfoIterator::reset = reset_;
        C_ActorInfoIterator::next = next_;
    }
    void reset()
    {
        index = 0;
    }
    ActorInfo* next()
    {
        if(index >= vec.size())
            return nullptr;
        return vec[index++];
    }
    apr_int32_t index;
    std::vector<ActorInfo*> vec;
};
#endif
```

现在我将逐行剖析它，请注意。它的工作很好
简单。的集合(std::vector)上提供仅向前迭代
不可变的ActorInfo对象。它还允许重置它的内部指针
集合的开始，因此可以多次传递。的
接口有一个next()方法，该方法返回指向当前对象的指针
(ActorInfo)并将内部指针推进到下一个对象，如果为NULL
它已经返回了最后一个对象。第一个调用将返回第一个
如果集合为空，则为NULL。语义不同于
STL迭代器，迭代器只是前进，你需要显式地
解引用它以获得底层对象。STL迭代器也可以指向
值对象，集合结束的指示符是如果迭代器等于
到集合的end()迭代器。我使用a有几个原因
与STL接口不同的迭代接口。支持STL迭代器
更多不同的迭代风格，而不仅仅是向前
对不可变集合进行迭代。因此，他们更
复杂的使用和需要更多的代码。然而，主要原因是
插件对象的迭代接口也应该支持C接口。
最后，我喜欢这样一个事实，即NULL结果表示收集结束，而I
不必对迭代器解引用来访问对象。我可以这样写
非常紧凑的代码来迭代集合。
回到ActorInfoContainer，它是子类
IActorInfoIterator和C_ActorInfoIterator这就是它的组成
一个双重C/ c++对象;参见例4。

```
struct ActorInfoContainer : IActorInfoIterator, C_ActorInfoIterator
{
};
```

当然，它需要实现这两个接口。c++接口(参见
例5)是典型的ABC(抽象基类)，其中所有的成员
函数(next()和reset())是纯虚函数。

```
strcut IActorInfoIterator
{
    virtual void reset() = 0;
    virtual ActorInfo* next() = 0;
};
```

C接口(参见例6)有一个不透明句柄，它只是一个
指向一个包含一个字符的虚拟结构体的指针，它有两个字符
作为第一个参数接受的next()和release()的函数指针
句柄。

```
typedef struct C_ActorInfoIteratorHandle_
{
    char c;
}* C_ActorInfoIteratorHandle;
typedef struct C_ActorInfoIterator_
{
    void (*reset)(C_ActorInfoIteratorHandle handle);
    C_ActorInfo* (*next)(C_ActorInfoIteratorHandle handle);
    C_ActorInfoIteratorHandle handle;
}C_ActorInfoIterator;
```

ActorInfoContainer管理一个ActorInfo对象和它的向量
通过保留索引来实现c++ IActorInfoIterator接口
它的ActorInfo对象的向量;参见例7。当next()被调用时
返回数组中当前索引中的对象，如果索引为，则返回NULL
大于向量大小。当调用reset()时，它只是将索引设置为
0. 当然，索引被初始化为0

```
struct ActorInfoContainer : 
 IActorInfoIterator,
 C_ActorInfoIterator
{
 ... 
 ActorInfoContainer() : index(0)
 {
 ...
 }
 void reset()
 {
 index = 0;
 }
 ActorInfo * next()
 {
 if (index %gt;= vec.size())
 return NULL;
 return vec[index++];
 }
 apr_uint32_t index;
 std::vector<ActorInfo *> vec; 
};
```

它通过填充C_ActorInfoIterator结构体来实现C接口。在
它将reset_()和next_()静态方法赋值给的构造函数
reset和next函数指向
C_ActorInfoIterator基结构体。它还将this指针赋值给
处理;参见例8。
```
 static void reset_(C_ActorInfoIteratorHandle handle)
 {
 ActorInfoContainer * aic = reinterpret_cast<ActorInfoContainer 
*>(handle);
 aic->reset();
 }
 static C_ActorInfo * next_(C_ActorInfoIteratorHandle handle)
 {
 ActorInfoContainer * aic = reinterpret_cast<ActorInfoContainer 
*>(handle);
 return aic->next();
 }
 ActorInfoContainer() : index(0)
 {
 C_ActorInfoIterator::handle = (C_ActorInfoIteratorHandle)this;
 C_ActorInfoIterator::reset = reset_;
 C_ActorInfoIterator::next = next_;
 }

```

现在是揭开这双物体内部工作原理的时候了。实际的
每个二元对象的实现总是在每个二元对象的c++部分
对象。C函数指针总是指向c++的静态方法
对象，该对象将工作委托给c++的相应方法
接口。这是棘手的部分。尽管C和c++的接口是
在c++中没有可移植的方法来获取ActorInfoContainer的“父类”
从一个基类到另一个基类。要做到这一点，静态C函数
需要访问ActorInfoContainer实例(“子”)。这是
把手派上用场的地方(双关语)。每个静态C方法都进行强制转换
ActorInfoContainer指针的句柄(使用reinterpret_cast)和
调用相应的c++方法。reset()方法接受no
参数，不返回任何值。next()方法不接受参数和
返回ActorInfo指针，这是C和c++相同的返回类型
接口。
当涉及到Turn的双重时，情况就有点复杂了
对象。这个对象实现了itturnc++接口(参见例9)。
和C_Turn C接口(参见例10)。

```
struct ITurn
{
    virtual ActorInfo * getSelfInfo() = 0;
    virtual IActorInfoIterator* getFriends() = 0;
    virtual IActorInfoIterator* getFoes() = 0;
    virtual void move(apr_uint32_t x,apr_int32_t y) = 0;
    virtual void attack(apr_uint32_t id) = 0;
};

typedef struct C_TurnHandle_{
    char c;
}* C_TurnHandle;
typedef struct C_Turn_
{
    C_ActorInfo* (*getSelfInfo) ( C_TurnHandle handle);
    C_ActorInfoIterator* (*getFriends)(C_TurnHandle handle);
    C_ActorInfoIterator* (*getFoes)(C_TurnHandle handle);
    void (*move) (C_TurnHandle handle, apr_uint32_t x,apr_uint32_t y);
    void (*attack)(C_TurnHandle handle,apr_uint32_t id);
    C_TurnHandle handle;
}C_Turn;
```

Turn对象跟随ActorInfoContainer的脚步，并且具有
的函数指针连接的静态C方法
在构造函数中调用C_Turn接口，并将工作委托给c++
方法。让我们关注getFriends()方法。这种方法被认为
从c++的itreturn接口返回IActorInfoIterator
和C_ActorInfoIterator从C_Turn接口。不同的回报
值类型。真是个难题!静态getFriends_()不能只返回
调用getFriends()的结果，它是IActorInfoIterator指针
不能只做reinterpret_cast或C强制转换到C_ActorInfoIterator，因为
C_Turn基结构的偏移量是不同的。解决办法是用一点
内幕信息。itturn::getFriends()的结果是

实际上是IActorInfoIterator，但实际上它返回ActorInfoContainer double
对象，它实现
IActorInfoIterator和C_ActorInfoIterator。
为了得到
从IActorInfoIterator到C_ActorInfoIterator，执行getFriends_()
向上转换为ActorInfoContainer双C/ c++对象
(使用static_cast < ActorInfoContainer >)。曾经有过
作为ActorInfoContainer实例，它可以作为C_ActorInfoIterator。这是
好了，现在喝口水或者喝点烈性酒。这是你应得的。
其核心思想是，整个对象模型都是按照双重标准来实现的
C/ c++对象，可以通过C或c++接口使用
一些推搡和选角)。使用基本类型和C结构也是可以的
比如ActorInfo，可以在C和c++中使用。注意所有这些
令人难以置信的东西被安全地埋藏在应用程序对象模型中
实现。其余的应用程序代码和插件代码则不需要
必须处理多语言多继承，讨厌的强制转换和
通过派生类从一个接口切换到另一个接口。这
诚然，设计模式/习惯用法令人费解，但一旦你掌握了
在上面你可以看到它只是在整个对象模型上重复。
我们还没有做完。我说谎了。就在两句话之前，我说过这个奇怪的对偶
到处都是C/ c++模式的重复。嗯,差不多。当涉及到
IActor(见例11)和C_Actor(见例12)接口了这个
事实并非如此。这些接口代表了实际的插件对象
使用PF_CreateFunc创建的。没有双Actor对象
同时实现IActor和C_Actor。

```
struct IActor
{
 virtual ~IActor() {}
 virtual void getInitialInfo(ActorInfo * info) = 0;
 virtual void play(ITurn * turnInfo) = 0;
};

typedef struct C_ActorHandle_ { char c; } * C_ActorHandle; 
typedef struct C_Actor_
{
 void (*getInitialInfo)(C_ActorHandle handle, C_ActorInfo * info);
 void (*play)(C_ActorHandle handle, C_Turn * turn); 
 C_ActorHandle handle;
} C_Actor;
```

实现IActor和C_Actor的对象来自于插件。
它们不是应用程序对象模型的一部分。他们是
应用程序对象模型。类中定义了它们的接口
应用程序的对象模型头文件(object_model.h和
c_object_model.h)。每个插件对象实现
C++ IActor接口或C++ C_Actor接口(它们被注册了)
相应地使用PluginManager)。PluginManager将适应C语言
对象实现C_Actor接口到基于iactor的改编c++
对象和应用程序将保持无知。
在下一期中，我将讨论编写插件。我来检查一下样品
应用程序及其插件。我还将快速浏览源代码(有
很多)。

### 第4部分
这是关于开发跨平台插件的系列文章中的第四篇
在c++中。在前面的文章(第1部分、第2部分和第3部分)中，我研究了
由于二进制文件的原因，用可移植的方式使用c++插件的困难
兼容性问题。然后介绍了插件框架及其设计
以及实现，探讨了插件的生命周期，涵盖了跨平台的开发，并深入研究了用于中的对象模型的设计
基于插件的系统(特别强调双重C/ c++对象)。
在这一部分中，我将演示如何创建C/ c++混合插件
插件通过C接口与应用程序通信
绝对兼容性，但开发人员针对c++接口进行编程。
最后，我将介绍RPG(角色扮演游戏)
一个使用插件框架和主机插件的示例应用程序。我
解释游戏的概念，界面的设计方式和原因;
最后探讨了应用对象模型

面向插件开发者的c++ Facade
正如您回忆的那样，插件框架同时支持C和c++插件。C
插件是非常便携的，但不是那么容易使用。好消息是
插件开发人员是有可能拥有c++编程模型的
具有C兼容性。不过这是要花钱的。从C的转变
到c++再返回并不是免费的。方法调用插件方法时
应用程序，它通过C接口到达一个C函数调用。一个精心设计的
一组c++包装类(由应用程序提供，供插件使用)
(开发人员)将封装C插件，将每个带有非原始数据类型的C参数包装为相应的c++类型，并调用c++方法
由插件对象提供的实现。然后返回值(如果有的话)
必须从c++转换回C，并通过C接口发送到
应用程序。这听起来有点熟悉。
双重C/ c++模型不完全是自动适应C对象吗
是一样的吗?一句话，“不”。这里的情况完全不同。的
所讨论的对象总是来自应用程序对象模型。的
应用程序实例化它们，并且它可以在其C
和c++接口。在插件方面，你有C接口，你有
对双重物体一无所知。这种知识是必要的
从一个接口到另一个接口。此外，即使插件知道类型
对于双重对象来说这是不够的，因为应用和
插件可能是使用不同的编译器，不同的内存构建的
模型、不同的调用约定等等。内存中的物理布局
同样的物体可能非常不同。如果你能保证
应用程序和插件是vtable兼容的，只需直接使用c++
接口

c++对象模型包装器
这有点奇怪，但很必要。你有一个完美的C/ c++
对象，您只能访问它的C接口，然后将它包装在
具有相同接口的c++包装器。包装纸可以是瘦的也可以是胖的，
特别是当涉及到迭代器时。包装器可以保留C迭代器
并在响应next()和reset()调用时调用它，或者它可以复制整个
收集。
对于样本游戏，我选择了第二种方法。稍微多一点
在通话时容量大，但如果你一次又一次地使用相同的数据，那么容量就大了
实际上可以更快，因为您不必包装每个结果
迭代(如果迭代多次)。
清单1展示了演示游戏的对象模型包装器

```
#ifndef OBJECT_MODEL_WRAPPERS_H
#define OBJECT_MODEL_WRAPPERS_H
#include <string>
#include <vector>
#include <map>
#include "object_model.h"
#include "c_object_model.h"
struct ActorInfoIteratorWrapper : public IActorInfoIterator
{
 ActorInfoIteratorWrapper(C_ActorInfoIterator * iter) : index_(0)
 {
 iter->reset(iter->handle);
 
 // Create an internal vector of ActorInfo objects
 const ActorInfo * ai = NULL;
 while ((ai = iter->next(iter->handle)))
 vec_.push_back(*ai);
 }
 // IActorInfoIteraotr methods
 virtual void reset()
 {
 index_ = 0;
 }
 virtual ActorInfo * next()
 {
 if (index_ == vec_.size())
 return NULL;
 
 return &vec_[index_++];
 }
private:
 apr_uint32_t index_;
 std::vector<ActorInfo> vec_;
};
struct TurnWrapper : public ITurn
{
 TurnWrapper(C_Turn * turn) : 
 turn_(turn),
 friends_(turn->getFriends(turn->handle)),
 foes_(turn->getFoes(turn->handle))
 { 
 }
 
 // ITurn methods
 virtual ActorInfo * getSelfInfo()
 {
 return turn_->getSelfInfo(turn_->handle);
 }
 virtual IActorInfoIterator * getFriends()
 {
 return &friends_;
 }
 virtual IActorInfoIterator * getFoes()
 {
 return &foes_;
 }
 virtual void move(apr_uint32_t x, apr_uint32_t y)
 {
 turn_->move(turn_->handle, x, y);
 }
 virtual void attack(apr_uint32_t id)
 {
 turn_->attack(turn_->handle, id);
 }
 private:
 C_Turn * turn_;
 ActorInfoIteratorWrapper friends_;
 ActorInfoIteratorWrapper foes_;
};
#endif // OBJECT_MODEL_WRAPPERS_H
```

注意，我需要包装传递给main的任何对象的C接口
接口C_Actor，以及递归传递给其参数的任何对象。
幸运的是(或通过设计)，不需要太多的对象
包装。ActorInfo结构体对于C和c++接口都是通用的
而且不需要包装。其他对象是C_Turn对象和
C_ActorInfoIterator对象。这些物体被包裹起来
相应地调用ActorInfoIteratorWrapper和TurnWrapper。的
包装器对象的实现通常非常简单，但是如果您有
如果它们很多，就会让人感到厌烦，而且维护起来也很头疼。每一个
包装器派生自c++接口，并接受相应的c++接口
接口指针在其构造函数中。例如，TurnWrapper对象
派生自c++的itturn接口，并接受类中的a C_Turn指针
它的构造函数。包装器对象将它们的C接口指针和
实现它们的方法，它们通常将调用转发到包装
对象通过存储的C接口指针，并在运行时包装结果
必要的。在本例中，ActorInfoIteratorWrapper采用不同的
的方法。在它的构造函数中，它遍历传递的对象
在C_ActorInfoIterator中，并将ActorInfo对象存储在一个内部的
向量。稍后在它的next()和reset()方法中，它只使用它的
密集的向量。当然，如果集合是迭代器的话，这就行不通了
施工后可进行修改。这很好，因为所有
传入的ActorInfo集合是不可变的。但是，它是一些东西
考虑一下，你需要理解你的对象模型以及它是怎样的
应该用来设计智能包装。TurnWrapper是一个
更保守一点，转发调用getSelfInfo()，attack()，
并将move()移到其存储的C_Turn指针上。它采用了不同的方法
getfoe()和getFriends()方法。它拯救了朋友和敌人
在ActorInfoIteratorWrapper数据成员中，它只是从中返回
调用getFriends()和getfoe()。
ActorInfoIteratorWrapper对象实现
IActorInfoIterator接口，所以它们有适当的数据
c++ itturn接口所需的类型

性能受到的影响有多严重?
视情况而定。记住，你可以在对象模型中包装每个C类型，
但你没有。您可以选择直接使用一些C对象。的
如果将深度嵌套的数据结构作为参数传递，则会产生实际开销
然后你决定把每一个都包起来。这就是
我在最近的一个项目中做出的选择。我有一个复杂的数据结构
涉及多个包含某种结构向量的映射。我不是
担心包装开销，因为这个复杂的数据结构
仅用于初始化。
这里的大问题是，如果您希望调用者保持数据的所有权
或者如果你想复制它而不担心内存管理
调用者的策略，以及数据是否可变(这将排除)
存储快照)。这些都是一般的c++设计关注点，而不是
特定于对象模型包装器。

ActorBaseTemplate
ActorBaseTemplate是混合方法的核心。插件
开发人员只需要从它派生并实现c++ IActor接口
插件会自动与插件管理器进行通信
C接口，并提供完整的二进制兼容性。插件开发者
应该永远不会看到C接口，甚至不会意识到它。
这个模板为它的子类提供了许多服务，所以让我们来看看它
缓慢。例1包含模板的声明。

```
template >typename T, typename Interface=C_Actor<
class ActorBaseTemplate : 
 public C_Actor,
 public IActor
{
...
};
```

模板参数有两个:T和Interface。T是
子类，当你从ActorBaseTemplate派生时，你必须提供
基类(模板)的派生类的类型。这是一个实例
CRTP的“奇怪重复的模板模式”。接口是
插件对象将使用该接口与插件通信
经理。它可以是c++的IActor或c++的C_Actor。默认情况下
C_Actor。你可能想知道为什么不总是C_Actor。毕竟如果……
插件对象希望使用c++与插件管理器通信
可以将自己注册为一个c++对象，并直接从IActor派生。这
是好的想法。AutoBaseTemplate也支持IActor的原因是
让您轻松地从C切换到c++接口。这在
当您想跳过整个C包装器代码时，也可以进行调试
希望在受控环境中部署，并且不需要完整的C
兼容性。在这种情况下，翻转一个模板参数，你
更改底层通信通道。

ActorBaseTemplate本身派生自C_Actor和IActor。它甚至
提供了一个简单的IActor实现，如果你想实现的话
只有部分接口。这样可以避免声明空方法
你自己。C_Actor是关键接口，因为这就是接口
当Interface=C_Actor时，用于与插件管理器通信。
例2是构造函数。

```
ActorBaseTemplate() : invokeService_(NULL)
 {
 // Initialize the function pointers of the C_Actor base class
 C_Actor::getInitialInfo = staticGetInitialInfo;
 C_Actor::play = staticPlay;
 C_Actor * handle = this;
 C_Actor::handle = (C_ActorHandle)handle;
 }
```

它不接受任何参数，初始化invokeService_函数指针
然后将其C_Actor接口的成员初始化为
指向静态函数，并将this指针赋值给句柄。这是
非常类似于C/ c++的双对象模型，它确实是一个双对象
除了实际完成实际工作的c++实现在
派生类。
例3是必须的PF_CreateFunc和PF_DestroyFunc
在插件管理器中注册，用于创建和销毁
实例。

```
// PF_CreateFunc from plugin.h
static void * create(PF_ObjectParams * params)
{
T * actor = new T(params);
// Set the error reporting function pointer
actor->invokeService_ = params->platformServices->invokeService;
// return the actor with the correct inerface
return static_cast<Interface *>(actor);
}
// PF_DestroyFunc from plugin.h
static apr_int32_t destroy(void * actor)
{
if (!actor)
 return -1;
delete ActorBaseTemplate<T, Interface>::getSelf(reinterpret_cast<Interface 
*>(actor));
return 0;
}
```

它们被命名为create()和destroy()，但是它们的名字是不相关的，因为
它们是作为函数指针而不是按名称注册和调用的。的
事实上，ActorBaseTemplate定义了它们，省去了很多麻烦
有抱负的插件开发人员。create()函数只是创建一个新的
T(派生类)的实例并初始化赋值
指向invokeService数据成员的invokeService函数指针。
destroy()函数强制转换它到达的void指针
接口模板参数，然后使用getSelf()方法
稍后讨论)，以获得指向T派生类的正确类型的指针。它
随后调用delete永久销毁实例。这真的很好。
插件开发人员使用标准构造函数创建一个简单的c++类
(它接受PF_ObjectParams，但可以忽略它)和析构函数和
ActorBaseTemplate在幕后发挥它的魔力，确保
所有奇怪的静态函数将被正确地路由到派生类。
示例4包含重载了三次的getSelf()静态方法。

```
// Helper method to convert the C_Actor * argument 
// in every method to an ActorBaseTemplate<T, Interface> instance pointer
static ActorBaseTemplate<T, Interface> * getSelf(C_Actor * actor)
{
return static_cast<ActorBaseTemplate<T, Interface> *>(actor);
}
static ActorBaseTemplate<T, Interface> * getSelf(IActor * actor)
{
return static_cast<ActorBaseTemplate<T, Interface> *>(actor);
}
static ActorBaseTemplate<T, Interface> * getSelf(C_ActorHandle handle)
{
return static_cast<ActorBaseTemplate<T, Interface> *>((C_Actor *)handle);
}
```

IActor, C_Actor和C_ActorHandle有三个重载。
getSelf()方法只执行static_cast从接口获取
就像你们之前看到的那样。在句柄的情况下
执行C强制转换使其成为C_Actor。正如您在构造函数和
稍后，ActorBaseTemplate通常会在
它真的需要自己继续前进。
示例5包含静态reportterror方法。

```
// Helper method to report errors from a static function
 static void reportError(C_ActorHandle handle, 
 const apr_byte_t * filename, 
 apr_uint32_t line, 
 const apr_byte_t * message)
 {
 ActorBaseTemplate<T, Interface> * self = ActorBaseTemplate<T, 
Interface>::getSelf(handle);
 ReportErrorParams rep;
 rep.filename = filename;
 rep.line = line;
 rep.message = message;
 self->invokeService_((const apr_byte_t *)"reportError", &rep);
 }

```

这是一个纯粹的方便函数，将调用转发给
invokeService函数指针。它节省了呼叫者打包它的
参数输入到应用程序定义的reporterparams中
h头文件以及使用“reportterror”调用服务
字符串。这些错误报告约定由应用程序定义
服务层和无关紧要的插件开发人员只是想
尽可能快速和简单地制作插件对象。
例6包含C_Actor接口的实现。

```
// C_Actor functions
 static void staticGetInitialInfo(C_ActorHandle handle, C_ActorInfo * info)
 {
 ActorBaseTemplate<T, Interface> * self = ActorBaseTemplate<T, 
Interface>::getSelf(handle);
 try
 {
 self->getInitialInfo(info);
 }
 catch (const StreamingException & e)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)e.filename_.c_str(), e.line_, (const apr_byte_t *)e.what());
 }
 catch (const std::runtime_error & e)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)__FILE__, __LINE__, (const apr_byte_t *)e.what());
 } 
 catch (...)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)__FILE__, __LINE__, (const apr_byte_t *)"ActorBaseTemplate<T, 
Interface>::staticGetInitialInfo() failed");
 }
 }
 static void staticPlay(C_ActorHandle handle, C_Turn * turn)
 {
 try
 {
 TurnWrapper tw(turn);
 getSelf((C_Actor *)handle)->play(&tw);
 }
 catch (const StreamingException & e)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)e.filename_.c_str(), e.line_, (const apr_byte_t *)e.what());
 }
 catch (const std::runtime_error & e)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)__FILE__, __LINE__, (const apr_byte_t *)e.what());
 } 
 catch (...)
 {
 ActorBaseTemplate<T, Interface>::reportError(handle, (const apr_byte_t 
*)__FILE__, __LINE__, (const apr_byte_t *)"ActorBaseTemplate<T, 
Interface>::staticPlay() failed");
 }
 }
```

这两个接口函数的实现几乎是一样的
相同的:getSelf()，在派生类中调用c++ IActor实现
通过多态性的奇迹和采用健壮的错误处理。之前
我讨论了错误处理，注意了staticPlay()函数。它接受
一个C_Turn接口，将它包装在TurnWrapper中，然后传递给
在IActor::play()方法中，它将作为c++的返回值到达。这就是
包装纸是给。
错误处理是ActorBaseTemplate的另一个很好的特性。它允许
插件开发人员再次忘记了他们正在编写的插件对象
必须遵守严格的规则(例如不跨
二进制兼容性边界)，并在错误时抛出异常。每一个
对派生类的调用(构造函数和析构函数除外)为
包裹在这些try-except子句中。这里有一连串的例外
从信息最多的到信息最少的。插件
定义的StreamingException类
插件框架。这是一个很好的独立异常类，它包含
抛出异常的位置(文件名和行号)
错误消息。如果你想了解更多关于StreamingException的信息，
参见混合环境中的实用c++错误处理。
清单2包含一些方便的宏，用于检查和断言
失败时抛出StreamingException。

```
#ifndef PF_BASE
#define PF_BASE
#include "StreamingException.h"
#define THROW throw StreamingException(__FILE__, __LINE__) \
#define CHECK(condition) if (!(condition)) \
 THROW << "CHECK FAILED: '" << #condition <<"'"
#ifdef _DEBUG
 #define ASSERT(condition) if (!(condition)) \
 THROW <<"ASSERT FAILED: '" << #condition << "'"
#else
 #define ASSERT(condition) {}
#endif // DEBUG
//----------------------------------------------------------------------
namespace base
{
 std::string getErrorMessage();
}
#endif // BASE_H
```

这对于调试来说非常好，因为最终结果就是全部
此信息将传播到
应用程序invokeService()通过reportError()方法实现。
如果插件开发者选择抛出标准的std::runtime_error，那么
错误处理代码将从中提取错误消息
what()方法，但没有有意义的文件名和行号
提供。__FILE__和__LINE__宏将报告文件和行
在ActorBaseTemplate中错误处理代码的编号，而不是
错误的实际位置。最后，退一步是捕捉任何异常
使用省略的except处理程序。在这里，甚至没有错误消息
提取和一个通用消息，该消息至少记录了失败的名称
函数提供。
底线是ActorBaseTemplate将插件开发人员从
实现插件对象的所有变化无常，并允许开发人员
专注于在标准c++ (IActor)中实现对象接口
在这种情况下)，而不会被奇怪的需求(如定义
用于创建和销毁的特定静态方法，通过
有趣的函数指针或处理C的任何碎片。

### PluginHelper
PluginHelper是另一个帮助器类，它可以帮你摆脱繁琐的工作
编写插件粘合代码。清单3是代码

```#ifndef PF_PLUGIN_HELPER_H
#define PF_PLUGIN_HELPER_H
#include "plugin.h"
#include "base.h"
class PluginHelper
{ 
 struct RegisterParams : public PF_RegisterParams
 { 
 RegisterParams(PF_PluginAPI_Version v,
 PF_CreateFunc cf,
 PF_DestroyFunc df,
 PF_ProgrammingLanguage pl)
 {
 version=v;
 createFunc=cf;
 destroyFunc=df;
 programmingLanguage=pl;
 }
 };
 public:
 PluginHelper(const PF_PlatformServices * params) : 
 params_(params),
 result_(exitPlugin)
 {
 }
 PF_ExitFunc getResult()
 {
 return result_;
 } 
 template <typename T>
 void registerObject(const apr_byte_t * objectType, 
 PF_ProgrammingLanguage pl=PF_ProgrammingLanguage_C,
 PF_PluginAPI_Version v = {1, 0})
 {
 RegisterParams rp(v, T::create, T::destroy, pl);
 apr_int32_t rc = params_->registerObject(objectType, &rp);
 if (rc < 0)
 {
 result_ = NULL;
 THROW << "Registration of object type " 
 << objectType << "failed. "
 << "Error code=" << rc;
 }
 }
 private:
 static apr_int32_t exitPlugin()
 {
 return 0;
 }
private:
 const PF_PlatformServices * params_;
 PF_ExitFunc result_;
}; 
#endif // PF_PLUGIN_HELPER_H
```

它被设计为使用插件对象类来实现
PF_CreateFunc和PF_DestroyFunc强制函数是静态的
方法。就是这样。无其他要求。因为它
发生ActorBaseTemplate满足这个要求，所以插件对象
派生自ActorBaseTemplate的类是自动兼容的
PluginHelper。PluginHelper被设计为在
必须的PF_initPlugin()入口点。您将在下一节中看到它的实际应用
当我谈到编写插件时。现在，我先过一遍
PluginHelper为插件开发者提供服务。的工作
所支持的所有插件对象类型
如果成功，返回一个指向PF_ExitFunc出口的函数指针
具有特定签名的函数。如果出了什么问题，就应该这样
返回NULL。
PluginHelper构造函数接受一个指向
包含主机系统插件API的PF_PlatfromServices结构体
version和invokeService和registerObject函数指针和存储
他们。它还在其结果成员中存储exitPlugin函数指针
如果插件初始化成功，将返回。
PluginHelper提供了模板化的registerObject方法
大部分的工作。模板参数T是您需要的对象类型
登记。它应该有一个create()和destroy()静态方法
符合PF_CreateFunc和PF_DestroyFunc。它接受一个对象
类型字符串和可选编程语言(默认值)
PF_ProgrammingLanguage_C)。此方法执行版本检查
确保插件版本与主机系统兼容。如果
一切都很好，它准备一个RegisterObjectParams结构体并调用
调用registerObject()函数并检查结果。如果版本检查或
调用registerObject函数指针失败，它将报告

错误(如果条件为false，则由CHECK宏执行)，设置
将result_转换为NULL，并吞下CHECK抛出的异常。的
它不让异常传播的原因是
因为PF_initPlugin (PluginHelper应该使用的地方)是C语言
函数，该函数不应允许异常在二进制文件中传播
兼容性bounday。捕获registerObject中的所有异常保存
插件开发人员做这件事很麻烦(或者更糟的是，忘记做这件事)。这是一个
很好地说明了使用THROW、CHECK和
断言宏。构造错误消息很容易
流操作符。不需要分配缓冲区，连接字符串或使用
printf。生成的reportError调用将包含控件的确切位置
error (__FILE__， __LINE__)，而不必显式指定它。
通常，一个插件会注册多个对象类型。如果有任何物体
type注册失败，结果_将为NULL。对一些人来说可能没问题
注册失败的对象类型。例如，您可以注册多个
不支持具有相同对象类型和其中一个版本的版本
不再被主机系统控制。在这种情况下，只有这个对象类型将失败
登记。插件开发人员可以在每次调用后检查result_的值
查看PluginHelper::registerObject()并判断它是否致命。如果是。
如果是良性故障，它最终可能返回PluginHelper::ExitPlugin。
默认的行为是，每一个失败都是致命的，插件开发人员
应该只是返回PluginHelper::getResult()，将返回值
的result_，它将是PluginHelper::ExitPlugin(如果所有注册)
成功)或NULL(如果任何注册失败)。

RPG游戏
我喜欢RPG(角色扮演游戏)，作为一名程序员，我喜欢
一直想写我自己的。然而，严肃游戏的问题
开发是指，要产生一个好的产品，需要的不仅仅是编程
游戏。我在索尼Playstation工作过一段时间，但我在
多媒体相关的项目，而不是关于游戏。所以，我收起了我的抱负
一个100人年，价值10亿美元的RPG。我做了几个
小型射击游戏和桌面游戏，专注于写作
各种开发者期刊上的文章。
我选择了一款非常简单的RPG游戏作为展示工具
插件框架。这不会有什么意义。这更像是一场游戏
Demo是因为主程序控制英雄而不是用户。的
虽然概念基础是健全的，但它绝对可以扩展。
现在我已经把你的期望降到零了我们可以继续了

概念
这款游戏的概念非常基础。有一个英雄英雄，他是英雄
尽管他勇敢无畏。这位英雄被一个神秘的
强迫到一个充满各种怪物的战斗舞台。英雄必须
战斗和击败所有的怪物赢得胜利。
英雄和所有的怪物都是演员。演员是实体
有一些属性，如在战场上的位置，生命值和速度。
当演员的健康值降至0(或更低)时，它就会死亡。
游戏发生在二维网格(战场)上。这是一款回合制游戏。
每次轮到演员表演。当演员表演时，他可以移动或攻击
(如果它在另一个怪物旁边)。每个演员都有一个朋友和敌人的名单。
这就产生了党派、氏族和部落的概念。在这个游戏中，英雄
没有朋友，所有的怪物都是他的敌人。

设计界面
接口当然应该支持概念框架。演员
由ActorInfo结构体表示，该结构体包含了它们的所有状态。演员
应该实现IActor接口，允许BattleManager
获取他们的初始状态并指导他们玩游戏。itturn接口是
轮到演员表演时的感受。itturn接口允许
actor获取自己的信息(如果它不存储信息)，四处移动
进攻。这个想法是，BattleManager负责数据
参与者接收他们的信息并在一个管理的
环境。当演员移动时，BattleManager应该执行
根据它的移动点移动，确保它不会出界
等。BattleManager也可以忽略非法操作(根据其
比如多次攻击或攻击朋友。这是
就这么简单。参与者通过不透明的id相互关联。这些
id每一回合都会刷新，因为演员可能会死亡，新角色也可能死亡
出现。因为这只是一个样本游戏，我并没有执行太多内容
政策执行。在网络游戏(尤其是MMORPG)中，用户
通过网络协议使用客户端与服务器交互是非常简单的
重要的是验证客户的任何行动，以防止欺骗，欺诈和
逆反心理。有些游戏拥有虚拟和/或真实经济
人们一直在尝试。这些很容易破坏所有游戏的用户体验
合法的用户

实现对象模型
对象模型的实现非常简单
C/ c++的双重特性。实际的实现驻留在c++中
方法。ActorInfo只是一个带有数据的结构体。ActorInfoIterator是
只是一个ActorInfo对象的容器。让我们检查一下Turn对象。这是一个
这是一个很重要的目标，因为这是一款回合制游戏。一个
当轮到演员上场时，为每个演员创建fresh Turn对象。
Turn对象被传递给每个角色的IActor::play()方法。
Turn对象有它的actor信息(如果actor不存储它)
它有两张敌人和朋友的名单。它提供了三个访问器
方法getSelfInfo()、getFriends()和getfoe()以及两个动作
方法:attack()和move()。
例7包含访问器方法的代码，这些方法只返回
对应的数据成员和move()方法
当前actor的位置。

```
ActorInfo * Turn::getSelfInfo()
{
 return self;
}
IActorInfoIterator * Turn::getFriends()
{
 return &friends;
}
IActorInfoIterator * Turn::getFoes()
{
 return &foes;
}
void Turn::move(apr_uint32_t x, apr_uint32_t y)
{
self->location_x += x;
 self->location_y += y;
}
 ```

我不验证任何东西。演员可能会离开舞台或者
移动超过其移动点允许的范围。这在现实中是飞不起来的
游戏。
示例8包含attack()代码及其帮助程序
doSingleFightSequence()函数

```
static void doSingleFightSequence(ActorInfo & attacker, ActorInfo & 
defender)
{
 // Check if attacker hits or misses
 bool hit = (::rand() % attacker.attack - ::rand() % defender.defense) > 0;
 if (!hit) // miss
 {
 std::cout << attacker.name <<" misses " << defender.name <<std::endl;
 return;
 }
 // Deal damage
 apr_uint32_t damage = 1 + ::rand() % attacker.damage;
 defender.health -= std::min(defender.health, damage);
 std::cout << attacker.name << "(" <<attacker.health << ") hits " 
 << defender.name <<"(" <<defender.health <<"), damage: " << damage 
<< std::endl;
}
void Turn::attack(apr_uint32_t id)
{
 ActorInfo * foe = NULL;
 foes.reset();
 while ((foe = foes.next()))
 if (foe->id == id)
 break;
// Attack only foes
 if (!foe)
 return; 
 std::cout << self->name << "(" << self->health << ") attacks " 
 << foe->name << "(" << foe->health << ")" << std::endl;
 while (true)
 {
 // first attacker attacks
 doSingleFightSequence(*self, *foe);
 if (foe->health == 0)
 {
 std::cout << self->name << " defeated " << foe->name << std::endl;
 return;
 }
 // then foe retaliates
 doSingleFightSequence(*foe, *self);
 if (self-&tl;health == 0)
 {
std::cout << self->name << " was defeated by " << foe->name 
<<std::endl;
 return;
 }
 }
}
```

攻击逻辑很简单。当一个参与者攻击另一个参与者时(已识别)
通过id)，攻击者位于敌人列表中。如果不是敌人攻击
结束。actor(通过dosingle打斗序列()函数)击中敌人，然后
造成的伤害从敌人的生命值中减少。如果敌人是
还活着的时候，它会报复并攻击攻击者，以此类推，直到1
战士死亡。
今天就到这里。在本系列的下一篇(也是最后一篇)文章中，我将介绍
BattleManager和游戏的主循环。我将探索深度写作
插件的RPG游戏，并引导您通过的目录结构
各种库和项目的插件框架和示例
游戏都是由。最后，我将比较插件框架I
这里介绍一下NuPIC的插件框架。NuPIC代表Numenta's
智能计算平台。我开发了大部分的概念
我在这里介绍创建NuPIC插件基础架构时的想法。

这是关于的系列文章的最后一篇
用c++开发跨平台插件。在
以前的文章-第1部分，第2部分，第3部分，
第四部分，我考察了工作的困难
以可移植的方式使用c++插件。
在这一部分中，我将介绍
在第4部分介绍的样例游戏，并给出一个
演示。我还快速参观了
本系列文章附带的源代码如下
几个好故事(是时候了)，最后
比较插件框架
Numenta的智能平台(NuPIC
计算)插件框架，这是它的
概念上的祖先。但首先，让我们看一下
在一些怪物插件，将加载到
这个游戏

怪物的插件
我创建了四个不同的插件来演示
插件框架的范围和多样性。
一个纯c++插件
一个纯C插件
混合插件部署为动态/共享
库
一个静态的c++插件，应该被链接
直接到可执行文件。
所有这些插件都把他们的怪物注册到
PluginManager作为演员。此外，游戏
它将Hero作为一个对象来实现
实现了IActor接口和的大部分
而在IActor接口上工作的代码却不能
(并且无法区分)游戏提供的英雄和任何怪物。游戏也可以
提供一些内置的怪物。

动态c++插件
动态c++插件寄存器
杀人兔子和撒旦怪物。
清单1是KillerBunny.h头文件，其中
定义KillerBunny类。KillerBunny是
直接从c++ IActor接口派生
(这使它成为一个纯c++插件)。它实现了
的create()和destroy()静态函数
支持创建和销毁
PluginManager。文具撒旦和任何其他
纯c++插件对象应该看起来完全相同
(私人会员除外，如有的话)。

```
#ifndef KILLER_BUNNY_H
#define KILLER_BUNNY_H
#include <object_model/object_model.h>
struct PF_ObjectParams;
class KillerBunny : public IActor
{
public:
 // static plugin interface
 static void * create(PF_ObjectParams *);
 static apr_int32_t destroy(void *);
 ~KillerBunny();
 // IActor methods
 virtual void getInitialInfo(ActorInfo * info);
 virtual void play(ITurn * turnInfo);
private:
 KillerBunny();
};
#endif
```
示例1包含该实现
create()和destroy()的函数。它们几乎是微不足道的。
create()函数只是实例化一个
new KillerBunny对象并返回它
不透明的void指针)。destroy()函数
接受一个void指针，它实际上是一个指针
到前面创建的实例
create()函数。将void指针强制转换为
一个KillerBunny指针并删除它。重要的
部分原因是这些函数让
PluginManager创建KillerBunny对象
“知道”任何关于“兔子杀手”课程的事。的
返回的实例可通过
稍后再使用IActor接口(尽管它是)
作为空指针返回)。

```
void * KillerBunny::create(PF_ObjectParams *)
{
 return new KillerBunny();
}
apr_int32_t KillerBunny::destroy(void * p)
{
 if (!p)
 return -1;
 delete (KillerBunny *)p;
 return 0;
}
```

的实现
IActor接口方法。这些方法是
微不足道的。getInitialInfo()方法
用一些数据填充ActorInfo结构体。
play()方法是真正的KillerBunny使用的方法
做实际工作，跑来跑去，躲避还是攻击
敌人，并一般证明其名称。在这里,它
只是从
itturn接口来验证它是否工作。这是
只是我的懒惰，事实上所有的
怪物什么也不做。英雄是唯一
一场真正的战斗。怪物们会保卫
当受到攻击时，它们自己甚至会报复。
```
void KillerBunny::getInitialInfo(ActorInfo * info)
{
 ::strcpy((char *)info->name, "KillerBunny");
 info->attack = 10;
 info->damage = 3;
 info->defense = 8;
 info->health = 20;
 info->movement = 2;
 // Irrelevant. Will be assigned by system later
 info->id = 0;
 info->location_x = 0;
 info->location_y = 0;
}
void KillerBunny::play(ITurn * turnInfo)
{
 IActorInfoIterator * friends = turnInfo->getFriends();
}
```
这里的要点是编写纯c++插件
对象很简单。除了
样板文件create()和destroy()静态方法，
你只需要实现一个标准的c++类。没有晦涩难懂的
需要咒语。
清单2包含插件初始化
代码。还不错，但是很无聊，而且会出错
中定义一个退出函数
定义PF_initPlugin函数
一个PF_RegisterParams结构体，填充它，然后
注册每个插件对象。一定要回来
如果初始化失败，则为NULL。不到
振奋人心。这就是写一个纯
c++怪物插件(有两个怪物)。

```
#include "cpp_plugin.h"
#include "plugin_framework/plugin.h"
#include "KillerBunny.h"
#include "StationarySatan.h"
extern "C" PLUGIN_API apr_int32_t ExitFunc()
{
 return 0;
}
extern "C" PLUGIN_API PF_ExitFunc PF_initPlugin(const
PF_PlatformServices * params)
{
 int res = 0;
 
 PF_RegisterParams rp;
 rp.version.major = 1;
 rp.version.minor = 0;
 rp.programmingLanguage = PF_ProgrammingLanguage_CPP;
 
 // Register KillerBunny
 rp.createFunc = KillerBunny::create;
 rp.destroyFunc = KillerBunny::destroy;
 res = params->registerObject((const apr_byte_t
*)"KillerBunny", &rp);
 if (res < 0)
 return NULL;
 // Regiater StationarySatan
 rp.createFunc = StationarySatan::create;
 rp.destroyFunc = StationarySatan::destroy;
 res = params->registerObject((const apr_byte_t
*)"StationarySatan", &rp);
 if (res < 0)
 return NULL;
 return ExitFunc;
}
```

静态c++插件
的初始化代码
静态插件。它也是一个c++插件，但它是
初始化不同。所有动态插件(两者
C和c++)必须实现众所周知的入口
点函数PF_initPlugin。这就是
PluginManager正在寻找初始化。静态
另一方面，插件是静态链接的
到应用程序。这意味着如果两个插件
实现相同的函数名，有名称冲突

发生，并且应用程序链接失败。所以静态
插件必须有一个唯一的初始化函数
而主应用程序必须知道它
在程序开始时初始化它。一次
初始化函数已经被调用，静态的
插件与其他插件无法区分
插件。最后一行定义了一个插件寄存器
实例。这种不可移植的技术适用于非Windows平台，并允许静态插件
注册自己。这很好，节省了
应用程序代码与静态紧密耦合
插件(链接除外)。这就很容易
只需更改构建即可添加新的静态插件
系统，而无需重新编译现有代码。
静态插件对象看起来就像动态插件
对象并实现IActor接口。例子
的play()方法
静态插件的FidgetyPhantom类
注册到PluginManager。
烦躁的幻影实际上在做一些事情
它的play()方法。这是一个插件的好例子
对象创建和管理的对象
应用程序通过对象模型
接口。烦躁的幻影得到第一个敌人
(通常是英雄)，向他移动并攻击，
如果可能的话。它使用findclose()实用程序
函数查找最近的点(受限制)
它的运动指向它的敌人并朝它移动
这一点。如果它碰到敌人，它就会攻击。

```
void FidgetyPhantom::play( ITurn * turnInfo)
{
 // Get self
 const ActorInfo * self = turnInfo->getSelfInfo();
 // Get first foe
 IActorInfoIterator * foes = turnInfo->getFoes();
 ActorInfo * foe = foes->next();
 
 // Move towards and attack the first foe (usually the 
hero)
 Position p1(self->location_x, self->location_y);
 Position p2(foe->location_x, foe->location_y);
 Position closest = findClosest(p1, p2, self->movement);
 turnInfo->move(closest.first, closest.second);
 if (closest == p2)
 turnInfo->attack(foe->id);
}
```

动态C插件
C插件注册实现的对象
C_Actor接口。插件本身——甚至
C_Actor的实现——可以是c++类
(使用静态方法)。在本例中，我实现了
所有的东西都在C语言中，只是为了确保系统支持
纯C插件(有一些编译
在它工作之前的一些问题)。C插件注册了一个
一个叫MellowMonster的怪物。头
这个古怪怪物的文件在示例中给出
4. 这是一个C对象，所以没有类
只定义全局
函数MellowMonster_create()和MellowMonster_
Destroy()，它们对应
到PF_CreateFunc和PF_DestroyFunc。名字是
符合怪物的类型，因为
一般来说，一个插件可以注册怪物类型
用不同的配对
create()/destroy()函数，而在C中我们不能
将它们隐藏在名称空间中或作为类的静态方法
类。

```
#ifndef MELLOW_MONSTER_H
#define MELLOW_MONSTER_H
#include <plugin_framework/plugin.h>
// static plugin interface
void * MellowMonster_create(PF_ObjectParams *);
apr_int32_t MellowMonster_destroy(void *);
#endif
```

例5展示了实际的怪物。这只是
包含C_Actor成员和的结构体
可选的更多特定于怪物的数据。没有多少
到目前为止是一个怪物

```
typedef struct MellowMonster_
{
 C_Actor actor;
 /* additional monster-specific data */
 apr_uint32_t dummy;
} MellowMonster;
```

的实现
C_Actor接口，由两个静态的
函数(在编译之外不可见)
单元)
—MellowMonster_getInitialInfo()和MellowMonster_ . info (
play() -对应于IActor方法。的
最大的区别是c++方法得到
对象实例作为隐式的' this '指针。在
C，你必须显式地传递一个C_ActorHandle(好吧，
不是你，而是PluginManager)和C函数
费力地把把手扳上
一个MellowMonster指针。当C_Turn对象是
在play()函数中使用时，必须将其传递给它
还有自己的手柄。

```
void MellowMonster_getInitialInfo(C_ActorHandle handle,
C_ActorInfo * info)
{
 MellowMonster * mm = (MellowMonster *)handle;
 strcpy((char *)info->name, "MellowMonster");
 info->attack = 10;
 info->damage = 3;
 info->defense = 8;
 info->health = 20;
 info->movement = 2;
 /* Irrelevant. Will be assigned by system later */
 info->id = 0;
 info->location_x = 0;
 info->location_y = 0;
}
void MellowMonster_play(C_ActorHandle handle, C_Turn *
turn)
{
 MellowMonster * mm = (MellowMonster *)handle;
 C_ActorInfoIterator * friends =
turn->getFriends(turn->handle);
}
```
例7包含
创建()和销毁()
Y()函数并将松散的部分捆绑起来。
函数的作用是分配
一个MellowMonster结构体(当然是使用malloc)，
类的句柄成员的指针
演员字段(不检查是否内存
分配失败，boo:-)，然后进行分配
getinitialinfo()和getinitialinfo (
R_play()函数转换为适当的函数指针。
最后，它返回一个MellowMonster指针
不透明的void指针。重要的是
的第一个成员
使用MellowMonster结构，因为PluginManager
(通过适配器)强制转换返回的void指针
指向C_Actor指针，并将其视为C_Actor指针
然后在。
MellowMonster_destroy()释放内存。如果
是否需要类似析构函数的清理
我也能做到。
让我们检查一下C插件的初始化代码
如清单4所示。它看起来就像c++插件。
这并不奇怪，因为它是一个C函数
需要准备一个C结构体并调用
另一个C函数。唯一真正的区别是
即注册编程语言
为MellowMonster是PF_ProgrammingLanguage_C。那
告诉PluginManager它正在处理一个C
对象，它应该适应它。

```
#ifdef WIN32
#include "stdafx.h"
#endif
#include "c_plugin.h"
#include "c_plugin.h"
#include "plugin_framework/plugin.h"
#include "MellowMonster.h"
PLUGIN_API apr_int32_t ExitFunc()
{
 return 0;
}
PLUGIN_API PF_ExitFunc PF_initPlugin(const
PF_PlatformServices * params)
{
 int res = 0;
 
 PF_RegisterParams rp;
 rp.version.major = 1;
 rp.version.minor = 0;
 
 // Regiater MellowMonster
 rp.createFunc = MellowMonster_create;
 rp.destroyFunc = MellowMonster_destroy;
 rp.programmingLanguage = PF_ProgrammingLanguage_C;
 
 res = params->registerObject((const apr_byte_t
*)"MellowMonster", &rp);
 if (res < 0)
 return NULL;
 return ExitFunc;
}
```

正如您所看到的，在C API级别工作是非常困难的
棘手。你需要传递显式句柄，
多投值，注意函数名，以及
将免费函数连接到
monster C_Actor接口。这并不好玩，但是
如果你必须和C一起工作的话。
混合C/ c++插件
所以C很麻烦，你想用
c++ API，但必须提供全部二进制文件
兼容性。真是个难题。幸运的是,
混合的C/ c++插件是正确的。它提供了
C与c++ API的兼容性
通过ActorBaseTemplate。它也减少了
重要的是对插件初始化代码由
使用PluginHelper。这个插件注册了两个
怪物:GnarlyGolem和PsychicPiranea。这两个

派生自ActorBaseTemplate，两者都实现
c++ IActor接口，但GnarlyGolem是
实际上是用C语言与PluginManager对话
而PsychicPiranea正在谈论c++。
例8包含这两个类的定义。
你可以看到它们看起来是多么紧凑和干净。在那里
create()或destroy()是静态的吗
方法不再由ActorBaseTemplate处理
它)。两者之间唯一的区别是
那个PsychicPiranea指定IActor作为
ActorBaseTemplate的第二个模板参数。
这是臭名昭著的Interface参数，它
默认为C_Actor。

```
class GnarlyGolem : public ActorBaseTemplate<GnarlyGolem>
{
public:
 GnarlyGolem(PF_ObjectParams *);
 // IActor methods
 virtual void getInitialInfo(ActorInfo * info);
 virtual void play(ITurn * turnInfo);
};
class PsychicPiranea : public
ActorBaseTemplate<PsychicPiranea, IActor>
{
public:
 PsychicPiranea(PF_ObjectParams *);
 // IActor methods
 virtual void getInitialInfo(ActorInfo * info);
 virtual void play(ITurn * turnInfo);
};
```

灵皮拉尼亚可能已经衍生出来了
直接从IActor
比如c++中的KillerBunny和StationarySatan
插件。它产生的原因
from ActorBaseTemplate是三重的:
它使你免于
编写create()/destroy()静态方法
它可以让你在C和c++之间快速切换
相同的插件部署在不同的
的情况下
所以我可以展示一下这种很酷的能力
这真的很酷，因为在自动之间
通过PluginManager和

漂亮的c++包装器
ActorBaseTemplate提供的应用程序
开发人员和插件开发人员可以很幸福
不知道在它们之间流动的C。唯一的
应该关注C/ c++的开发人员
二元论是对象模型的开发人员。如果你的
系统是一个严肃的平台，然后是对象模型
迟早会凝固的。然后每个人都可以
忘记C，只扩展应用程序
这是建立在对象模型之上的
很多插件——都是用c++编写的。
实现
GnarlyGolem和PsychicPiranea是典型的c++
插件实现。GnarlyGolem在C下面
封面，但它不在乎。
的初始化代码
混合插件。还有更多的#include语句
比代码。我不骗你。数一数。这是
接近于特定于领域的语言
反正没有宏。)您定义
一个PluginHelper对象并调用registerObject(
每个对象。不需要恼人的结构体
函数指针，不需要错误检查。
纯粹的简单性。最后，返回结果。

```
#include "wrapper_plugin.h"
#include "plugin_framework/plugin.h"
#include "plugin_framework/PluginHelper.h"
#include "GnarlyGolem.h"
#include "PsychicPiranea.h"
extern "C" PLUGIN_API
PF_ExitFunc PF_initPlugin(const PF_PlatformServices *
params)
{
 PluginHelper p(params);
 p.registerObject<GnarlyGolem>((const apr_byte_t
*)"GnarlyGolem");
 p.registerObject<PsychicPiranea>((const apr_byte_t
*)"PsychicPiranea", PF_ProgrammingLanguage_CPP);
 return p.getResult();
}
```

有一件事要挑。在注册
灵媒，你得详细说明
PF_ProgrammingLanguage_CPP(默认为
PF_ProgrammingLanguage_C)。编程语言
可以自动从
因为它通过了
作为编程语言的接口参数来
ActorBaseTemplate。然而，这需要一些
模板元编程技巧(类型检测)
我很快就看不清了。这是一个
如果你好奇的话，快速链接:
www.ddj.com/cpp/184402050。

让我们玩!
现在是玩耍时间。现在我将带您快速参观一下
游戏本身。你会看到如何
PluginManager初始化，怪物是怎样的
创建和如何进行战斗。让我们开始
与主要()。
清单6包含main()函数。你可以
跳过所有的#include语句，专注于
DummyInvokeService()函数。这个函数
中的invokeService
PF_PlatformServices结构体用于插件
接收。在这种情况下它没有任何作用，但是
在实际应用中，它扮演着重要的角色
为插件提供系统服务。

```
#ifdef WIN32
#include "stdafx.h"
#endif
#include "plugin_framework/PluginManager.h"
#include "plugin_framework/Path.h"
#include "BattleManager.h"
#include "static_plugin/static_plugin.h"
#include <string>
#include <iostream>
using std::cout;
using std::endl;
apr_int32_t DummyInvokeService(const apr_byte_t *
serviceName, void * serviceParams)
{
 return 0;
}
#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main (int argc, char * argv[])
#endif
{
 cout << "Welcome to the great game!" << endl;
 if (argc != 2)
 {
 cout << "Usage: great_game <plugins dir>" << endl;
 return -1;
 }
 // Initialization
 ::apr_initialize();
 PluginManager & pm = PluginManager::getInstance();
 pm.getPlatformServices().invokeService =
DummyInvokeService;
 pm.loadAll(Path::makeAbsolute(argv[1]));
 
PluginManager::initializePlugin(StaticPlugin_InitPlugin);
 // Activate the battle manager
 BattleManager::getInstance().go();
 ::apr_terminate();
 return 0;
}
```
main()函数的定义符合这两者
Windows和UNIX系统。这个游戏很有趣。
便携。我在Windows XP SP2、Vista、Mac上进行了测试
OS X 10.4 (Tiger)、Mac OS X 10.5 (Leopard)和
Kubuntu 7.10 (Gutsy Gibbon)。这些是最
常见的现代操作系统。它可能会起作用
的构建过程进行调整
其他操作系统范围。
在main()中有一个用户通过的检查
插件目录作为命令行参数。
初始化APR库

PluginManager登场。它是一个单元素
并在应用程序终止时销毁。
下一步是将DummyInvokeService分配给
平台服务结构。一旦invokeService
准备好了，可以初始化插件了。首先,所有
的动态插件加载
目录并传入为**argv[1]**，然后传入静态
插件被显式初始化。这是
不舒服，但我找不到便携的解决方案
可以在Windows上运行。一旦所有的插件
初始化后，BattleManager接受命令。最后,
APR库被清理。
非常简单:检查命令行
参数，初始化全局资源，加载
插件，将控制转移到应用程序逻辑，
清理全球资源。
BattleManager是游戏的大脑。例9
包含整个go()方法。它是这样开始的
提取所有已注册的怪物类型
插件管理。

```
void BattleManager::go()
{
 // Get all monster types
 PluginManager & pm = PluginManager::getInstance();
 const PluginManager::RegistrationMap & rm =
pm.getRegistrationMap();
 for (PluginManager::RegistrationMap::const_iterator i =
rm.begin(); i != rm.end(); ++i)
 {
 monsterTypes_.push_back(i->first);
 }
 // Dump all the monsters
 for (MonsterTypeVec::iterator i = monsterTypes_.begin();
i != monsterTypes_.end(); ++i)
 {
 std::string m = *i;
 std::cout << m.c_str() << std::endl;
 }
 // Add the Hero to its faction (later allies may join)
 ActorInfo ai, heroInfo;
 hero_.getInitialInfo(&heroInfo);
  // Don't keep the hero's IActor *, because she is 
treated differently
 actors_.insert(std::make_pair((IActor *)0, heroInfo));
 heroFaction_.push_back(&actors_[0]);
 // Instantiate some monsters randomly
 for (apr_int32_t i = 0; i < MONSTER_COUNT; ++i)
 {
 IActor * monster = createRandomMonster(rm);
 monster->getInitialInfo(&ai);
 ai.id = i+1; // Hero is id 0
 actors_.insert(std::make_pair(monster, ai));
 
 enemyFaction_.push_back(&actors_[monster]);
 }
 while (!gameOver_)
 {
 playTurn();
 }
 heroInfo = actors_[0];
 if (heroInfo.health > 0)
 std::cout << "Hero is victorious!!!" << std::endl;
 else
 std::cout << "Hero is dead :-(" << std::endl;
}

```

这是一个动态的步骤。《BattleManager》则没有
一个有什么怪物的线索-不关心。
它甚至不知道《烦躁幽灵》的存在
静态插件。它转储了所有的怪物类型
对于主机来说也是如此
当然，所有的怪物都登记妥当了)。
然后它将Hero(已知并得到)
特殊待遇)在演员名单。
BattleManager需要了解Hero，因为
英雄的命运与整个民族的命运联系在一起
游戏和所有重要的“游戏结束”条件。
然后怪物被随机创造
createRandomMonster()函数。最后，我们得到
对于主循环:“虽然游戏还没有结束
转”。当游戏结束时，是时候
向控制台显示一条令人眼花缭乱的文本消息，

上面写着英雄是赢了还是死了。只要你能
我对这款游戏的美术预算完全是虚构的。
例10包含
createRandomMonster()方法。它选择了
随机怪物按索引，基于总数
注册怪物类型，然后创建它
调用ActorFactory::createActor()方法，
将怪物类型传递给它。

```
IActor * BattleManager::createRandomMonster(const
PluginManager::RegistrationMap & rm)
{
 // Select monster type
 apr_size_t index = ::rand() % monsterTypes_.size();
 const std::string & key = monsterTypes_[index];
 const PF_RegisterParams & rp = rm.find(key)->second;
 // Create it
 IActor * monster = ActorFactory::createActor(key);
 
 return monster;
}
```

ActorFactory是特定于应用程序的对象
派生的适配器
由插件提供的通用ObjectAdapter
框架。从…的角度来看
BattleManager，所有创造的怪物都是公正的
对象来实现IActor接口。的
事实上，它们中的一些是改编的C对象或
一些来自远程插件的欢呼是无关紧要的。
这是一款回合制游戏，所以当玩家
回合是在主循环中播放的吗?例11
包含playTurn()方法，该方法提供
的答案。

```
void BattleManager::playTurn()
{
 // Iterate over all actors (start with Hero)
 //For each actor prepare turn info (friends and foes)
 Turn t;
 ActorInfo & ai = actors_[(IActor *)0];
 t.self = &ai;
 std::copy(heroFaction_.begin(), heroFaction_.end(),
std::back_inserter(t.friends.vec));
 std::copy(enemyFaction_.begin(), enemyFaction_.end(),
std::back_inserter(t.foes.vec));
 hero_.play(&t);
 ActorInfo * p = NULL;
 ActorMap::iterator it;
 for (it = actors_.begin(); it != actors_.end(); ++it)
 {
 if (!it->first || isDead(&it->second))
 continue;
 
 t.self = &(it->second);
 std::copy(heroFaction_.begin(), heroFaction_.end(),
std::back_inserter(t.foes.vec));
 std::copy(enemyFaction_.begin(), enemyFaction_.end(),
std::back_inserter(t.friends.vec));
 it->first->play(&t);
 }
 
 // Clean up dead enemies
 Faction::iterator last =
std::remove_if(enemyFaction_.begin(),
enemyFaction_.end(), isDead);
 while (last != enemyFaction_.end())
 {
 enemyFaction_.erase(last++);
 }
 // Check if game is over (hero dead or all enemies are 
dead)
 if (isDead(&actors_[(IActor *)0]) ||
enemyFaction_.empty())
 {
 gameOver_ = true;
 return;
 }
}
```

BattleManager首先创建一个Turn对象
在堆栈上。每个[现场]演员都得到x
他的Turn对象与适当的信息和行动
它。英雄先走。BattleManager调用
它的play()方法，传递回合对象。
英雄四处移动并发起攻击。的BattleManager
意识到所有发生的行为是因为什么
被操纵的数据结构是
ActorInfo结构体是BattleManager管理的。
一旦英雄完成，其他演员
得到了制造混乱的机会。毕竟
演员们尽了最大的努力，是时候把
战场上的尸体。这是用
标准std::remove_if算法的帮助和
isDead()谓词检查参与者是否有
零生命值。在转弯结束前
BattleManager检查是否英雄或所有的
怪物都死了。游戏一直持续到
这些条件都满足了。图1显示了
游戏正在进行中。

源代码演练
好的，这里有很多源代码。这个主题
很复杂，我不想只给一个
一些示例代码片段和半生不熟的代码
库。我花了很多精力来组织
可重用形式的代码。我把代码分成
多个库和目录根据
它们的功能和依赖关系
库。有一个核心库，两个游戏特定库，游戏本身
可执行文件)和四个不同的插件(三个
动态库和一个静态库)。在
此外，还有两个第三方库
- APR和boost。最后是构建
用于各种平台的系统。

目录结构
有三个顶级目录:include，
Lib和项目。Include和lib包含
外部库和项目包括代码I
写道。include和lib内部是子目录
对于darwin86、linux32和win32，包含
平台特定的头文件(在include中)和
静态库。事实上，唯一真正的图书馆
是APR，因为我只使用头文件从Boost
不需要建造，而且它们是一样的
在所有平台上。所以Boost就在正下方
包含而不是在特定于平台的子目录中。

源代码
所有的源代码都在子目录下
“项目”:
plugin_framework。这是核心库
包含可移植性层(DynamicLibrary，
目录和路径)，插件API定义
(plugin.h)以及所有的支持和帮手
对象。你将需要这个图书馆
基于插件的系统。您可能希望将操作系统
可移植性类在单独的库或
用操作系统抽象层替换它们。我
把它们放在同一个库中，使其独立
包含(参见下面外部的警告)
库部分)。

跑龙套。这是一个愚蠢的小图书馆
包含两个
函数:calcDistance和findnearest。的
它之所以是一个库，是因为它被
Hero是主可执行文件的一部分
并通过一些插件对象，所以它需要
链接到多个二进制文件。你可以
安全地忽略它。
object_model。这是所有游戏特定的双重C/ c++对象驻留的地方
使用ActorFactory，对象模型包装和
混合使用的ActorBaseTemplate类
插件。如果你想使用这个插件
框架为您自己的项目，然后这个
图书馆会给你所有东西的蓝图
你需要去实现。值得注意的是
你可以挑选。如果你不在乎
关于C的兼容性，因为你构建了main
系统和所有的插件，然后你就可以
去掉所有C的东西。另一方面
你关心C，但你不想提供
很好的C/ c++混合API插件开发者，你
可以忽略ActorBaseTemplate和
对象模型包装器。再读一遍文章
(尤其是第3和第4部分)
你就知道该怎么做了

插件库很简单。c_plugin是C插件
(MellowMonster)， cpp_plugin是直接的c++插件
(KillerBunny和StationarySatan)， static_plugin是静态的
链接插件(FidgetyPhantom)和wrapper_plugin是C/ c++的混合
插件(GnarlyGolem和PsychicPiranea)。在大多数
在某些情况下，wrapper_plugin是正确的选择。写起来很容易
插件，如果你投资编写基本模板和对象模型
包装器。与插件相关的开销几乎为零。
great_game。这就是游戏本身。它包含
英雄和BattleManager，当然还有
main()函数。它给你一种很好的感觉
如何引导整个过程
是否参与了集成PluginManager
通过应用程序

本系列文章很好地介绍了这些代码
解释设计决策和交易的原因
走开。我试着保持干净和一致
风格。plugin_framework和object_model
作为系统核心的库是
非常接近工业强度水平(也许吧)
使用更多的错误处理和
文档)。代码的其余部分演示
整个系统在运行。唯一让我
没有令人信服的证明
调用invokeService()。所有的机制都在里面
不过的地方。我调用我的作者特权和
把它作为读者的恐惧练习
中实现几个服务
great_game.cpp中的DummyInvokeService()函数

外部库
游戏和框架使用两个外部
库:APR和boost。Apache Portable
运行时(APR)是一个C库。我用的是便携的
基本类型(所有apr_xxx_t的东西)和一些
(非常少)它的操作系统抽象能力。我
实现了大部分可移植操作系统层
(DynamicLibrary, Directory, and Path)直接作为
在顶级本机Windows上的替代实现
和POSIX api。Boost主要用于
它的shared_ptr和scoped_ptr模板。这是
非常小的ROI(投资回报)的负担
自己与外界的依赖，尤指在
跨平台可移植系统的上下文。
这是真的。我考虑过把所有的
依赖并实现我所需要的一些东西
从APR直接，有我自己的便携式基本类型
头，并获取一个智能指针模板
拖拽Boost。我决定把它们留在
表明这确实是一个严肃的系统，可以
迎合不同的需求，并与之相融合
外部库。我和你一样为此付出了代价
很快就会听到

构建示例游戏及其插件
我在这部分投入了很多努力，因为
跨平台会增加程序的复杂性
一个数量级的系统。如果你不使用
适当的工具和流程可以花费很多
关注特定于平台的问题。
窗口。我提供了一个Visual Studio 2005解决方案
根目录下的文件。这是首选
开发环境的99.99%的Windows
开发人员。Visual Studio Express是免费的
所有的，所以没有成本。有些人认为
跨平台开发意味着一次构建
系统。我不是他们中的一员。我有我的那份
使用cygwin和MinGW的不良经历。很高兴
当它起作用的时候，当它不起作用的时候是很糟糕的。
保持多个构建绝对是件苦差事
系统同步。也许在以后的文章中我会
进一步探讨这个问题。
Mac OS X.我在Mac(和Kubuntu)上使用的是
NetBeans 6.0。我无法形容我有多感动
点。这是一个惊人的c++开发
环境。就差这么一点(“两杆”)
手指靠得很近”)
工作室。它完全击败了Eclipse CDT (C/ c++)
开发工具)。唯一令人讨厌的部分是
Visual Studio得到正确的)是当你设置
项目属性，如预处理器符号，
编译器标志等，必须单独执行
对于每个项目，即使你有20个或200个
需要相同设置的项目。另一个
NetBeans稍显薄弱的部分是
调试器。它使用的是GDB，它不是一流的
c++调试器(你有没有试过
gdb中的std::map ?)最好的部分是
NetBeans 6.0(以及更早的版本)就是这样
它使用Makefiles作为底层构建
系统。这意味着你可以部署你的
源代码给任何人(正如我在这里所做的)和
他们可以这样做:" ./configure;使”,
运行应用程序。我还是鼓励你这么做
如果您不开发，请获取NetBeans 6.0
仅在Windows上运行

Linux (Kubuntu 7.10)。笑点:最终，我
让它工作。故事从我开始
假设我可以带上我的NetBeans
它可以在未成年人的情况下使用
拧一两下。再想想。Kubuntu 7.10发布
安装NetBeans 5.5.1。我很开心
我只是想浏览一下我的项目。不。它
事实证明，只有Java环境才是如此
安装。好吧，我试过更新，但是c++
没有现成的包装。好了，继续。我试着
下载源码从NetBeans.org和
构建它，但我被告知JDK1.6更新
3是必需的。我试着下载它和Kubuntu
问我要安装光盘，现在，我
运行在我从。iso文件安装的虚拟机中
的形象。我删除了4+ gb的iso映像
因为我认为我不需要它(为什么不需要)
从网上更新?)。我不想下载
所以我放弃了NetBeans，我选择了
使用KDevelop。它是原生的KDE开发
环境，c++是其强大的套件，我知道
它使用automake/autoconf作为底层
构建系统。我有大约10个项目，但我
我想检查一下，看看有多容易
曲柄一流的汽车制造为基础的解决方案
多个子项目。嗯，我失败了。我不
我记得所有的细节，但是我失败了。我
记住，我们花了几分钟才找到
的一个垂直选项卡
IDE的右侧，当所有重要的
标签在左边，你可以把它移到左边
当你找到的时候)。我决定退回去
命令行。毕竟是Linux。人们可以
不需要华丽的GUI IDE。我带着我的
netbeans生成的基于makefile的项目和
开始配置和构建。我不得不“嘘”
s/GNU-MacOSX/GNU-Linux-x86/g -i
还有一些更严重的调整，比如
替换链接器标志(结果是
这完全不同)，我走了很远。
所有的项目编译和所有的插件
连接成功。只有great_game本身
由于APR失败，不会链接(找不到)
pthreads大部分)。这说不通啊

当然是安装的。我试着去
查看是否缺少一些开发库。
没有什么结果。我的预构建APR库(实际上是两个)
库)是在不同的Linux上构建的
发行版。我下载了APR源代码，并试图
从源代码构建。造得还不错，但我
仍然有相同的链接错误。
我真的很难过，勉强忍受了痛苦。我
重新下载Kubuntu-7.10巨大的iso镜像文件
安装它。事情从这里开始走下坡路。我
通过apt-get安装JDK 1.6 update 3
安装NetBeans 6.0 c++包
NetBeans.org。它在桌面上创建了一个图标
我推出了NetBeans。它加载了Mac项目
文件很好，我不得不做几个小的
修改和一个更严重的问题。结果
因此，我必须显式地将pthreads添加到
项目中，它在Mac上运行。这么多
对于一个统一的构建系统。最后一切都好了，
经过几次试用(我建立了插件到
目录错了)，我把一切都修好了。
Mac和Linux版本共享相同的Makefile
每个项目目录。这个共享的Makefile
包含子目录中的其他文件
nbproject。有些文件是不同的
Mac和Linux。我想要发布构建系统
所以我在每个目录下创建了两个子目录
这个项目叫做nbproject。MAC和nbproject。linux。
这意味着，如果您尝试打开这些项目
在Mac或Linux上使用NetBeans，您将失败。
您需要重命名适当的sud目录
(基于你的平台)到nbproject。这是
当你有八个不同的项目时很烦人，所以
我创建了一个名为build.sh in的shell脚本
根目录。你应该用任何一个来运行它
' mac '或' linux '，它会复制nbproject。mac或者
nbproject。Linux到nbproject在每个项目和
即使是跑步。你只需要做一次，然后
然后，您将能够打开项目
NetBeans。

建立外部图书馆
好消息是，你不需要这么做。我提供
为APR预先构建的静态库
平台和我使用的Boost子集只是
头文件，所以不需要构建它。
这里也有一个小故事。跨平台的
开发是一项艰巨的任务，所以
公司通常从一个平台开始
他们成长，成熟和代码库
稳定，他们移植到其他平台。Numenta
(我工作的公司)有不同的想法。他们
从一开始就决定跨平台，但是
只做Mac和Linux。这可能是唯一的
从来没有哪个公司会这么做。当我们推出
调研平台上，最常见的投诉是
Windows版本的缺失。我刚刚完成
NuPIC插件框架的第二阶段。
这是插件框架和的祖先
我在这里展示的所有习语。我很快
自愿接手Windows移植我是
只有具有重要Windows专业知识的开发人员
(其他人都是UNIX大师)，但大多数情况下我
希望能够在Visual Studio中调试
忘掉gdb吧。我们的代码库几乎
高级标准c++。有几个安静的人
直接系统调用和许多UNIX路径问题，但是
没什么太可怕的。一切都很顺利，直到我
我要去提振。在这个时候，我们使用了一些Boost
需要建立的库:
Boost::file_system, Boost::regex和
boost::序列化。我辛苦了好几天，搜寻着
网络和奇怪的符号组合，
但我不能在vc++下构建这些库
2005. 最后我不得不写一个操作系统抽象
层在Win32和POSIX api之上进行替换
这些Boost库。目录和路径
类的插件框架是一些
这种努力的成果

NuPIC插件框架
在我让你回家之前，我想和你分享
插件框架的历史。它长在
NuPIC插件框架。NuPIC是一个平台
智能计算，它有一个漂亮的
有趣又干净的建筑。我不
我赞赏它，因为它是在我之前设计的
我也加入了，所以这不是自动恭维
(不要与自动完成混淆)。
NuPIC是一个分布式计算平台
设想运行大量的HTM(分层时态)
内存)网络。术语“节点”通常是
保留给物理机或集群中的主机使用
命名法。在NuPIC中，术语“节点”指的是a
HTM网络中的节点。这些节点可以是
传感器、计算节点或效应器。从一个
从软件工程的角度来看，有很多
节点处理器进程在多个核心上运行
(相同或不同的机器)，每一个
运行多个HTM节点时，有一个主管
协调所有行动的过程
与主管沟通的工具是通过
复杂的API。目标通常是训练一个
HTM网络通过给它提供一些数据，然后
通过显示新数据来执行推理，然后查看
它和神经网络学到的东西有多相似。
运行时引擎是管理者
节点的处理器。应用程序或工具
与主管沟通加载网络
文件，运行计算，并控制活动
网络在运行时通过发送命令和设置
参数。插件在图片中的位置?
其实无处不在。HTM网络中的每个节点
作为插件实现。当网络文件是
将其加载到运行时引擎中，然后对其进行解析和分析
特定节点是基于它们的实例化的
像游戏中的怪物一样打字。图2
是NuPIC的
体系结构

我想看看另一个系统
完全不同的界面有助于澄清
通用插件框架关注点之间的边界
应用程序和它的对象模型。
清单7包含的定义
INode接口。这在道德上相当于
游戏中的IActor界面。这就是
运行时引擎知道它的节点与游戏相同
了解它的演员。

```
struct INode
{
virtual void init(INodeInfo & nodeInfo) = 0;
virtual void compute() = 0;
virtual void execute(IReadBufferIterator & args,
IWriteBuffer & out) = 0;
virtual void getParameter(const Byte * name, IWriteBuffer
& value) = 0;
virtual void setParameter(const Byte * name, IReadBuffer
& value) = 0;
virtual void saveState(IWriteBuffer & state) = 0;
};

```

compute()方法是主要的方法。它叫做
不断重复，这很像
IActor::play()方法。init()方法
初始化节点并允许持久化节点
使用相应的saveState()方法。的
运行时引擎可以告诉节点保存其状态。
稍后，它可以重新创建相同的节点(例如，在
不同的主机)基于状态。
get/setParameter()和execute()方法是
一些新的东西。它们允许插件节点
任意扩展它们的接口。任何节点都可以
具有任意数量的参数并执行命令。
这有效地避开了
定义良好的接口。运行时引擎不能
使用这些特定于节点的参数和命令
因为它在INode接口上工作，但是用户
应用程序构建的网络包含
特定的节点类型，并可以与它们交互
运行时通过获取/设置特定参数和
发送命令。这里没有什么新东西。的
泛型方法使用的数据类型
是IReadBuffer和IWriteBuffer，这是
美化的不透明缓冲区。它们有一些格式
功能类似于IO流
重载了read()和write()方法，但是它已经完成了
要让用户知道各是什么格式
参数，每个命令期望的参数和
传递的有效值是什么?这非常。
很脆弱，类似于拥有你所有的功能
接受一个void指针，即调用者的不透明缓冲区
必须打包，函数必须解包或解析。
这个问题的部分解决方案是节点
节点spec是一个结构体，它定义了
每个参数的名称、类型、元素计数(1)
一个标量，>(1是数组，0是可变大小)，
描述、约束、默认值和访问权限
(创建、获取、设置和组合)。这是很多
元数据并让运行时引擎和通用
工具在非常高的层次上与节点交互。为
例如，工具可以自动显示
作为工具提示的每个参数的描述
验证可以基于数值进行

范围(0.5…1.2)，正则表达式(ab.*)或
枚举(红色、绿色、蓝色)。好的一面是
关于它，所有这些元数据是不可用的
不仅在运行时引擎上，而且在客户端上
(工具)。工具库加载插件
也通过PluginManager，他们有这一切
他们掌握的信息。如果一个用户
错误拼写工具可以允许的命令的名称
她知道就在那里，并显示一个有效的列表
命令。如果她试图设置一个参数
只有一个“get”访问权限，它将得到描述性的
包含访问权限的错误消息
这个参数。
总结
我完成了。谢谢你一路赶来。我
当然，这并不容易。我希望你会使用这个
框架及其支持的模式和习惯用法
在您自己的应用程序和系统中。


