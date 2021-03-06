/*!
    \file   MoIp.cpp
    \brief  IPプロトコル実装クラス

    Copyright (c) 2004 Yamami
    All rights reserved.
    License=MIT/X License

    \author  Yamami
    \version $Revision$
    \date   create:2004/09/20 update:$Date$
*/

/*! \class MoIp
 *  \brief IPプロトコル実装クラス
 */


#include "MonesGlobal.h"

/*!
    \brief initialize
         MoIp コンストラクタ
    \author Yamami
    \date   create:2004/09/20 update:
*/
MoIp::MoIp()
{

}

/*!
    \brief initialize
         MoIp initIp
    \author Yamami
    \param  AbstractMonic *pminsNic [in] NICクラスへのポインタ
    \date   create:2004/09/20 update:
*/
void MoIp::initIp(AbstractMonic *pminsNic ) 
{
    //NICクラスを保持
    insAbstractNic = pminsNic;
    return;
}


/*!
    \brief initialize
         MoIp デスクトラクタ
    \author Yamami
    \date   create:2004/08/20 update:
*/
MoIp::~MoIp() 
{

}


/*!
    \brief receiveIp
         IPプロトコル受信 処理
    \param  IP_HEADER *ipHead [in] IPヘッダへのポインタ
    \return int 結果 
        
    \author Yamami
    \date   create:2004/09/20 update:2004/09/20
*/
int MoIp::receiveIp(IP_HEADER *ipHead)
{

    /* チェックサムの確認。 */
    if(MoPacUtl::calcCheckSum((uint32_t*)ipHead,(ipHead->verhead&0xf)*4)){
        return 0;
    }


//2005/05/06 プロトコルタイプを表示
//printf("ipHead->prot:%x \n",ipHead->prot);

    /* 上位プロトコルへ渡す。 */
    switch(ipHead->prot)
    {
        case IPPROTO_TCP:
          return g_MoTcp->receiveTcp(ipHead);
        case IPPROTO_UDP:
          return g_MoUdp->receiveUdp(ipHead);
        case IPPROTO_ICMP:
          return g_MoIcmp->receiveIcmp(ipHead);
    }

    return 0;
}


/*!
    \brief transIp
         IPプロトコル送信 処理
    \param  TRANS_BUF_INFO *tbi [in] 送信バッファ構造体へのポインタ
    \param  uint32_t dstip [in] 送信先IPアドレス(エンディアン変換済みが前提)
    \param  uint8_t tos [in] サービスタイプ
    \param  int flag [in] フラグ
    \return int 結果 

    \author Yamami
    \date   create:2004/09/20 update:2004/09/20
*/
int MoIp::transIp(TRANS_BUF_INFO *tbi, uint32_t dstip, uint8_t tos, int flag)
{

    uint8_t *transPacket; //パケット送信バッファ

    uint8_t dstmac[6];
    uint32_t transip;
    //int num;
    int rest;
    IP_HEADER ipHead;
    int max,total;
    //int trans;

    int ret;
    //ルーティング解決
    ret = ipRouting(dstip , &transip);
    //transip = dstip;

    // 送信先 MACアドレス取得(ARP解決)
    if((rest=g_MoArp->getMac(transip,dstmac)) != 0){
        //Yamami デバッグ
        //printf("MoIp::transIp rest = %d!!\n",rest);

        //ARPキャッシュでは見つからず、IP送信はすぐにできないので、待ちへ
        //パケット内容を待ちリストへ追加
        MAC_REPLY_WAIT* nowWait;
        nowWait = g_MoArp->macWaitList->get(rest-1);
        
        //パケットは共有メモリへ格納
        monapi_cmemoryinfo* ret = monapi_cmemoryinfo_new();
        if (!monapi_cmemoryinfo_create(ret, tbi->size[1], false))
        {
            monapi_cmemoryinfo_delete(ret);
            return rest;
        }
        
        memcpy(ret->Data,tbi->data[1],tbi->size[1]);
        nowWait->ipPacketBuf01 = ret;
        
        nowWait->ipType = tbi->ipType;
        
        return rest;
    }

    //IPヘッダを作成する。
    ipHead.verhead=IP_HEAD_VERSION|(sizeof(IP_HEADER)/4);
    ipHead.tos=tos;
    
    //GET_ID(ipHead.id);
    //Yamami ここはスレッドセーフにする必要がある？
    ipHead.id++;
    
    ipHead.ttl=255;
    ipHead.prot=tbi->ipType;
    ipHead.srcip=MoPacUtl::swapLong(G_MonesCon.getGl_myIpAdr());
    ipHead.dstip=dstip;
    ipHead.chksum=0;

    //送信TBL
    tbi->type=ETHER_PROTO_IP;
    tbi->data[0]=(char*)&ipHead;
    tbi->size[0]=sizeof(IP_HEADER);         // IP header size.
    //max=ethDev[num].mtu-sizeof(IP_HEADER);  // データ最大送信サイズ。
    max=G_MonesCon.getGl_myMTU()-sizeof(IP_HEADER);  // データ最大送信サイズ。
    total=tbi->size[1]+tbi->size[2];        // データトータルサイズ。

    if(total<=max)
    {
        
        //送信要求パケットサイズがMTU以下ならば
        //IPサイズ
        ipHead.len=MoPacUtl::swapShort(tbi->size[0]+total);
        //フラグ
        ipHead.frag=MoPacUtl::swapShort(flag<<14);
        //チェックサム
        ipHead.chksum=MoPacUtl::calcCheckSum((uint32_t*)&ipHead,tbi->size[0]);
        
        //抽象NICを使って送信!        
        //パケット作成
        transPacket = (uint8_t *)malloc(sizeof(IP_HEADER) + total);
        memcpy(transPacket,(uint8_t *)&ipHead,sizeof(ipHead));
        memcpy(transPacket + sizeof(ipHead),tbi->data[1],tbi->size[1]);
        
        insAbstractNic->frame_output(transPacket , (uint8_t *)dstmac , sizeof(IP_HEADER) + total , ETHER_PROTO_IP);
        
        //パケットバッファ解放
        free(transPacket);
    }

/*
    else
    {
        //送信要求パケットサイズがMTU以上ならば  未実装
        ipHead.len=swapWord(tbi->size[0]+max);
        ipHead.frag=swapWord(IP_HEAD_FRAG_ON);
        tbi->size[2]=max-tbi->size[1];
        ipHead.chksum=calcSum((uint*)&ipHead,tbi->size[0]);
        ethDev[num].dev->write(ethDev[num].dev->linf,(size_t)tbi,(size_t)dstmac);

        trans=max;                      // 送信済サイズ
        tbi->data[2]+=max-tbi->size[1];
        tbi->size[1]=0;

        for(;;)
        {
            ipHead.chksum=0;

            if(total-trans<=max)
            {
                ipHead.len=swapWord(tbi->size[0]+total-trans);
                ipHead.frag=swapWord(trans);
                GET_ID(ipHead.id);
                ipHead.chksum=calcSum((uint*)&ipHead,tbi->size[0]);
                tbi->size[2]=total-trans;
                ethDev[num].dev->write(ethDev[num].dev->linf,(size_t)tbi,(size_t)dstmac);
                break;
            }
            else
            {
                ipHead.len=swapWord(tbi->size[0]+max);
                ipHead.frag=swapWord(trans|IP_HEAD_FRAG_ON);
                GET_ID(ipHead.id);
                ipHead.chksum=calcSum((uint*)&ipHead,tbi->size[0]);
                tbi->size[2]=max;
                ethDev[num].dev->write(ethDev[num].dev->linf,(size_t)tbi,(size_t)dstmac);
            }

            trans+=max;
            tbi->data[2]+=max;
        }
    }
*/

    return 0;
}


 
/*!
    \brief ipRouting
         IPルーティング 処理
    \param  uint16_t ip [in] 送信先IPアドレス
    \param  uint32_t dstip [in] 転送先IPアドレス(他のサブネットならルータとなる)
    \return int 結果 

    \author Yamami
    \date   create:2004/10/30 update:2004/10/30
*/
int MoIp::ipRouting(uint32_t ip,uint32_t *transip)
{

    //同一サブネット内かチェックする。
    if((ip & MoPacUtl::swapLong(G_MonesCon.getGl_mySubnet())) 
        == (MoPacUtl::swapLong(G_MonesCon.getGl_myIpAdr()) & MoPacUtl::swapLong(G_MonesCon.getGl_mySubnet())))
    {
        *transip=ip;
        return 0;
    }

    /* デフォルトゲートウェイへ。 */
    *transip=MoPacUtl::swapLong(G_MonesCon.getGl_myGw());
    return 0;
}
