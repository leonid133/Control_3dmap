//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <vcl.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>


//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
   TButton *Button1;
   TEdit *Edit1;
   TEdit *Edit2;
   TEdit *Edit3;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label3;
   TImage *Image1;
   TButton *Button2;
   TButton *Button6;
   TButton *Button7;
   TSaveDialog *SaveDialog1;
   TOpenDialog *OpenDialog1;
   TStatusBar *StatusBar1;
   TStringGrid *StringGrid1;
   TShape *Shape1;
   TShape *Shape2;
   TShape *Shape3;
   TStaticText *StaticText1;
   TButton *Button11;
   TButton *Button8;
   TCheckBox *CheckBox1;
   TCheckBox *CheckBox2;
   TLabel *Label4;
   void __fastcall Button1Click(TObject *Sender);
   void __fastcall Image1MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
   void __fastcall Button2Click(TObject *Sender);
   void __fastcall Edit1KeyPress(TObject *Sender, char &Key);
   void __fastcall Edit2KeyPress(TObject *Sender, char &Key);
   void __fastcall Edit3KeyPress(TObject *Sender, char &Key);
   void __fastcall FormCanResize(TObject *Sender, int &NewWidth,
          int &NewHeight, bool &Resize);
   void __fastcall Button6Click(TObject *Sender);
   void __fastcall Button7Click(TObject *Sender);
   void __fastcall Button11Click(TObject *Sender);
   void __fastcall Button8Click(TObject *Sender);
   void __fastcall StringGrid1KeyPress(TObject *Sender, char &Key);
   void __fastcall Button6Exit(TObject *Sender);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

private:	// User declarations
public:

   __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
class CashControl
{
   private:

   public:

      ~CashControl() {};
      virtual CreateCasheBMP(double Lat, double Lon, double region);
      virtual CreateCasheTerrain(double Lat, double Lon, double region);


};
Graphics::TBitmap* picture=new Graphics::TBitmap;
int x1=0, y1=0, X=0, Y=0, Xa=0, Ya=0;
bool fEdit = false;

//---------------------------------------------------------------------------
#endif
