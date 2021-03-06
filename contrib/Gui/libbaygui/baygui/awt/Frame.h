/*
Copyright (c) 2005 bayside

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation files 
(the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if !defined(_FRAME_H_INCLUDED_)
#define _FRAME_H_INCLUDED_

namespace baygui {
	/**
	 フレームクラス
	*/
	class Frame : public Window {
	protected:
		/** タイトル */
		String title;
		
	protected:
		/** 閉じるボタンがクリックされたかどうか */
		virtual bool getCloseButtonClicked(int px, int py)
		{
			return (4 <= px && px < 17 && 4 <= py && py < 17) ? true : false;
		}
		
		/** タイトルバーがクリックされたかどうか */
		virtual bool getTitlebarClicked(int px, int py)
		{
			return (0 <= px && px < getWidth() && 0 <= py && py < getInsets()->top);
		}
	
	public:
		/** デフォルトコンストラクタ */
		Frame();
		
		/**
		 * コピーコンストラクタ
		 * @param title タイトル
		*/
		Frame(const String& title);
		
		/** デストラクタ */
		virtual ~Frame();
		
		/** タイトルを得る */
		inline const char* getTitle() { return this->title.getBytes(); }
		
		/**
		 タイトル設定
		 @param title タイトル
		 */
		virtual void setTitle(const String& title);
		
		/** 再描画 */
		virtual void repaint();
	};
}

#endif // _FRAME_H_INCLUDED_
