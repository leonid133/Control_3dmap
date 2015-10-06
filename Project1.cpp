
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("Unit1.cpp", Form1);
USEFORM("draw.cpp", OpenGL_Form);
//---------------------------------------------------------------------------
/*
const char *NamedMutex= "OneOnly";
   HANDLE CheckInstance(const char *Name)
   {
      HANDLE Mutex = CreateMutex(NULL, true, Name);
      int er = GetLastError();
      if (er) return 0;
      return Mutex;
   }
          */
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   try
   {
       Application->Initialize();
       Application->CreateForm(__classid(TForm1), &Form1);
       Application->Run();
   }
   catch (Exception &exception)
   {
       Application->ShowException(&exception);
   }
   catch (...)
   {
       try
       {
          throw Exception("");
       }
       catch (Exception &exception)
       {
          Application->ShowException(&exception);
       }
   }
   return 0;
}
//---------------------------------------------------------------------------











