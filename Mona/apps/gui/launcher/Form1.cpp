// This file is in the public domain.
// There are no restrictions on any sort of usage of this file.

#include <monapi.h>
#include <gui/System/Mona/Forms/Application.h>
#include <gui/System/Mona/Forms/Button.h>
#include <gui/System/Mona/Forms/Form.h>
#include <gui/System/Collections/ArrayList.h>

using namespace System;
using namespace System::Collections;
using namespace System::Drawing;
using namespace System::Mona::Forms;

extern _P<MonAPI::Screen> GetDefaultScreen();

static int ProcessStart(const String& file)
{
	int len = file.get_Length();
	_A<char> elf(len + 1);
	for (int i = 0; i < len; i++)
	{
		wchar ch = file[i];
		elf[i] = ch < 128 ? ch : '?';
	}
	elf[len] = '\0';
	CommandOption list;
	list.next = NULL;
	char* e = elf.get(), * p = &e[len];
	for (; *p != '/' && e < p; p--);
	p++;
	return syscall_load_process(e, p, &list);
}

class Form1 : public Form
{
private:
	static _P<ArrayList<String> > elfs;
	_A<_P<Button> > buttons;
	
public:
	Form1()
	{
		this->InitializeComponent();
		
		int len = Form1::elfs->get_Count();
		this->buttons.Alloc(len);
		this->set_ClientSize(Size(128, len * 20));
		this->set_Location(Point(::GetDefaultScreen()->getWidth() - this->get_Width(), 0));
		for (int i = 0; i < len; i++)
		{
			this->buttons[i] = new Button();
			this->buttons[i]->set_Bounds(Rectangle(0, i * 20, 128, 20));
			this->buttons[i]->set_Text(Form1::elfs->get_Item(i));
			this->buttons[i]->add_Click(new EventHandler<Form1>(this, &Form1::button_Click));
			this->get_Controls()->Add(this->buttons[i].get());
		}
	}
	
private:
	void InitializeComponent()
	{
		this->set_Text("Mona Launcher");
	}
	
	void button_Click(_P<Object> sender, _P<EventArgs> e)
	{
		_P<Button> button = (Button*)sender.get();
		String file = "/APPS/" + button->get_Text();
		if (file.EndsWith(".APP"))
		{
			file += '/';
			int len = button->get_Text().get_Length();
			file += button->get_Text().Substring(0, len - 4);
			file += ".ELF";
		}
		::ProcessStart(file);
	}
	

public:
	static void Main(_A<String> args)
	{
		if (syscall_cd("/APPS"))
		{
			printf("ERROR: directory /APPS not found\n");
			return;
		}
		if (syscall_dir_open())
		{
			printf("ERROR: can not open directory\n");
			return;
		}
		Form1::elfs = new ArrayList<String>;
		char name[16];
		int size;
		while (syscall_dir_read(name, &size) == 0)
		{
			String n = name;
			if (n.EndsWith(".ELF") || n.EndsWith(".APP")) Form1::elfs->Add(name);
		}
		syscall_dir_close();
		syscall_cd("/");
		
		Application::Run(new Form1());
	}
};
_P<ArrayList<String> > Form1::elfs;

SET_MAIN_CLASS(Form1)
