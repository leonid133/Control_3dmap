//---------------------------------------------------------------------------

#pragma hdrstop

#include "matrc.h"

//---------------------------------------------------------------------------
//class Vektor****************************************************************
//----------------------------------------------------------------------------
const float ToGrad = 57.2957795130823;

//Перегрузка операции +  ---------------------------------------------------
vektor operator + (const vektor& u,const vektor& v)
{
	return vektor (u.z+v.z, u.x+v.x);
}

//Перегрузка операции -  ---------------------------------------------------
vektor operator - (const vektor& u,const vektor& v)
{
	return vektor (u.z-v.z, u.x-v.x);
}

//Перегрузка операции *. Умножение скаляра на вектор.------------------------
vektor operator* (const vektor& v, float f)
{
	return vektor (f*v.z, f*v.x);
}

//Перегрузка операции *. Умножение скаляра на вектор.------------------------
vektor operator* (float f, const vektor& v)
{
	return vektor (f*v.z, f*v.x);
}

//Деление вектора на скаляр.-------------------------------------------------
vektor operator / (const vektor& v, float f)
{
	return vektor (v.z/f, v.x/f);
}

//Косинус угла между векторами.---------------------------------------------
float cos(const vektor& a, const vektor& b)
{
   float ab = !a*!b;
   if(ab > 0.0000001)
   {
      float k2 = (a*b)/ab;
      if (k2 > 1) k2 = 1;
      if (k2 < -1) k2 = -1;
   	return k2;
   }
   else
      return 0;
}

//----------------------------------------------------------------------------
String vektor:: show(void)
{
  String msg;  msg.sprintf("%6.3f %6.3f ", z, x);  return msg;
}

//атангенс угла между векторами.----------------------------------------------
float atan(const vektor& a, const vektor& b)
{
   return a.alfa() - b.alfa();
}

//class Matric ***************************************************************
//----------------------------------------------------------------------------
template <class TYPE_MATRIC>
matric<TYPE_MATRIC>::matric(const long m_, const long n_)
   : m(m_)
   , n(n_)
{
   if (m == 0 || n == 0)
      ar = NULL;
   else
   {
      ar = new TYPE_MATRIC [m*n];
      for(long i = 0; i < (m*n); i++)
         ar[i] = 0;
   }
};

//----------------------------------------------------------------------------
template <class TYPE_MATRIC> matric<TYPE_MATRIC>::matric(const matric &r)
{
   m = r.m;
   n = r.n;
   ar = new TYPE_MATRIC [m*n];

   for (long j = 0; j < m*n; j++)
      ar[j] = r.ar[j];
};

//-----------------------------------------------------------------------------
template <class TYPE_MATRIC>
const matric<TYPE_MATRIC> &matric<TYPE_MATRIC>::operator=(const matric &r)
{
   if(this != &r)
   {
      delete [] ar;
      m = r.m;
      n = r.n;
      ar = new TYPE_MATRIC [m*n];

      for (long j = 0; j < m*n; j++)
         ar[j] =  r.ar[j];
   }
   return *this;
};

//Сложение--------------------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC>
matric<TYPE_MATRIC> matric<TYPE_MATRIC>::operator+(const matric &r)
{
   matric temp(m, n);

   for (long j = 0; j < m*n; j++)
      temp.ar[j] =  ar[j]+r.ar[j];
   return temp;
};

//Вычитание ------------------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC>
matric<TYPE_MATRIC> matric<TYPE_MATRIC>::operator-(const matric &r)
{
   matric temp(m, n);
   for (long j = 0; j < m*n; j++)
      temp.ar[j] =  ar[j]-r.ar[j];

   return temp;
};

//Умножение-------------------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC>
matric<TYPE_MATRIC> matric<TYPE_MATRIC>::operator*(const matric &r)
{
   matric temp(m, n);
   for (long j = 0; j < m; j++)
      for (long k = 0; k < n; k++)
      {
         temp.ar[j*n+k] = 0;
         for (long i = 0; i < n; i++)
            temp.ar[j*n+k] += ar[j*n+i] * r.ar[i*n+k];
      }
   return temp;
};

//Умножение на скаляр----------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC>
matric<TYPE_MATRIC> matric<TYPE_MATRIC>::operator*(const TYPE_MATRIC x)
{
   matric temp(m, n);
   for (int j = 0; j < m*n; j++)
      temp.ar[j] =  ar[j]*x;

   return temp;
};

//Определитель----------------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC>
TYPE_MATRIC  matric<TYPE_MATRIC>::D(void)
{
   TYPE_MATRIC temp = 1, temp2 = -1, sum = 0;
   long i, i2;
   for(long kk = 1; kk < n+1; kk++)
   {
      i = kk - 1 ;
      i2 = n - kk;
      for (long j = 0; j < m; j++)
      {
         if (i >= n){i = 0;}
         temp = temp * ar[j*n+i++];
         if (i2 < 0){i2 = n - 1;}
         temp2 = temp2 * ar[j*n+i2--];
      }
      sum += temp + temp2;
      temp = 1;
      temp2 = -1;
   }
   return sum;
};

//Транспонирование матриц-----------------------------------------------------
/*virtual*/template <class TYPE_MATRIC> matric<TYPE_MATRIC> matric<TYPE_MATRIC>::T(void)
{
   matric temp(n, m);
   for(long j = 0; j < n; j++)
      for(long k = 0; k < m; k++)
         temp.ar[j*n+k] = ar[k*m+j];

   return temp;
};

//След матрицы ---------------------------------------------------------------
/*virtual*/template <class TYPE_MATRIC> TYPE_MATRIC matric<TYPE_MATRIC>::Sp(void)
{
    TYPE_MATRIC Spur=0;
    long x=m<n?m:n;
    for(long i=0; i<x; i++)
    Spur+=ar[i*m+i];
    return Spur;
};
//Вывод-----------------------------------------------------------------------
String matric<float>::Show(void)
{
   String tmp(""), txt;
   for (long j = 0; j < m; j++)
      for (long k = 0; k < n; k++)
      {
         tmp = tmp.FormatFloat("0.00     ", ar[j*n+k]);
         txt += tmp;
      }
   return txt;
};
//Вывод-----------------------------------------------------------------------
String matric<int>::Show(void)
{
   String tmp(""), txt;
   for (long j = 0; j < m; j++)
      for (long k = 0; k < n; k++)
      {
         tmp = ar[j*n+k];
         txt += tmp;
      }
   return txt;
};
//Фильтр Гаусса---------------------------------------------------------------
matric<float> FilterGauss (float sigma, int n, float factorY) // sigma(0.5..1.5) n(1..3) factorY(1..2.5)
{
   matric<float> FilterGauss(n, n);
   for (int j = 0; j < n; j++)
     for (int k = 0; k < n; k++)
        FilterGauss.ar[j*n+k] = factorY * (exp((-pow(k, 2)-pow(j, 2))/(2*pow(sigma, 2)))/(2 * M_PI * pow(sigma, 2)));
   return FilterGauss;
};

//Class Image*****************************************************************
//----------------------------------------------------------------------------
const image &image::operator=(const image &img)
{
   if(this != &img)
   {
      m = img.m;
      n = img.n;
      dm = img.dm;
      dn = img.dn;

      R = img.R;
      G = img.G;
      B = img.B;
      Y = img.Y;
      Sx = img.Sx;
      Sy = img.Sy;
      Sxy = img.Sxy;
      Lnorm = img.Lnorm;
      H = img.H;
   }
   return *this;
};

//Инициализация из потока-----------------------------------------------------
/*virtual*/image image::LoadFromFile(ifstream &file)
{
   dm=dn=0;
   unsigned char buf[1023];
   file.read(buf, 54);
   long int location = ((long int)(buf[10]))+((long int)(buf[11]) << 8)+((long int)(buf[12]) << 16)+((long int) (buf[13]) << 24);
   int n = ((long int)(buf[18])) + ((long int)(buf[19]) << 8) + ((long int)(buf[20]) << 16) + ((long int)(buf[21]) << 24);
   int m = ((long int)(buf[22]))+((long int)(buf[23]) << 8)+((long int)(buf[24]) << 16)+((long int) (buf[25]) << 24);


   long int bytes_bitmap_data = ((long int)(buf[34]))+((long int)(buf[35]) << 8)+((long int)(buf[36]) << 16)+((long int) (buf[37]) << 24);
   file.ignore((location - 54));
   long int j = (m - 1), k = 0, i = 0, mem_i = 0, s;
   B = G = R = Y = Lnorm = Sx = Sy = Sxy = H = matric<float>(m, n);   
   while (mem_i <= bytes_bitmap_data)
   {
      file.read(buf, 3);
      while ((j >= 0) && (i <= 2))
      {
         B.ar[j*B.n+k] = buf[i++];
         G.ar[j*G.n+k] = buf[i++];
         R.ar[j*R.n+k++] = buf[i++];
         if (k >= n)
         {
            k = 0;
            j--;
         }
      }
      mem_i+=i;
      i = 0;
      if (file.eof())
         break;
   }
   file.close();
   Y =(B*0.11 + G*0.59 + R*0.3);
   return *this;
};

//Сохранение в поток ----------------------------------------------------------
/*virtual*/image image::SaveFromFile(ofstream &file)
{
   BYTE  buf;
   for(int i=1; i<1024; i++)
   {
      for(int j=1; j<1024; j++)
      {
         buf = Y.ar[(int)(i*m/1024)*n + (int)(j*n/1024)];
         file << buf;
      }
      file << "\t";
   }
   return *this;
};

//Свертка---------------------------------------------------------------------
/*virtual*/image image :: Convolution(const matric<float> &Filter)
{
   matric<float> A(Filter.m, Filter.n);
   matric<float> temp(Y);

   for (int jm = Filter.m; jm < Y.m; jm++)
   {
      for (int kn = Filter.n; kn < Y.n; kn++)
      {
         for (int j = 0; j < Filter.m; j++)
            for (int k = 0; k < Filter.n; k++)
               A.ar[j*Filter.n+k] = temp.ar[(jm - Filter.m + j)*Y.n+(kn - Filter.n + k)];
//Умножение A = A * Filter и свертка в temp
         for (int jj = 0; jj < A.m; jj++)
            for (int kk = 0; kk < A.n; kk++)
            {
               int l = (jm - Filter.m + jj)*Y.n + (kn - Filter.n + kk);
               temp.ar[l] = 0;
               for (int i = 0; i < A.n; i++)
                  temp.ar[l] += A.ar[jj*A.n+i] * Filter.ar[i*Filter.n+kk];
            }
//-------------------------------------
      }
   }
   Y = temp;
   return *this;
};

//Эквализация-----------------------------------------------------------------
/*virtual*/ image  image :: Eqalization(void)
{
   float Ymax=0, Ymin=255, Rmax=0, Rmin=255, Gmax=0, Gmin=255, Bmax=0, Bmin=255;
   m = Y.m;
   n = Y.n;
   for (int j = 0; j < m; j++)
      for (int k = 0; k < n; k++)
      {
         if(Y.ar[j*n+k]>Ymax){Ymax=Y.ar[j*n+k];}
         if(Y.ar[j*n+k]<Ymin){Ymin=Y.ar[j*n+k];}
         if(R.ar[j*n+k]>Rmax){Rmax=R.ar[j*n+k];}
         if(R.ar[j*n+k]<Rmin){Rmin=R.ar[j*n+k];}
         if(G.ar[j*n+k]>Gmax){Gmax=G.ar[j*n+k];}
         if(G.ar[j*n+k]<Gmin){Gmin=G.ar[j*n+k];}
         if(B.ar[j*n+k]>Bmax){Bmax=B.ar[j*n+k];}
         if(B.ar[j*n+k]<Bmin){Bmin=B.ar[j*n+k];}
      }
   for (int j = 0; j < m; j++)
      for (int k = 0; k < n; k++)
      {
         Y.ar[j*n+k] = 255*(Y.ar[j*n+k]-Ymin)/(Ymax-Ymin);
         R.ar[j*n+k] = 255*(R.ar[j*n+k]-Rmin)/(Rmax-Rmin);
         G.ar[j*n+k] = 255*(G.ar[j*n+k]-Gmin)/(Gmax-Gmin);
         B.ar[j*n+k] = 255*(B.ar[j*n+k]-Bmin)/(Bmax-Bmin);
      }
   return *this;
}


//Тестовый вывод--------------------------------------------------------------
void image::Draw(void)
{
   m = R.m;
   n = R.n;
   for (int j = 0; j < m; j++)
      for (int k = 0; k < n; k++)
        Form1->Image1->Canvas->Pixels[k][j] = (int)R.ar[j*n+k] + ((int)(G.ar[j*n+k]) << 8) + ((int)(B.ar[j*n+k]) << 16);
};

void image::DrawY(void)
{
   m = Y.m;
   n = Y.n;
   for (int j = 0; j < m; j++)
      for (int k = 0; k < n; k++)
        Form1->Image1->Canvas->Pixels[k][j] = (int)Y.ar[j*n+k] + ((int)(Y.ar[j*n+k]) << 8) + ((int)(Y.ar[j*n+k]) << 16);
};

#pragma package(smart_init)
