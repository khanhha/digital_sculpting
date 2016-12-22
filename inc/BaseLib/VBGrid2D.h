#ifndef BASELIB_WPGRID2D_H
#define BASELIB_WPGRID2D_H
#include "util.h"

namespace BaseType{

    template<class T>
    class VBGridCell{
        unsigned short _size;
        unsigned short _nElem;
        T **_elements;

    public:
        VBGridCell() :
            _size(0),
            _nElem(0),
            _elements(nullptr)
        {

        }
        ~VBGridCell(){
            if (_elements){
                /*for (unsigned i = 0; i < _nElem; ++i){
                    delete _elements[i];
                }*/
                Util::k_free((void*)_elements);
            }
        }

        void addElement(T* element){
            if (nullptr == _elements){
                _elements = (T**)Util::k_malloc(sizeof(T*) * 4);
                _nElem = 0;
                _size = 0;
            }
            else if (_size <= _nElem){
                _size += 4;
                _elements = (T **)Util::k_realloc((void*)_elements, sizeof(T*)*_size);
            }
            _elements[_nElem] = element;
            _nElem++;
        };

        T* getElement(const unsigned i){
            if (i < 0 || i >= _size){
                return 0;
            }
            return _elements[i];
        };

        T** getElements(){
            return _elements;
        };

        T* findLeftMostElement(){
            T *ret = nullptr;
            float *coord;
            float mincoord = float(HUGE_VAL);
            for (unsigned i = 0; i < _nElem; ++i){
                if (_elements[i]->getId() >= 0){
                    coord = _elements[i]->getCoord();
                    if (coord[0] < mincoord){
                        ret = _elements[i];
                        mincoord = coord[0];
                    }
                }
            }
            return ret;
        }

        T* getElementInsideTriangleNeedFar(float *v0, float *dv1, float *dv2, float *nv, float &dis){
            float *coord,dv[2];
            T *ret = nullptr;
            float d,d1, d2;
            for (unsigned i = 0; i < _nElem; ++i){
                if (_elements[i]->getId() >= 0){
                    coord = _elements[i]->getCoord();
                    if (coord != v0){
                        dv[0] = coord[0] - v0[0];
                        dv[1] = coord[1] - v0[1];
                        d = dv[0] * nv[0] + dv[1] * nv[1];
                        if (d < 0.0f && d > dis){
                            d1 = dv[0] * dv1[1] - dv[1] * dv1[0];
                            d2 = dv[0] * dv2[1] - dv[1] * dv2[0];
                            if (d1*d2 < -EPSILON_VAL_){
                                dis = d;
                                ret = _elements[i];
                            }
                        }
                    }
                }
            }
            return ret;
        }
    };




    template<class T>
    class VBGrid2D
    {
        VBGridCell<T> ***_grid;
        unsigned  _dimension[2];
        float _origin[2];
        float _size[2];
        float _step;

    public:
        VBGrid2D() :
            _grid(nullptr),
            _step(1.0){
            _dimension[0] = _dimension[1] = 1;
            _origin[0] = _origin[1] = 0.0f;
            _size[0] = _size[1] = 0.0f;

        }
        ~VBGrid2D()
        {
            if (_grid){
                for (size_t i0 = 0; i0 < _dimension[0]; ++i0){
					for (size_t i1 = 0; i1 < _dimension[1]; ++i1){
                        delete _grid[i0][i1];
                    }
                    Util::k_free((void*)_grid[i0]);
                }
                Util::k_free((void*)_grid);
                _grid = 0;
            }
        }

        void setOrigin(float origin[2]){
            _origin[0] = origin[0];
            _origin[1] = origin[1];
        };

        void setSize(float size[2]){
            _size[0] = size[0];
            _size[1] = size[1];
        };

        void setStepSize(float step){
            //assert(step > EPSILON_VAL_E3);
            _step = step;
        }

        void init()
        {
            _dimension[0] = static_cast<unsigned>(_size[0] / _step) + 1;
            _dimension[1] = static_cast<unsigned>(_size[1] / _step) + 1;
            _size[0] = _step*static_cast<float>(_dimension[0]);
            _size[1] = _step*static_cast<float>(_dimension[1]);

            _grid = (VBGridCell<T>***)Util::k_malloc(sizeof(VBGridCell<T>**)*_dimension[0]);
			for (size_t i0 = 0; i0 < _dimension[0]; ++i0){
                _grid[i0] = (VBGridCell<T>**)Util::k_malloc(sizeof(VBGridCell<T>**)*_dimension[1]);
                
				for (size_t i1 = 0; i1 < _dimension[1]; ++i1){
                    _grid[i0][i1] = nullptr;

                }
            }
        }

        void addElement(T *element, unsigned i, unsigned j){
            assert(_grid);

            if (i >= 0 && j >= 0 && i < _dimension[0] && j < _dimension[1]){

                if (0 == _grid[i][j]){
                    _grid[i][j] = new VBGridCell<T>();
                }
                _grid[i][j]->addElement(element);
            }
            else{
                assert(!"Index is out of range!");
            }
        }

        void addElement(T* element, float* coord){
            assert(_grid);
			size_t i0 = static_cast<unsigned>((coord[0] - _origin[0]) / _step);
			size_t i1 = static_cast<unsigned>((coord[1] - _origin[1]) / _step);

            assert(i0 >= 0);
            assert(i1 >= 0);
            assert(i0 < _dimension[0]);
            assert(i1 < _dimension[1]);

            if (0 == _grid[i0][i1]){
                _grid[i0][i1] = new VBGridCell<T>();
            }
            _grid[i0][i1]->addElement(element);
        };

       

        void addElement(T* element, float *sp, float *ep){
            assert(_grid);
            int i0 = static_cast<unsigned>((sp[0] - _origin[0]) / _step);
            int j0 = static_cast<unsigned>((sp[1] - _origin[1]) / _step);
            int i1 = static_cast<unsigned>((ep[0] - _origin[0]) / _step);
            int j1 = static_cast<unsigned>((sp[1] - _origin[1]) / _step);
            
            if (0 <= i0 && i0 < _dimension[0] &&
                0 <= j0 && j0 < _dimension[1] &&
                0 <= i1 && i1 < _dimension[0] &&
                0 <= j1 && j1 < _dimension[1]){
                if (i0 > i1){
                    vb_swap<int>(i0, i1);
                }
                if (j0 > j1){
                    vb_swap<int>(j0, j1);
                }
                float ov[2] = { _origin[0] + static_cast<float>(i0)*_step, _origin[1] + _step*static_cast<float>(j0) };
                for (int i = i0; i <= i1; ++i){
                    ov[1] = _origin[1] + _step*static_cast<float>(j0);
                    for (int j = j0; j <= j1; ++j){
                        if (BaseType::Util::is2DSegmentAabbCollisionf(sp, ep, ov, _step)){
                            if (0 == _grid[i][j]){
                                _grid[i][j] = new VBGridCell<T>();
                            }
                            _grid[i][j]->addElement(element);
                        }
                        ov[1] += _step;
                    }
                    ov[0] += _step;
                }
            }
            else{
                assert(!"Index is out of range!");
            }
            
        };

        T* findLeftMostElement(){
            T *ret = nullptr;
            if (_grid){
				for (size_t i = 0; i < _dimension[0]; ++i){
					for (size_t j = 0; j < _dimension[1]; ++j){
                        if (_grid[i][j]){
                            ret = _grid[i][j]->findLeftMostElement();
                            if (ret){
                                return ret;
                            }
                        }
                    }
                }
            }
            return ret;
        }


        T* getElementCollisionWithTriangle(float *v0, float *v1, float *v2){
            float x0 = v0[0], x1 = v0[0];
            if (v1[0] < x0){
                x0 = v1[0];
            }
            else if (v1[0] > x1){
                x1 = v1[0];
            }
            if (v2[0] < x0){
                x0 = v2[0];
            }
            else if (v2[0] > x1){
                x1 = v2[0];
            }
            float y0 = v0[1], y1 = v0[1];
            if (v1[1] < y0){
                y0 = v1[1];
            }
            else if (v1[1] > y1){
                y1 = v1[1];
            }
            if (v2[1] < y0){
                y0 = v2[1];
            }
            else if (v2[1] > y1){
                y1 = v2[1];
            }
            int i0 = static_cast<int>((x0 - _origin[0]) / _step);
            int j0 = static_cast<int>((y0 - _origin[1]) / _step);
            int i1 = static_cast<int>((x1 - _origin[0]) / _step);
            int j1 = static_cast<int>((y1 - _origin[1]) / _step);
            float dv1[2] = { v1[0] - v0[0], v1[1] - v0[1] };
            float dv2[2] = { v2[0] - v0[0], v2[1] - v0[1] };
            float nv[2] = { v1[1] - v2[1], v2[0] - v1[0] };
            float d = dv1[0]*nv[0] + dv1[1]*nv[1];
            T *ret = nullptr, *tempret;
            for (int i = i0; i <= i1; ++i){
                for (int j = j0; j <= j1; ++j){
                    if (_grid[i][j]){
                        tempret = _grid[i][j]->getElementInsideTriangleNeedFar(v0, dv1, dv2, nv, d);
                        if (tempret){
                            ret = tempret;
                        }
                    }
                }
            }
            if (ret && ret->getId() >= 0){
                return ret;
            }
            return nullptr;
        }
        /*void addElements(const std::vector<T*> &elements){
            unsigned n = elements.size();
            for (unsigned i = 0; i < n; ++i){
                addElement(elements[i], elements[i]->getDCoord());
            }
        }*/


    };


    
}// namespace
#endif