#include <monapi.h>
#include <sys/HashMap.h>

using namespace MonAPI;

int MonaMain(List<char*>* pekoe)
{
    int reti;

    char buf[1024];
    memset(buf, 0, sizeof(buf));

    sprintf(buf , "protType:%04x \n",1234);

    //FileOutputStream �ƹ�
    FileOutputStream fos("YTEST.TXT");

    printf("fileout:open=%d\n", fos.open());

    reti = fos.write((byte*)buf , 512);
    printf("fileout:read=%d\n", reti);

    printf("fos close pre\n");
    fos.close();
    printf("fos close after\n");

    return 0;
}


#if 0
typedef union
{
    dword command;
    struct
    {
        unsigned reserved1 : 2;
        unsigned address   : 6;
        unsigned function  : 3;
        unsigned device    : 5;
        unsigned bus       : 8;
        unsigned reserved2 : 7;
        unsigned enabled   : 1;
    } p;
} PciPacket;

enum
{
    REG_CONFIG_ADDRESS = 0x0CF8,
    REG_CONFIG_DATA    = 0x0CFC


};

int MonaMain(List<char*>* pekoe)
{

    syscall_get_io();


    for (int i = 0; i < 32; i++)
    {
        PciPacket packet;
        dword in;

        packet.p.enabled = 1;
        packet.p.bus = 0;
        packet.p.device = i;
        packet.p.function = 0;
        packet.p.address = 0;
        packet.p.reserved1 = 0;
        packet.p.reserved2 = 0;

        outp32(REG_CONFIG_ADDRESS, packet.command);

        in = inp32(REG_CONFIG_DATA);

        printf("[%x:%x]\n", in & 0x0000FFFF, in >> 16);

        packet.p.enabled = 0;
        outp32(REG_CONFIG_ADDRESS, packet.command);
    }
}

#endif

#if 0
int MonaMain(List<char*>* pekoe)
{
    syscall_get_io();


//     for (int i = 0; i < 500; i++)
//     {
//     printf("Hello World!\n");
//     }
//     dword p;
//     dword* q;

//     q = (dword*)malloc(sizeof(dword) * 2);

//     p = 0x12345678;
//     q[0] = 0x87654321;

//     printf("p = %x address=%x\n", p, &p);
//     syscall_test((dword)(&p));
//     printf("p = %x address=%x\n", p, &p);

//     printf("q = %x address=%x\n", q[0], q);
//     syscall_test((dword)(q));

//     //HashMap�ƹ�
//     HashMap<int>* testHash;
//     testHash = new HashMap<int>(20);

//     testHash->put("AAA" , 10);
//     //testHash->put("BBB" , 55);
//     //testHash->put("ZZZ" , 88);

//     int aaaVal = testHash->get("AAA");
//     int bbbVal = testHash->get("BBB");

//     printf("aaaVal = %d : bbbVal = %d \n" ,aaaVal,bbbVal);

//     char buf[32];
//     sprintf(buf, "sin(3.14 / 2) = %0.8f\n", sin(3.14 / 2));
//     printf("%s", buf);
//     sprintf(buf, "cos(3.14 / 2) = %0.8f\n", cos(3.14 / 2));
//     printf("%s", buf);
//     sprintf(buf, "sqrt(1.44) = %0.8f\n", sqrt(1.44));
//     printf("%s", buf);

//     double seed = 0.31415926535897931;
//     for (int i = 0; i < 10000000; i++)
//     {
//         seed *= 17.0;
//         seed -= (int)seed;
//     }

//     char buf[64];
//     sprintf(buf, "%1.8f", seed);
//     printf("answer=%s", buf);

    return 0;
}

// int MonaMain(List<char*>* pekoe)
// {
//     byte buf[32];
//     int result;
//    FileInputStream fis("HOGE.TXT");

//    if (0 != (result = fis.open()))
//    {
//        printf("can not open %s\n", pekoe->get(0));
//        return -1;
//    }

//    printf("file size = %d\n", fis.getFileSize());

//    if (fis.read(buf, 32))
//    {
//        printf("can not read %s\n", pekoe->get(0));
//        fis.close();
//        return -1;
//    }

//    printf("contents\n");
//    sleep(5000);

//    for (int i = 0; i < 32; i++)
//    {
//        printf("[%x]", buf[i]);
//    }

//    fis.close();



//     printf("Hello World\n");

//     for (int i = 0; i < 10; i++)
//     {
//         printf("tick=%x\n", System::getTick());
//     }

//     sleep(100);

//     for (int i = 0; i < 10; i++)
//     {
//         printf("tick=%x\n", System::getTick());
//     }

//     return 0;
// }

// Mutex��ȤäƤߤ���E// static Mutex* mutex;

// void sub()
// {
//     printf("sub start\n");
//     sleep(5000);

//     printf("sub:tryLock=%x\n", mutex->tryLock());

//     sleep(10000);

//     printf("sub:lock start\n");
//     printf("sub:Lock=%x\n", mutex->lock());
//     printf("sub:lock done\n");

//     for (;;);
// }

// int MonaMain(List<char*>* pekoe)
// {
//     mutex = new Mutex();

//     int id = syscall_mthread_create((dword)sub);
//     printf("join result = %x", syscall_mthread_join(id));

//     printf("main:lock=%x\n", mutex->lock());

//     sleep(20000);

//     mutex->unlock();
//     printf("main:unlock\n");

//     return 0;
// }

// ��ͭ����ʤ��
// int MonaMain(List<char*>* pekoe)
// {
    /* 5000byte�ζ�ͭ��⥁E�������Х�E���ܮ �ºݤΥ�������8192byte�ˤʤ�E*/
//     dword id1 = MemoryMap::create(5000);

//     if (id1 == 0)
//     {
//         printf("map create error = %x", MemoryMap::getLastError());
//         exit(1);
//     }

//     printf("shared size = %d", MemoryMap::getSize(id1));

//     /* ��ܮ������ͭ�����ʬ�ζ��֤�Ž��Eդ���E*/
//     byte* p = MemoryMap::map(id1);
//     if (p == NULL)
//     {
//         printf("map error\n");
//         exit(1);
//     }

//     printf("map=[%x]", p);

//     /* ��ͭ����E��˽񤭹��� */
//     strcpy((char*)p, "data share top hello!!\n");

//     /* ���ᤷ��Server��id1�����äƤߤ褦 */
//     dword targetID = Message::lookupMainThread("KEYBDMNG.BIN");
//     if (targetID == 0xFFFFFFFF)
//     {
//         printf("hello:Server not found\n");
//         exit(1);
//     }

//     MessageInfo info;
//     Message::create(&info, MSG_MEMORY_MAP_ID, id1, 0, 0, NULL);

//     /* send */
//     if (Message::send(targetID, &info)) {
//         printf("hello send error\n");
//     }

    /* ��ͭ�����ʬ�ζ��֤���Ϥ��� */
    //MemoryMap::unmap(id1);

    /* �Ĥ��Ǥ˥ե�����EǤ����E� */
// //     FileOutputStream fos("HELLO.LOG", true);

// //     printf("open=%x\n", fos.open());

// //     char str[] = "Hello! \n Mona can create file and write\n";
// //     printf("write%x\n", fos.write((byte*)str, sizeof(str)));
// //     printf("write%x\n", fos.write((byte*)str, sizeof(str)));
// //     fos.close();
// //     for (;;);
// }


// �ե�����E���Ȥ�Ϥ����
// int MonaMain(List<char*>* pekoe)
// {
//     int result;
//     byte buf[32];

//     if (pekoe->isEmpty())
//     {
//         printf("usage: HELLO.ELF pathToFile\n");
//         return -1;
//     }

//     FileInputStream fis(pekoe->get(0));

//     if (0 != (result = fis.open()))
//     {
//         printf("can not open %s\n", pekoe->get(0));
//         return -1;
//     }

//     printf("file size = %d\n", fis.getFileSize());

//     if (fis.read(buf, 32))
//     {
//         printf("can not read %s\n", pekoe->get(0));
//         fis.close();
//         return -1;
//     }

//     printf("contents\n");
//     sleep(5000);

//     for (int i = 0; i < 32; i++)
//     {
//         printf("[%x]", buf[i]);
//     }

//     fis.close();

//     return 0;
// }

// �ޥ�E�����Eåɤʤ��
// void sub1() {for (;;) printf("sub1");}
// void sub2() {for (;;) printf("sub2");}
// void sub3() {for (;;) printf("sub3");}

// int MonaMain(List<char*>* pekoe)
// {
//     int id;

//     id = syscall_mthread_create((dword)sub1);
//     printf("join result = %x", syscall_mthread_join(id));

//     id = syscall_mthread_create((dword)sub2);
//     printf("join result = %x", syscall_mthread_join(id));

//     id = syscall_mthread_create((dword)sub3);
//     printf("join result = %x", syscall_mthread_join(id));

//     for (;;)
//     {
//         printf("main");
//     }

//     return 0;
// }

//     Floppy fd(Floppy::FLOPPY_1);
//     fd.open();
//     printf("changed %s\n", fd.diskChanged() ? "true" : "false");
//     sleep(10000);
//     printf("changed %s\n", fd.diskChanged() ? "true" : "false");
//     fd.close();

#endif
