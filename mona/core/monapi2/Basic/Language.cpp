/**
	@file	Language.cpp
	@brief	言語・文字コード

	言語・文字コード

	License=Mona License
	@version $Revision$
	@date	$Date$
*/
//バグ修正をする時は関数本体説明の@date履歴に日付と名前と作業内容を付け足しておいてください。
//また.hファイルにあるクラス説明などの@date履歴部分にも同様の事をしておいてください。
#ifdef MONA
#include <monapi.h>
#endif

#include "Language.h"
#include "StringFn.h"
#include "Buffer.h"
#include "FileFn.h"


namespace monapi2	{

//UnicodeとShiftJISの変換ルーチン。
//一度ファイルを読む込む初期化作業が必要なのでオブジェクトとして存在している。
CLanguageCodeConverter g_ShiftJISUnicodeConverter;

//変換領域。
Rect g_arectConversionShiftJIStoUnicode[5+1];
Rect g_arectConversionUnicodeShiftJISto[8+1];


//LanguageFn/////////
/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertShiftJIStoUnicode(pcharv wszOut,cpchar1 cszIn,int iMaxInLen)
{
	if (cszIn==NULL || iMaxInLen==0)	return 0;

	pcharv	pWriteStart	= wszOut;
	pcharv	pWrite		= pWriteStart;
	cpchar1	pReadStart	= cszIn;
	cpchar1	pRead		= pReadStart;
	cpchar1	pReadEnd	= pRead + iMaxInLen;

	char1 c;
	while ((c=*pRead) != '\0')
	{
		if (CharFn::isASCII(c))
		{
			*pWrite = (charv)convertShiftJIStoUnicode(0,c);
			pRead++;
		}
		else
		{
			*pWrite = (charv)convertShiftJIStoUnicode(pRead[0],pRead[1]);
			pRead+=2;
		}
		pWrite++;

		if (iMaxInLen>=0 && pRead>=pReadEnd)		break;
	}

	*pWrite = '\0';
	return pWrite - pWriteStart;
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertUnicodetoShiftJIS(pchar1 szOut,cpcharv cwszIn,int iMaxInLen)
{
	pchar1	pWriteStart	= szOut;
	pchar1	pWrite		= pWriteStart;
	cpcharv	pRead		= cwszIn;
	cpcharv	pReadEnd	= pRead + iMaxInLen;

	charv c;
	while ((c=*pRead) != '\0')
	{
		if (c<0x80)
		{
			*pWrite++ = (char1)convertUnicodetoShiftJIS((uint)c);
		}
		else
		{
			int iReturn = convertUnicodetoShiftJIS((uint)c);
			*pWrite++ = (char1)((iReturn & 0xFF00) >> 8);
			*pWrite++ = (char1)(iReturn & 0xFF);
		}

		pRead++;

		if (iMaxInLen>=0 && pRead>=pReadEnd)
			break;
	}

	*pWrite = '\0';
	return pWrite - pWriteStart;
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertShiftJIStoUnicode(Buffer* pbufOut,cpchar1 cszIn)
{
	int iSizeIn = StringFn::getLength(cszIn) + 1;
	
	pbufOut->extendBuffer(iSizeIn * 2);
	return convertShiftJIStoUnicode((pcharv)pbufOut->getBuffer(),cszIn);
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertUnicodetoShiftJIS(Buffer* pbufOut,cpcharv cwszIn)
{
	int iSizeIn = (StringFn::getLength(cwszIn) + 1) * 2;

	pbufOut->extendBuffer(iSizeIn/2);
	return convertUnicodetoShiftJIS((pchar1)pbufOut->getBuffer(),cwszIn);
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertShiftJIStoUnicode(int iSJISCode)
{
	return g_ShiftJISUnicodeConverter.convert1to2(iSJISCode);
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertShiftJIStoUnicode(uint8_t x,uint8_t y)
{
	return g_ShiftJISUnicodeConverter.convert1to2(x,y);
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertUnicodetoShiftJIS(int iUnicodeCode)
{
	return g_ShiftJISUnicodeConverter.convert2to1(iUnicodeCode);
}

/**
	@brief	説明、引数、戻り値はMonapi2リファレンス参照。
	@date	2005/08/20	junjunn 作成
*/
int LanguageFn::convertUnicodetoShiftJIS(uint8_t x,uint8_t y)
{
	return g_ShiftJISUnicodeConverter.convert2to1(x,y);
}

/**
	@brief	ルールを読み込む。
	@date	2005/08/20	junjunn 作成
*/
void LanguageFn::initRule()
{
	g_arectConversionShiftJIStoUnicode[0].setRectRB(0x00,0x00	,0x00 + 1,0xDF + 1);
	g_arectConversionShiftJIStoUnicode[1].setRectRB(0x81,0x40	,0x9F + 1,0xFC + 1);
	g_arectConversionShiftJIStoUnicode[2].setRectRB(0xE0,0x40	,0xEE + 1,0xFC + 1);
	g_arectConversionShiftJIStoUnicode[3].setRectRB(0xFA,0x40	,0xFB + 1,0xFC + 1);
	g_arectConversionShiftJIStoUnicode[4].setRectRB(-1,0,0,0);	//番兵

	g_arectConversionUnicodeShiftJISto[0].setRectRB(0x00,0x00	,0x00 + 1,0xF7 + 1);
	g_arectConversionUnicodeShiftJISto[1].setRectRB(0x03,0x91	,0x03 + 1,0xC9 + 1);
	g_arectConversionUnicodeShiftJISto[2].setRectRB(0x04,0x01	,0x04 + 1,0x51 + 1);
	g_arectConversionUnicodeShiftJISto[3].setRectRB(0x20,0x00	,0x26 + 1,0xE9 + 1);
	g_arectConversionUnicodeShiftJISto[4].setRectRB(0x2F,0x00	,0x33 + 1,0xFE + 1);
	g_arectConversionUnicodeShiftJISto[5].setRectRB(0x4E,0x00	,0x9E + 1,0xFF + 1);
	g_arectConversionUnicodeShiftJISto[6].setRectRB(0xF9,0x0E	,0xFA + 1,0xDC + 1);
	g_arectConversionUnicodeShiftJISto[7].setRectRB(0xFF,0x01	,0xFF + 1,0xE5 + 1);
	g_arectConversionUnicodeShiftJISto[8].setRectRB(-1,0,0,0);	//番兵

	g_ShiftJISUnicodeConverter.m_ConversionRule2Way.setShiftJISUnicode();
}

/**
	@brief	初期化。
	@date	2005/08/20	junjunn 作成
*/
bool LanguageFn::init(cpchar1 cszPathShiftJIStoUnicode)
{
	initRule();
	return g_ShiftJISUnicodeConverter.readTable(cszPathShiftJIStoUnicode);
}


//CLanguageCodeConverter///////////////
/**
	@date	2005/08/20	junjunn 作成
*/
bool CLanguageCodeConverter::readTable(cpchar1 cszPath)
{
//すでに変換データを読み込んでいるので何もすることなし。
	if (m_bReady)	return true;

//ファイルを読み込む
	String strSource;
	if (! FileFn::read(cszPath,&strSource))	return false;

	cpchar1 pSource = strSource.getString();

//1→2と2→1の二つを巡回
	for (int iConversionWay=0;iConversionWay<2;iConversionWay++)
	{
		CConversionRule* pConversionRule = &m_ConversionRule2Way.m_aConversionRule[iConversionWay];

//全ての領域を巡回。
		for (int iRuleRect=0;;iRuleRect++)
		{
			Rect* pRect = &pConversionRule->m_parectConversion[iRuleRect];
			if (pRect->getLeft() < 0)	break;		//番兵

//領域を確保
			uint16_t* awBuffer = new uint16_t[pRect->getArea()];
			m_arrayPWordConversionData[iConversionWay].add(awBuffer);

//awTable_UnicodetoShiftJIS_0_224_0_1などのテーブル文字列を探す。
			char szTableName[64];
			getCGenerateConversionCodeTableName(szTableName,pConversionRule,iRuleRect);
			cpchar1 pTableNamePos = StringFn::find(pSource,szTableName);
			if (pTableNamePos==NULL)		continue;
//次の{を探す
			cpchar1 pBracketLeft = StringFn::find(pTableNamePos,'{');
			if (pBracketLeft==NULL)			continue;
//テーブルの最初のエントリー。0xで始まる文字列を探す。
			cpchar1 pTableElementStart = StringFn::find(pBracketLeft,"0x");
			if (pTableElementStart==NULL)	continue;

			cpchar1 p = pTableElementStart;

//後に続く0x〜数字をありったけ読み込む。
			for (int iCount=0;;iCount++)
			{
				if (p[0]!='0' && p[1]!='x')		break;
				int iInt = StringFn::toInt(p+2,16,&p);

				awBuffer[iCount] = (uint16_t)iInt;

				p++;	//','をスキップ
				while (*p=='\n')	p++;	//改行をスキップ。
				while (*p=='\t')	p++;	//タブをスキップ。
				if (*p=='}')	break;		//終わり。
			}
		}
	}

	m_bReady=true;

	return true;
}

/**
	@date	2005/08/20	junjunn 作成
*/
int CLanguageCodeConverter::convert1to2(int iCode1)
{
	return convert1to2((char1)((iCode1 & 0xFF00)>>8),(char1)(iCode1 & 0xFF));
}

/**
	@date	2005/08/20	junjunn 作成
*/
int CLanguageCodeConverter::convert1to2(uint8_t x,uint8_t y)
{
	return convert(x,y,0);
}

/**
	@date	2005/08/20	junjunn 作成
*/
int CLanguageCodeConverter::convert(uint8_t x,uint8_t y,int i1to2)
{
	if (! isReady())	return 0;

	CConversionRule* pConversionRule = &m_ConversionRule2Way.m_aConversionRule[i1to2];

//全ての領域を巡回。
	for (int iRuleRect=0;;iRuleRect++)
	{
		Rect* pRect = &pConversionRule->m_parectConversion[iRuleRect];
		if (pRect->getLeft() < 0)	break;		//番兵
//領域にあるなら
		if (pRect->isPointInside(x,y))
		{
			uint16_t* awConversionTable = m_arrayPWordConversionData[i1to2].getAt(iRuleRect);
			int iReturn = awConversionTable[(y-pRect->getTop()) + pRect->getHeight()*(x - pRect->getLeft())];

			return iReturn;
		}
	}

	return 0;
}

/**
	@date	2005/08/20	junjunn 作成
*/
int CLanguageCodeConverter::convert2to1(int iCode2)
{
	return convert2to1((char1)((iCode2 & 0xFF00)>>8),(char1)(iCode2 & 0xFF));
}

/**
	@date	2005/08/20	junjunn 作成
*/
int CLanguageCodeConverter::convert2to1(uint8_t x,uint8_t y)
{
	return convert(x,y,1);
}

/**
	@date	2005/08/20	junjunn 作成
*/
void CConversionRule2Way::setShiftJISUnicode()
{
	m_aConversionRule[0].set("ShiftJIStoUnicode",g_arectConversionShiftJIStoUnicode);
	m_aConversionRule[1].set("UnicodetoShiftJIS",g_arectConversionUnicodeShiftJISto);
}

/**
	@date	2005/08/20	junjunn 作成
*/
void getCGenerateConversionCodeTableName(pchar1 szOut,CConversionRule* pConversionRule,int iIndex)
{
	Rect* pRect = &pConversionRule->m_parectConversion[iIndex];

	StringFn::format(szOut,
		"awTable_%s_%2X_%2X_%2X_%2X",
		pConversionRule->m_strName.getString(),pRect->getLeft(),pRect->getTop(),pRect->getRight(),pRect->getBottom());
}

}	//namespace monapi2
