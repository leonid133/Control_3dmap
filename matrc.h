//---------------------------------------------------------------------------

#ifndef matrcH
#define matrcH

//#define TYPE_MATRIC double
#include <math.h>
#include <fstream.h>
#include <iostream.h>
#include <stdlib.h>
#include <vcl.h>

#include "Unit1.h"
 //---------------------------------------------------------------------------
class vektor{
public:
   float z, x ;                //z - ось абцисс, x - ось ординат
	vektor (const vektor& v): z (v.z), x (v.x)  {};
	vektor (float vz = 0 ,float vx = 0 ): z(vz), x(vx){};

	vektor& operator = (const vektor& v) {z = v.z; x = v.x;  return *this;};
   void FromPolar(float R, float alfa_r)
   {
      z = R*sin(alfa_r);
      x = R*cos(alfa_r);
   };

	vektor  operator - () const	{ return vektor(-z,-x);};
	friend vektor operator + (const vektor& u, const vektor& v);//Сложение векторов
	friend vektor operator - (const vektor&, const vektor&);//Вычитание векторов

	friend float operator  * (const vektor& u, const vektor& v)
			  { return u.z*v.z+u.x*v.x; };//Скалярное произведение.
	friend vektor operator * (const vektor&, float t);//Умножение скаляра на
	friend vektor operator * (float c, const vektor&);//    вектор.
	friend vektor operator / (const vektor&, float t);//Деление вектора на скаляр.

	float operator ! ()const { return (float)sqrt(z*z+x*x); }//модуль  (длина)
   float alfa(void)const
   {
      float alfa;
      if (fabs(x) < 1e-5)
      {
         if (z > 0)   alfa = M_PI/2;
         else         alfa = -M_PI/2;
      }
      else
         alfa = atan2(z, x);
      return alfa;
   };
	friend float cos(const vektor&, const vektor&);
	friend float acos(const vektor& v1, const vektor& v2)
      {
         double res = (double)cos(v1, v2);
         return (acos(res));
      };
	friend float atan(const vektor& v1, const vektor& v2);
	String show(void);
};

//----------------------------------------------------------------------------
template <class TYPE_MATRIC>
class matric{
public:
   TYPE_MATRIC *ar;
   long m, n;
   ~matric(){delete[] ar;}
   matric(const long m = 0, const long n = 0);
   matric(const matric &r);
   const matric &operator=(const matric &r);

   virtual matric operator+(const matric &r);
   virtual matric operator-(const matric &r);
   virtual matric operator*(const matric &r);
   virtual matric operator*(const TYPE_MATRIC x);
   virtual TYPE_MATRIC D(void);  // Детерминант
   virtual matric T(void);       // Транспонирование
   virtual TYPE_MATRIC Sp(void); // След матрицы 
   String matric::Show(void);
};

//Создание фильтра Гаусса-----------------------------------------------------
matric<float> FilterGauss (float sigma, int n, float factorY);

//----------------------------------------------------------------------------
class image{
   long m, n;
public:

   matric<float> B, G, R, Y, Lnorm, Sx, Sy, Sxy, H;
   ~image(void){};
   image(void): m(0){};
   image(const fstream &file);
   const image &operator=(const image &img);

   virtual image LoadFromFile(ifstream &file);
   virtual image SaveFromFile(ofstream &file);
   virtual image Convolution(const matric<float> &Filter); //Свертка
   virtual image Eqalization(void); //Растягивание гиcтограммы на весь диапазон

   void Draw(void);
   void DrawY(void);
   int dm, dn;
};

#endif
