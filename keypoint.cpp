#include "keypoint.h"

const float ToGrad = 57.2957795130823;

//----------------------------------------------------------------------------
bool GetPos (fstream &file, string Caption)
{
   string temp;
   file.seekg(0);
   while (!file.eof())
   {
      file >> temp;
      if(temp == Caption)
      {
         file >> temp;
         return true;
      }
   }
   MessageBox(NULL,"������ �� ������", Caption.c_str(), MB_OK);
   return false;
};

//���������� �������� +  ---------------------------------------------------

KPoint operator + (const KPoint& u,const KPoint& v)
{
	return KPoint (u.Lat+v.Lat, u.Lon+v.Lon);
}

//���������� �������� -  ---------------------------------------------------

KPoint operator - (const KPoint& u,const KPoint& v)
{
	return KPoint (u.Lat-v.Lat, u.Lon-v.Lon);
}

//����� ������ �����: <������> ��.� ' , <�������> ��.� '-------------------
String KPoint:: show(void)
{
   return showLat() + ", " + showLon();
}

//����� ������ �����: <������> ��.� '---------------------------------------
String KPoint:: showLat(void)
{
   float Minutes;
   String returnString, bufferMinutes;

   long Gradus = Lat*ToGrad;        //����� ����� �������� � �������
   if (Lat >= 0)                          // ���� ���� � ��� ������������
   {
      Minutes = (Lat*ToGrad-Gradus)*60;   //���� ������� ����� * �� 60 = ������
      returnString = IntToStr(Gradus)+"� ";   //���������� �������
   }
   else if (Gradus > -1)           // ���� ����� ���� � �������� ����� � ����� � 0
   {                                    // �� ����� �������� ��� � ��- ������
      Minutes = -Lat*ToGrad*60;
      returnString = "-0� ";
   }
   else
   {
      Minutes = (Gradus-Lat*ToGrad)*60;
      returnString = IntToStr(Gradus)+"� ";
   }
   bufferMinutes.sprintf("%6.4f'", Minutes);
   return returnString + bufferMinutes;
}

//����� ������ �����: <�������> ��.� '--------------------------------------
String KPoint:: showLon(void)
{
   float Minutes;
   String returnString, bufferMinutes;

   if (Lon < 0) Lon = Lon +2*M_PI;
   if (Lon > 2*M_PI) Lon = Lon - 2*M_PI;

   float copyLon = Lon;
   if(copyLon > M_PI )  copyLon = copyLon - 2* M_PI;
   long Gradus = copyLon*ToGrad;
   if (copyLon >= 0)
   {
      Minutes = (copyLon*ToGrad-Gradus)*60;
      returnString = IntToStr(Gradus)+"� ";
   }
   else if (Gradus > -1)
   {
      Minutes = -copyLon*ToGrad*60;
      returnString = "-0� ";
   }
   else
   {
      Minutes = (Gradus-copyLon*ToGrad)*60;
      returnString = IntToStr(Gradus)+"� ";
   }
   bufferMinutes.sprintf("%6.4f'", Minutes);
   return returnString + bufferMinutes;
}

//-- ����� � ���� --------------------------------------------------------------
fstream &operator<<(fstream &file, KPoint &obj)
{
   String temp;
   temp = obj.show();
   file << temp.c_str();
   return file;
}

//--- ������������� �� �����-------------------------------------------------------
fstream &operator >>(fstream &file, KPoint &obj)
{
   bool minus = false;
   float Lat,Lon;
   string tmp;

   file >> tmp;
   tmp.erase(tmp.length()-1,1);  //�������� �����������
   Lat = atoi(tmp.c_str())/ToGrad;
   if(tmp.c_str()[0] == '-')             //����� �.�. -0 == 0
   {
      minus = true;
      Lat = -Lat;
   }

   file >> tmp;
   tmp.erase(tmp.length()-2,2);
   obj.Lat = Lat + atof(tmp.c_str())/ToGrad/60;
   if(minus) obj.Lat = -(obj.Lat);
   minus = false;

   file >> tmp;
   tmp.erase(tmp.length()-1,1);
   Lon = atoi(tmp.c_str())/ToGrad;
   if(tmp.c_str()[0]== '-')             //����� �.�. -0 == 0
   {
      minus = true;
      Lon = -Lon;
   }

   file>>tmp;
   tmp.erase(tmp.length()-1,1);
   obj.Lon = Lon + atof(tmp.c_str())/ToGrad/60;
   if(minus ) obj.Lon = 2*M_PI - obj.Lon;
   minus = false;

   return file;
}

//----------------------------------------------------------------------------
bool FromString(String src, KPoint &a)
{
   char *s = src.c_str(), tmp[10];
   for(int i = 0; i < 10; i++)
   {
      if (*s == '�')
      {
         tmp[i] = '\n';
         s++;
         int lat = atoi(tmp);
         if (lat > 360)
         {
            Application->MessageBox("������ > 90�", "������");
            return false;
         }
         if (lat < 0)
         {
            Application->MessageBox("������ < -90�", "������");
            return false;
         }

         a.Lat = (float)lat/ToGrad;
         break;
      }
      tmp[i] = *s;
      s++;
   }
   for(int i = 0; i < 10; i++)
   {
      if (*s == '\'')
      {
         tmp[i] = '\n';
         s++;
         float lat = atof(tmp);
         if (lat > 60)
         {
            Application->MessageBox("������ ������ > 60�", "������");
            return false;
         }
         if (lat < 0)
         {
            Application->MessageBox("������ ������ < 0�", "������");
            return false;
         }

         a.Lat = a.Lat+lat/60./ToGrad;
         break;
      }
      tmp[i] = *s;
      s++;
   }
   s++;
   for(int i = 0; i < 10; i++)
   {
      if (*s == '�')
      {
         tmp[i] = '\n';
         s++;
         int lon = atoi(tmp);
         if (lon > 360)
         {
            Application->MessageBox("������� > 180�", "������");
            return false;
         }
         if (lon < 0)
         {
            Application->MessageBox("������� < -180�", "������");
            return false;
         }

         a.Lon = (float)lon/ToGrad;
         break;
      }
      tmp[i] = *s;
      s++;
   }
   for(int i = 0; i < 10; i++)
   {
      if (*s == '\'')
      {
         tmp[i] = '\n';
         float lon = atof(tmp);
         if (lon > 60)
         {
            Application->MessageBox("������ ������� > 60�", "������");
            return false;
         }
         if (lon < 0)
         {
            Application->MessageBox("������ ������� < 0�", "������");
            return false;
         }

         a.Lon = a.Lon+lon/60./ToGrad;
         break;
      }
      tmp[i] = *s;
      s++;
   }
   return true;
}


