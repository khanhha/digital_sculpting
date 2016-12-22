#ifndef BASELIB_KARRAY_H
#define BASELIB_KARRAY_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace BaseType{

template <class T> class KArray
{  
private:
   int _size, _increment;
   T *_data;
   int _length;
public:
   KArray(int Inc=1);
   ~KArray();
   T & operator [](int Index) { return _data[Index]; }
   const T & operator [](int Index) const { return _data[Index]; }
   void Clear(int Inc=0);
   void Add(const T &val);
   void AddSize(int count);
   void Insert(int Index, const T &val);
   void Delete(int Index);
   int GetLength() const { return _length; }
   void CopyData(T *Dest);
   bool ReadFile(FILE *hFile);
   bool WriteFile(FILE *hFile);
   bool ReadFile2(FILE *hFile);
   bool WriteFile2(FILE *hFile);
};

//==============================================================================
//==============================================================================
template <class T> KArray<T>::KArray(int Inc)
{  _length=0;
   _size=_increment=Inc;
   _data=(T *)malloc(_size*sizeof(T));
}
//==============================================================================
template <class T> KArray<T>::~KArray()
{  
   if (_data) free(_data);
}
//==============================================================================
template <class T> void KArray<T>::Clear(int Inc)
{  if (_length==0)
      return;
   if (_data) free(_data); 
      _length=0;
   if (Inc > 0)
      _increment = Inc;
   _size=_increment;
   _data=(T *)malloc(_size*sizeof(T));
}
//==============================================================================
template <class T> void KArray<T>::Add(const T &val)
{  if (_length>=_size)
   {  _size+=_increment;
      _data=(T *)realloc(_data, _size*sizeof(T));
   }
   _data[_length]=val;
   _length++;
}
//==============================================================================
template <class T> void KArray<T>::AddSize(int count)
{  if ((_length+count)>=_size) {  
      int c=count/_increment;
      if (count%_increment)
         c++;
      _size+=c*_increment;
      _data=(T *)realloc(_data, _size*sizeof(T));
   }
   _length+=count;
}
//==============================================================================
template <class T> void KArray<T>::Insert(int Index, const T &val)
{  if (_length>=_size)
   {  _size+=_increment;
      _data=(T *)realloc(_data, _size*sizeof(T));
   } 
   for (int i=_length; i>Index; i--)
      _data[i]=_data[i-1];
   _data[Index]=val;
   _length++;
}
//==============================================================================
template <class T> void KArray<T>::CopyData(T *Dest)
{  memcpy(Dest,_data,_length*sizeof(T));
}
//==============================================================================
template <class T> void KArray<T>::Delete(int Index)
{  
   memmove(&_data[Index], &_data[Index+1], (_length-Index-1)*sizeof(T));
   _length--; 
   if (_length>0 && _length<=_size-_increment)
   {  _size-=_increment;
      _data=(T *)realloc(_data, _size*sizeof(T)); 
   }
}
//==============================================================================
template <class T> bool KArray<T>::ReadFile(FILE *hFile)
{  int IntVar;
   fread(&IntVar, sizeof(int), 1, hFile);
   if (_length+IntVar>_size)
   {  _size=_increment*((int)((_length+IntVar)/_increment)+1);
      _data=(T *)realloc(_data, _size*sizeof(T));
   }
   fread(&_data[_length], IntVar*sizeof(T), 1, hFile);
   _length+=IntVar;
   return true;
}
//==============================================================================
template <class T> bool KArray<T>::WriteFile(FILE *hFile)
{  
   fwrite(&_length, sizeof(int), 1, hFile);
   fwrite(_data, _length*sizeof(T), 1, hFile);
   return true;
}
//==============================================================================
template <class T> bool KArray<T>::ReadFile2(FILE *hFile)
{  int IntVar, i;
   fread(&IntVar, sizeof(int), 1, hFile);
   if (_length+IntVar>_size)
   {  _size=_increment*((int)((_length+IntVar)/_increment)+1);
      _data=(T *)realloc(_data, _size*sizeof(T));
   }
   for (i=0;i<IntVar;i++)
      _data[_length+i].ReadFile(hFile);
   _length+=IntVar;
   return true;
}
//==============================================================================
template <class T> bool KArray<T>::WriteFile2(FILE *hFile)
{  
   fwrite(&_length, sizeof(int), 1, hFile);
   for (int i=0;i<_length;i++)
      _data[i].WriteFile(hFile);
   return true;
}
//==============================================================================
//==============================================================================

} // namespace
#endif