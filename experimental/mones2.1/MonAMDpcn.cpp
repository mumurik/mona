//$Id$
#include "MonAMDpcn.h"
using namespace MonAPI;
using namespace mones;


///////////////////////////////////////////////////////
MonAMDpcn::~MonAMDpcn()
{
    printf("destructor of monamdpcn\n");
}

MonAMDpcn::MonAMDpcn():rxdsc(NULL),txdsc(NULL),
                       rxindex(0),txindex(0),piblock(NULL)
{
    memcpy(devname,"pcnet32",8);
}

int MonAMDpcn::init()
{    
    AllocateDmaPages( (1<<LOGRXRINGLEN)+ (1<<LOGTXRINGLEN) +1);//pages.
    //initialize rx
    if( dma_head== NULL ){
        printf("buffer allocation was failed.");
        return -1;
    }
    //printf("DMA area was allocated properly.\n");
    rxdsc = (RXDSC*)dma_head;
    rxbuf = dma_head+((1<<LOGRXRINGLEN)*sizeof(RXDSC));
    for(int i=0;i<(1<<LOGRXRINGLEN);i++){
        (rxdsc+i)->bcnt=(word)(-ETHER_MAX_PACKET)|0xF000;
        (rxdsc+i)->status=RMD1_OWN|RMD1_STP|RMD1_ENP;
        (rxdsc+i)->rbaddr=(dword)(rxbuf+i*ETHER_MAX_PACKET);
    }
     //initialize tx 
    txdsc= (TXDSC*)( dma_head +(0x1000* (1<<LOGRXRINGLEN)));
    txbuf = dma_head + 0x1000*(1<<LOGRXRINGLEN) + (1<<LOGTXRINGLEN)*sizeof(TXDSC) ;
    for(int i=0;i<(1<<LOGTXRINGLEN);i++){
        (txdsc+i)->status=0;
        (txdsc+i)->control=0;
        (txdsc+i)->bcnt=0;
        (txdsc+i)->rbaddr=(dword)(txbuf+i*ETHER_MAX_PACKET);
    }
    ///////////////
    stop();
    reset();
    w_bcr(BCR_MISC,BCR_AUTOSEL);        //SET BCR_EDGETRG for Edge Sense.
    w_bcr(BCR_SSTYLE,BCR_PCI_II|BCR_SSIZE);
    //Use initalize block.
    piblock= (IBLK*)( dma_head  + (0x1000*( (1<<LOGRXRINGLEN)+(1<<LOGTXRINGLEN) )) );
    piblock->mode=0x0;         //set MODE_DNY_BCST for deny broadcast packets.
    piblock->rxlen=(LOGRXRINGLEN<<4);  //see page157.
    piblock->txlen=(LOGTXRINGLEN<<4);
    for(int i=0;i<6;i++){
        piblock->mac_addr[i]=inp8(iobase+i);
        macaddress[i]=piblock->mac_addr[i];
    }    
    piblock->filter[0]=0x0;
    piblock->filter[1]=0x0;
    piblock->rx_ring=(dword)rxdsc;
    piblock->tx_ring=(dword)txdsc;

    w_csr(CSR_IADR0, ((dword)piblock)&0xFFFF);
    w_csr(CSR_IADR1,(((dword)piblock)>>16)&0xFFFF);
    w_csr(CSR_CSR,CSR_INTEN|CSR_INIT);
    sleep(100);
    w_csr(CSR_FEATURE,FEAT_PADTX|FEAT_TXMSK|0x915);    //CSR 4
    //printf("chip version=%x\n",(r_csr(88)>>12)|(r_csr(89)<<4));
    w_csr(CSR_CSR,CSR_INTEN|CSR_START);          //CSR 0
    return 0;    
}

int MonAMDpcn::interrupt()
{
    word val;
    word ret=0x0000;
    if( ( val = r_csr(CSR_CSR)) & CSR_INTR ){
        if( val & CSR_RINT ){
            rxihandler();
            ret |= RX_INT;
        }
        if (val & CSR_TINT){
            txihandler();
            ret |= TX_INT;
        }

    }
    //error.
    w_csr(CSR_CSR, val & 0xFFF0);
    //Interrupt was masked by OS handler.
    enableNetwork(); //Now be enabled here. 
    return ret;
}

void MonAMDpcn::txihandler()
{
    //printf("TX\n");
    int i=txindex;
    while( ( (txdsc+i)->control & TMD1_OWN)==TMD1_OWN ){
        (txdsc+i)->control=0;
        i--;
        if( i<0 ){i=(( 1<<LOGTXRINGLEN)-1);}
    }
}

void MonAMDpcn::rxihandler()
{
    word length;
    while( ((rxdsc+rxindex)->status & RMD1_OWN) == 0 ){
        length=(((rxdsc+rxindex)->mcnt)&0x0FFF);
        Ether* frame = new Ether; //deleted by server.
        memcpy(frame,(byte*)((rxdsc+rxindex)->rbaddr),length);
        //printf("SIZE:%d\n",length);
        rxFrameList.add(frame);
        (rxdsc+rxindex)->mcnt=0;
        (rxdsc+rxindex)->bcnt = (word)(-ETHER_MAX_PACKET)|0xF000;
        (rxdsc+rxindex)->status = RMD1_OWN|RMD1_STP|RMD1_ENP;  
        rxindex = (rxindex+1) & ((1<<LOGRXRINGLEN)-1);
    }
}
void MonAMDpcn::SendFrm(Ether* frame)
{
    enableNetwork();
    word len=CalcFrameSize(frame);
    txFrameList.add(frame);
    while( txFrameList.size() != 0) {
        Ether* frame = txFrameList.removeAt(0);
        memcpy(txbuf+txindex*ETHER_MAX_PACKET,frame,len);
        (txdsc+txindex)->status=0;
        (txdsc+txindex)->bcnt=(word)(-len)|0xF000;
        //TODO: must be chaged.
        (txdsc+txindex)->control=TMD1_OWN|TMD1_STP|TMD1_ENP;
        (txdsc+txindex)->rbaddr=(dword)(txbuf+txindex*ETHER_MAX_PACKET);
        w_csr(CSR_CSR,CSR_TDMD|CSR_INTEN);     
        txindex = (txindex+1) & (( 1<<LOGTXRINGLEN)-1);
        delete frame;
    }
}
