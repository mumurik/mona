#include "NicServer.h"
#include <monapi/Thread.h>
#include <monapi/io.h>
using namespace mones;
using namespace MonAPI;
using namespace std;

// not real shared memory
static Ether::Frame sharedFrame;
static Mutex sharedMutex;
std::queue<Ether::Frame*> frameQueue;

void mones::SetFrameToSharedMemory(Ether::Frame* frame)
{
    sharedMutex.lock();
    memcpy(&sharedFrame, frame, sizeof(Ether::Frame));
    sharedMutex.unlock();
}
void mones::GetFrameFromSharedMemory(Ether::Frame* frame)
{
    sharedMutex.lock();
    memcpy(frame, &sharedFrame, sizeof(Ether::Frame));
    sharedMutex.unlock();
}

NicServer::NicServer() : observerThread(0xffffffff), nic(NULL), started(false), loopExit(false)
{
}

NicServer::~NicServer(){
}

bool NicServer::initialize()
{
    syscall_get_io();

    this->nic = NicFactory::create();
    if(this->nic == NULL) 
    {
        _printf("NicFactory error\n");
        return false;
    }
    syscall_set_irq_receiver(this->nic->getIRQ(), SYS_MASK_INTERRUPT);
//    syscall_set_irq_receiver(this->nic->getIRQ(), 0);
    this->nic->enableNetwork();
    this->nic->getMacAddress(this->macAddress);
    this->observerThread= Message::lookupMainThread();
    this->myID = System::getThreadID();
    this->started = true;
    return true;
}

#if 1 /* for Mona */

#pragma pack(2)
    typedef struct{
        uint8_t  dstmac[6];   // 送信先 MAC ID
        uint8_t  srcmac[6];   // 送信元 MAC ID
        uint16_t    type;     // フレームタイプ Frame type(DIX) or frame length(IEEE)
        uint8_t   data[0x600];// Data
    } Frame;

#pragma pack(2)
    typedef struct{
        uint8_t  verhead;  /* バージョン、ヘッダ長。 */
        uint8_t  tos;      /* TOS. */
        uint16_t len;       /* トータル長。 */
        uint16_t id;        /* 識別番号。 */
        uint16_t frag;      /* フラグ、フラグメントオフセット。 */
        uint8_t  ttl;      /* Time to Live. */
        uint8_t  prot;     /* プロトコル番号。 */
        uint16_t chksum;    /* ヘッダチェックサム。 */
        uint32_t srcip;        /* 送り元IP。 */
        uint32_t dstip;        /* 宛先IP。 */
        char     data[0];
    } IPHeader;
#pragma pack(0)
#pragma pack(2)
typedef struct
{
  uint16_t src;                    /* source port */
  uint16_t dst;                    /* destination port */
  uint32_t seq_number;             /* sequence number */
  uint32_t ack_number;             /* acknowledgement number */
#ifdef WORDS_BIGENDIAN
  uint8_t  header_length:4,        /* data offset */
            unused:4;               /* (unused) */
#else
  uint8_t  unused:4,               /* (unused) */
            header_length:4;        /* data offset */
#endif
  uint8_t  flags;
  uint16_t window;                 /* window */
  uint16_t checksum;               /* checksum */
  uint16_t urgent;                 /* urgent pointer */
} TCPHeader;
#pragma pack(0)
    inline static uint32_t swapLong(uint32_t value)
    {
        return (value>>24)+((value>>8)&0xff00)+((value<<8)&0xff0000)+(value<<24);
    }

    inline static uint16_t swapShort(uint16_t value)
    {
        return (value>>8)+(value<<8);
    }
#define TCP_SYN 0x02
#define TCP_ACK 0x10

#endif

void NicServer::interrupt(MessageInfo* msg)
{
    this->nic->inputFrame();
    Ether::Frame* frame = new Ether::Frame;
    this->nic->getFrameBuffer((uint8_t*)frame, sizeof(Ether::Frame));

    if (frame->type == 0x8) { // IP
        const IPHeader* h = (const IPHeader*)frame->data;
        if (h->prot == 6) // TCP
        {
            const TCPHeader* th = (const TCPHeader*)h->data;
            int ack = th->flags & TCP_ACK;
            int syn = th->flags & TCP_SYN;

            uint8_t* tcpdata = (uint8_t*)(h->data + sizeof(TCPHeader));
            for (int i = 0; i < 24; i ++)
            {
                _printf("[%c]", tcpdata[i]);
            }

            logprintf("NICSERVER:%d to %d:len=%4d %s%s seqno=%08x ackno=%08x\n", swapShort(th->src), swapShort(th->dst)
                   , th->header_length, ack ? "ACK " : "", syn ? "SYN " : "   ", swapLong(th->seq_number),swapLong(th->ack_number));
        } else {
            _printf("not tcp\n");
        }
    } else {
        _printf("not ip\n");
    }
    frameQueue.push(frame);
    return;
}

void NicServer::messageLoop()
{
    for (MessageInfo msg; !loopExit;)
    {
        if (Message::receive(&msg)) continue;

        switch (msg.header)
        {
        case MSG_INTERRUPTED:
        {
            this->interrupt(&msg);
            monapi_set_irq(this->nic->getIRQ(), MONAPI_TRUE, MONAPI_TRUE);
            break;
        }
        default:
            printf("default come %d", msg.header);
            break;
        }
    }
    printf("NicServer exit\n");
}
