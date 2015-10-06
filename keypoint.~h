//#ifndef keypointH
 //  #define keypointH
#include <cstring.h>
#include <sysdefs.h>
#include <fstream.h>
#include <math.h>
#include <vcl.h>

#define Rz  6380.
extern const float ToGrad;// 57.2957795130823

bool GetPos (fstream &file, string Caption);
/*  земная система координат
   z - ось абцисс, x - ось ординат   */

//--------------------------------------------------------------------------
class vektor3{
public:
	double x,y,z;

	vektor3 (double vx = 0, double vy = 0, double vz = 0): x(vx), y(vy), z(vz) { };
	vektor3 (const vektor3& v): x(v.x), y(v.y), z(v.z) { };
	vektor3& operator = (const vektor3& v)
		{ x = v.x; y = v.y; z = v.z; return *this;};
	vektor3& operator = (double f)	{ x = y = z = f; return *this;};
	vektor3  operator - () const	{ return vektor3(-x,-y,-z);};
	vektor3& operator += (const vektor3& v)
		{x += v.x; y += v.y; z += v.z; return *this;};
	vektor3& operator -= (const vektor3& v)
		{x -= v.x; y -= v.y; z -= v.z; return *this;};
	vektor3& operator *= (const vektor3& v);
	vektor3& operator *= ( double v)
		{	x *= v;	y *= v; z *= v; return *this;};
	vektor3& operator /= ( double v)
		{ x /= v; y /= v; z /= v; return *this;};
	friend vektor3 operator + (const vektor3 &u, const vektor3 &v);//Сложение векторов
	friend vektor3 operator - (const vektor3 &u, const vektor3 &v);//Вычитание векторов
	friend vektor3 operator * (const vektor3 &u, const vektor3&v)//Векторное прои-ние.
   {
   return vektor3(u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z,u.x*v.y-u.y*v.x);
   };
	friend double operator  ^ (const vektor3& u, const vektor3& v)
			  { return u.x*v.x+u.y*v.y+u.z*v.z; };//Скалярное произведение.
	friend vektor3 operator * (const vektor3 &v, double f)//Умножение скаляра на
   {     return vektor3 (f*v.x, f*v.y, f*v.z);   };    //    вектор.
	friend vektor3 operator * (double f, const vektor3 &v)
   {     return vektor3 (f*v.x, f*v.y, f*v.z);   };
	friend vektor3 operator / (const vektor3 &v, double f)//Деление вектора на скаляр.
   {  return vektor3 (v.x/f, v.y/f, v.z/f);   };
	double operator ! ()const { return (double)sqrt(x*x+y*y+z*z); }//модуль
	friend double mod(const vektor3 & v) { return v.x*v.x+v.y*v.y+v.z*v.z; }//квадрат модуля
	friend double cos(const vektor3 &u, const vektor3 &v)
   {
      std::numeric_limits <double> ep;
      double z = (!u)*(!v);
      if (fabs(z) < ep.epsilon())
      {
         if (z > 0) z = ep.epsilon();
         else       z = -ep.epsilon();
      }
      double arg = (u^v)/z;
      if (arg > 1)
         arg = 1;
      else if (arg < -1)
         arg = -1;
      return arg;
   };
	friend double acos(const vektor3 &u, const vektor3 &v)
   {
      return  acos(cos(u, v));
   };
	void show(void);
};

//Сложение векторов----------------------------------------------------------
inline vektor3 operator + (const vektor3 &u,const vektor3 &v)
{
	return vektor3 (u.x+v.x, u.y+v.y, u.z+v.z);
}

//Вычитание векторов---------------------------------------------------------
inline vektor3 operator - (const vektor3 &u,const vektor3 &v)
{
	return vektor3 (u.x-v.x, u.y-v.y, u.z-v.z);
}

//Точка на карте, [рад]********************************************************
class KPoint
{
public:
   float Lat, Lon; //широта (Latitude), долгота (Longitude) в радианах

   KPoint(float Lt = 0, float Ln = 0):Lat(Lt), Lon(Ln) {};
      //Преобразование км прям-ных коор-т в географ. коор-ты в радианах
   KPoint(float dz, float dx, int Lat0)
   {
      Lat = atan(dx/Rz);
      Lon = atan(dz/Rz/cos(Lat0/ToGrad));
   };
   KPoint(const vektor3 &v, int Lat0)
   {
      Lat = atan(v.x/Rz);
      Lon = atan(v.z/Rz/cos(Lat0/ToGrad));
   };
   vektor3 ToRect(int Lat0) //Преобразование разности географических координат
   {                            // в км прямоугольных
      return vektor3(Rz*tan(Lat), 0, Rz*tan(Lon)*cos(Lat0/ToGrad));
   }

	KPoint (const KPoint& v): Lat(v.Lat), Lon(v.Lon) {};
	KPoint& operator = (const KPoint& v)
      { Lat = v.Lat; Lon = v.Lon; return *this;};

	KPoint  operator - () const	{ return KPoint(-Lat, -Lon);};
	friend KPoint operator + (const KPoint& u, const KPoint& v);
	friend KPoint operator - (const KPoint&, const KPoint&);

	float operator ! ()const     //модуль  (длина)
   {  return (float)Rz*sqrt(Lat*Lat+Lon*Lon);   }
   float dir(int Lat0)
   {
      float x = tan(Lat);
      float z = tan(Lon)*cos(Lat0/ToGrad);
      if ((fabs(x) > FLT_EPSILON) || (fabs(z) > FLT_EPSILON))
         return atan2(z, x);
      else
         return 0;

/*      std::numeric_limits <float> ep;
      if (fabs(x) < ep.epsilon())
      {
         if (z > 0)   return M_PI_2;
         else         return -M_PI_2;
      }
      else
         return atan(z/x);*/
   };
   //Логические операции
	friend bool operator == (const KPoint& u, const KPoint& v)
   {
      std::numeric_limits <float> ep;
      if(fabs(u.Lat-v.Lat) > 5.*ep.epsilon()) return false;
      if(fabs(u.Lon-v.Lon) > 5.*ep.epsilon()) return false;
      return true;
   };

   //Вывод в файл  - Инициализация из файла
   friend fstream &operator<<(fstream &file, KPoint &obj);
   friend fstream &operator >>(fstream &file,KPoint &obj);
   friend bool FromString(String src, KPoint &a);

   String show(void);
   String showLat(void);
   String showLon(void);
};


//#endif

