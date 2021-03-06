/*!
    \file   MoIcmp.cpp
    \brief  ICMPプロトコル実装クラス

    Copyright (c) 2004 Yamami
    All rights reserved.
    License=MIT/X License

    \author  Yamami
    \version $Revision$
    \date   create:2004/09/20 update:$Date$
*/

/*! \class MoIcmp
 *  \brief ICMPプロトコル実装クラス
 */



#include "MoIcmp.h"
#include "MonesConfig.h"
#include "MonesGlobal.h"

/*!
    \brief initialize
         MoIcmp コンストラクタ
    \author Yamami
    \date   create:2004/09/20 update:
*/
MoIcmp::MoIcmp()
{

}

/*!
    \brief initialize
         MoIcmp initIp
    \author Yamami
    \param  AbstractMonic *pminsNic [in] NICクラスへのポインタ
    \date   create:2004/09/20 update:
*/
void MoIcmp::initIp(AbstractMonic *pminsNic ) 
{
    //NICクラスを保持
    insAbstractNic = pminsNic;
    return;
}


/*!
    \brief initialize
         MoIcmp デスクトラクタ
    \author Yamami
    \date   create:2004/08/20 update:
*/
MoIcmp::~MoIcmp() 
{

}


/*!
    \brief receiveIcmp
         ICMPプロトコル受信 処理
    \param  IP_HEADER *ipHead [in] IPヘッダへのポインタ
    \return int 結果 
        
    \author Yamami
    \date   create:2004/09/20 update:2004/09/20
*/
int MoIcmp::receiveIcmp(IP_HEADER *ipHead)
{
    
    int icmp_size;
    ICMP_HEADER *icmp;

    icmp=(ICMP_HEADER*)ipHead->data;

    icmp_size=MoPacUtl::swapShort(ipHead->len)-sizeof(IP_HEADER);

    /* チェックサムの確認。 */
    if(MoPacUtl::calcCheckSum((uint32_t*)icmp,icmp_size)){
        return 0;
    }


    switch(icmp->type)
    {
        case ICMP_TYPE_ECHOREQ:
            transIcmp(ipHead->srcip,ICMP_TYPE_ECHOREP,0,icmp,icmp_size);
            break;
        case ICMP_TYPE_ECHOREP:
            saveRecv(ipHead,icmp_size+sizeof(IP_HEADER));
            break;
    }

    return 0;
}


/*!
    \brief transIcmp
         ICMP送信 処理
    \param  uint16_t dstip [in] 送信先IPアドレス
    \param  uint8_t type [in] ICMPタイプ
    \param  ICMP_HEADER *icmpHead [in] ICMPヘッダへのポインタ
    \param  int size [in] パケットサイズ
    \return 無し
        
    \author Yamami
    \date   create:2004/09/20 update:2004/09/20
*/
void MoIcmp::transIcmp(uint32_t dstip, uint8_t type, uint8_t code, ICMP_HEADER *icmpHead, int size)
{
    
    TRANS_BUF_INFO tbi;

    //ICMPヘッダーの設定
    icmpHead->type=type;
    icmpHead->code=code;
    icmpHead->chksum=0;
    icmpHead->chksum=MoPacUtl::calcCheckSum((uint32_t*)icmpHead,size);

    //送信バッファテーブルの設定
    tbi.data[2]=NULL;
    tbi.size[2]=0;
    tbi.data[1]=(char*)icmpHead;
    tbi.size[1]=size;
    tbi.ipType=IPPROTO_ICMP;

    g_MoIp->transIp(&tbi,dstip,0,0);
}


/*!
    \brief saveRecv
         ICMP応答受信 処理
    \param  IP_HEADER *ipHead [in] IPヘッダ
    \param  int size [in] パケットサイズ
    \return 無し
    \author Yamami
    \date   create:2004/09/20 update:2004/09/20
*/
void MoIcmp::saveRecv(IP_HEADER *ipHead, int size)
{
    MessageInfo info;
    MONES_IP_REGIST *regist;

//Yamamiデバッグ
//printf("MoIcmp::saveRecv Call!!\n");
//printf("MonesRList->size() = %d \n",MonesRList->size());

    //登録しているプロセスに通知する。
    for (int i = 0; i < MonesRList->size() ; i++) {
        regist = MonesRList->get(i);
        
        //printf("regist->ip = %x \n",regist->ip);
        //printf("MoPacUtl::swapLong(ipHead->srcip) = %x \n",MoPacUtl::swapLong(ipHead->srcip));
        
        if(regist->ip == MoPacUtl::swapLong(ipHead->srcip) ){
            //登録されているIPへのリプライならば、メッセージ通知
            // create message
            Message::create(&info, MSG_MONES_ICMP_NOTICE, 0, 0, 0, NULL);
            memcpy(info.str , ipHead,size);
            info.length = size;
            // send
            if (Message::send(regist->tid, &info)) {
                //printf("MoIcmp::saveRecv error\n");
            }
            break;
            
        }
    }
}
