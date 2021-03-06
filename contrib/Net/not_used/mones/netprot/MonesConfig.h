/*!
    \file  MonesConfig.h
    \brief ネットワーク設定

    Copyright (c) 2004 Yamami
    All rights reserved.
    License=MIT/X License

    \author  Yamami 
    \version $Revision$
    \date   create:2004/08/28 update:$Date$
*/

#ifndef _MONA_MONESCONFIG_
#define _MONA_MONESCONFIG_

#include <sys/types.h>

/*!
    class MonesConfig
*/
class MonesConfig {


  private:
  
    //ネットワークコンフィグ 当面ここでコンスタント設定
    //IPアドレス 
    //*****************************************
    //MonaのIPアドレス設定
    //IPアドレス long 2バイトずつ合計4バイト
    // 例:192(C0).168(A8).9(01).1(01) → 0xC0A80101
    //*****************************************
    // TAPネットワーク 192.168.0.2
    static const uint32_t   gl_myIpAdr = 0xC0A80002;       //IPアドレス;
    
    //サブネットマスク 255.255.255.0
    static const uint32_t   gl_mySubnet = 0xFFFFFF00;

    //GW
    // 192.168.0.1 QEMUのTAPなら、TAP-Win32 Adapter V8 のアドレスをGWに
    static const uint32_t   gl_myGw = 0xC0A80001;

    //DNS 設定してもまだ使えない。
    static const uint32_t   gl_myDns = 0xC0A80001;
    
    //MTU  Yamami これは、NICごとに持つべき？？
    static const int   gl_myMTU = 1500;
    
  public:
    MonesConfig();
    ~MonesConfig();
  
    //IPアドレスゲッター
    uint32_t getGl_myIpAdr();
    
    //サブネットマスクゲッター
    uint32_t getGl_mySubnet();
    //GWゲッター
    uint32_t getGl_myGw();
    //DNSアドレスゲッター
    uint32_t getGl_myDns();

    //MTU値 ゲッター
    uint32_t getGl_myMTU();

};

#endif
