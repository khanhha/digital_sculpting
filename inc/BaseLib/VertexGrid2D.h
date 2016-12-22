#ifndef VERTEXGRID2D_H
#define VERTEXGRID2D_H
#pragma once
#include <vector>

template<class T>
class VertexContainer
{
   std::vector<T*> _elements;
public:
   VertexContainer(void){
   };
   ~VertexContainer(void){
      _elements.clear();
   };
   size_t getNumberOfElement(){
      return _elements.size();
   }
   int addElement(T* element){
      _elements.push_back(element);
      return _elements.size();
   };
   
   T* getElement(const int i){
      if(i < 0 || i >= _elements.size()){
         return 0;
      }
      return _elements.at(i);
   };

   std::vector<T*>* getElements(){
      return &_elements;
   };

};
template<class T>
class VertexGrid2D
{
   VertexContainer<T>***   _grid;
   int    _dimension[2];
   double _origin[2];
   double _size[2];
   double _resol[2];
   
public:
   VertexGrid2D(void)
   {
      _grid = 0;
      _dimension[0] = _dimension[1] = 0;
      _origin[0] = origin[1] = 0.0;
      _size[0] = size[1] = 0.0;
      _resol[0] = resol[1] = 0.0;
   };
   ~VertexGrid2D(void){
      if(_grid){
         for(int i = 0; i < _dimension[0]; ++i){
            for(int j = 0; j < _dimension[1]; ++j){
               delete _grid[i][j];
            }
            free((void*)_grid[i]);
         }
         free((void*)_grid);
         _grid = 0;
      }
   }

   int setOrigin(double origin[2]){
      _origin[0] = origin[0];
      _origin[1] = origin[1];
      return 0;
   };

   int setSize(double size[2]){
      _size[0] = size[0];
      _size[1] = size[1];
      return 0;
   };

   int createDimension(const int n)
   {
      int n0 = 5;
      double scale 0.0;
      if(_size[0] > _size[1]){
         scale = _size[0]/_size[1];
      }
      else{
         scale = _size[1]/_size[0];
      }
      scale *= (double)n0;
      double del = sqrt((double)n/scale);
      _dimension[0] = _size[0]/del;
      _dimension[1] = _size[1]/del;
      _resol[0] = _size[0]/((double)_dimension[0]);
      _resol[1] = _size[1]/((double)_dimension[1]);
      _grid = (VertexContainer<T>***)malloc(sizeof(VertexContainer<T>**)*_dimension[0]);
      for(int i = 0; i < _dimension[0]; ++i){
         _grid[i] = (VertexContainer<T>**)malloc(sizeof(VertexContainer<T>*)*_dimension[1]);
         for(int j = 0; j < _dimension[1]; ++j){
            _grid[i][j] = 0;
         }
      }
      return 0;
   };

   int addElement(T* element, double* coord){
      assert(_grid);

      int i = (coord[0] - _origin[0])/_resol[0];
      int j = (coord[1] - _origin[1])/_resol[1];
      i++; j++;
      assert(i < _dimension[0]);
      assert(j < _dimension[1]);

      if(0 == _grid[i][j]){
         _grid[i][j] = new VertexContainer<T>();
      }
      return _grid[i][j]->addElement(element);
      
   };
};

#endif
