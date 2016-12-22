#ifndef BMESH_BM_CUSTOM_DATA_H
#define BMESH_BM_CUSTOM_DATA_H
#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include <vector>
#include <boost/pool/pool.hpp>
VM_BEGIN_NAMESPACE


/* number of layers to add when growing a CustomData object */
#define CUSTOMDATA_GROW 5

/** descriptor and storage for a custom data layer */
typedef struct CustomDataLayer {
	int type;       /* type of data in layer */
	int offset;     /* in editmode, offset of layer in block */
	int flag;       /* general purpose flag */
	int active;     /* number of the active layer of this type */
	int uid;        /* shape keyblock unique id reference*/
	char name[64];  /* layer name, MAX_CUSTOMDATA_LAYER_NAME */
} CustomDataLayer;

/* CustomData.type */
typedef enum CustomDataType {
	CD_NORMAL = 0,
	CD_PROP_FLT = 1,
	CD_PROP_INT = 2,
	CD_PROP_POINTER = 3,
	CD_NUMTYPES = 4
} CustomDataType;

/* Bits for CustomDataMask */
#define CD_MASK_NORMAL		(1 << CD_NORMAL)
#define CD_MASK_PROP_FLT	(1 << CD_PROP_FLT)
#define CD_MASK_PROP_INT	(1 << CD_PROP_INT)
#define SOURCE_BUF_SIZE 100

/* CustomData.flag */
enum {
	/* Indicates layer should not be copied by CustomData_from_template or CustomData_copy_data */
	CD_FLAG_NOCOPY = (1 << 0),
	/* Indicates layer should not be freed (for layers backed by external data) */
	CD_FLAG_NOFREE = (1 << 1),
	/* Indicates the layer is only temporary, also implies no copy */
	CD_FLAG_TEMPORARY = ((1 << 2) | CD_FLAG_NOCOPY),
	/* Indicates the layer is stored in an external file */
	CD_FLAG_EXTERNAL = (1 << 3),
	/* Indicates external data is read into memory */
	CD_FLAG_IN_MEMORY = (1 << 4),
};

typedef void(*cd_interp)(const void **sources, const float *weights, const float *sub_weights, int count, void *dest);
typedef void(*cd_copy)(const void *source, void *dest, int count);

/********************* Layer type information **********************/
typedef struct LayerTypeInfo {
	int size;          /* the memory size of one element of this layer's data */

	/** name of the struct used, for file writing */
	const char *structname;
	/** number of structs per element, for file writing */
	int structnum;

	/**
	* default layer name.
	* note! when NULL this is a way to ensure there is only ever one item
	* see: CustomData_layertype_is_singleton() */
	const char *defaultname;

	/**
	* a function to copy count elements of this layer's data
	* (deep copy if appropriate)
	* if NULL, memcpy is used
	*/
	cd_copy copy;

	/**
	* a function to free any dynamically allocated components of this
	* layer's data (note the data pointer itself should not be freed)
	* size should be the size of one element of this layer's data (e.g.
	* LayerTypeInfo.size)
	*/
	void(*free)(void *data, int count, int size);

	/**
	* a function to interpolate between count source elements of this
	* layer's data and store the result in dest
	* if weights == NULL or sub_weights == NULL, they should default to 1
	*
	* weights gives the weight for each element in sources
	* sub_weights gives the sub-element weights for each element in sources
	*    (there should be (sub element count)^2 weights per element)
	* count gives the number of elements in sources
	*
	* \note in some cases \a dest pointer is in \a sources
	*       so all functions have to take this into account and delay
	*       applying changes while reading from sources.
	*       See bug [#32395] - Campbell.
	*/
	cd_interp interp;

	/** a function to swap the data in corners of the element */
	void(*swap)(void *data, const int *corner_indices);

	/**
	* a function to set a layer's data to default values. if NULL, the
	* default is assumed to be all zeros */
	void(*set_default)(void *data, int count);

	/** functions necessary for geometry collapse */
	bool(*equal)(const void *data1, const void *data2);
	void(*multiply)(void *data, float fac);
	void(*initminmax)(void *min, void *max);
	void(*add)(void *data1, const void *data2);
	void(*dominmax)(const void *data1, void *min, void *max);
	void(*copyvalue)(const void *source, void *dest, const int mixmode, const float mixfactor);

	/** a function to determine max allowed number of layers, should be NULL or return -1 if no limit */
	int(*layers_max)(void);
} LayerTypeInfo;

/** structure which stores custom element data associated with mesh elements
* (vertices, edges or faces). The custom data is organized into a series of
* layers, each with a data type (e.g. MTFace, MDeformVert, etc.). */
class CustomData
{
public:
	CustomData();
	CustomData(const std::vector<CustomDataLayer> &layers_, boost::pool<> *pool_);
	CustomData(const CustomData &other);
	~CustomData();

	/* adds a data layer of the given type to the CustomData object, optionally
	* backed by an external data array. the different allocation types are
	* defined above. returns the data of the layer.
	*/
	CustomDataLayer* CustomData_add_layer(int type, void *layer, int totelem);
	/*same as above but accepts a name*/
	CustomDataLayer* CustomData_add_layer_named(int type, void *layerdata, int totelem, const char *name);

	int CustomData_get_offset(int type);
	int CustomData_get_n_offset(int type, int n);
	void *CustomData_bmesh_get(void *block, int type);
	void *CustomData_bmesh_get_n(void *block, int type, int n);
	void CustomData_bmesh_set(void *block, int type, const void *source);
	void CustomData_bmesh_set_n(void *block, int type, int n, const void *source);

	/* returns 1 if a layer with the specified type exists */
	bool CustomData_has_layer(int type);
	int CustomData_get_layer_index(int type);
	int CustomData_get_layer_index_n(int type, int n);
	int CustomData_get_named_layer_index(int type, const char *name);
	int CustomData_get_active_layer_index(int type);
	/* make sure the name of layer at index is unique */
	void CustomData_set_layer_unique_name(int index);

	/* frees the active or first data layer with the give type.
	* returns 1 on success, 0 if no layer with the given type is found
	*
	* in editmode, use EDBM_data_layer_free instead of this function
	*/
	bool CustomData_free_layer(int type, int totelem, int index);

	/* frees the layer index with the give type.
	* returns 1 on success, 0 if no layer with the given type is found
	*
	* in editmode, use EDBM_data_layer_free instead of this function
	*/
	bool CustomData_free_layer_active(int type, int totelem);

	/* same as above, but free all layers with type */
	void CustomData_free_layers(int type, int totelem);

	void CustomData_bmesh_set_default(void **block);
	void CustomData_bmesh_free_block(void **block);
	void CustomData_bmesh_free_block_data(void *block);
	void CustomData_bmesh_interp(const void **src_blocks, const float *weights, const float *sub_weights, int count, void *dst_block);
	void CustomData_bmesh_interp_n(const void **src_blocks_ofs, const float *weights, const float *sub_weights, int count, void *dst_block_ofs, int n);
	static void CustomData_bmesh_copy_data(const CustomData *source, CustomData *dest, void *src_block, void **dest_block);
	void init_pool();
private:
	bool CustomData_is_property_layer(int type);
	bool cd_layer_find_dupe(const char *name, int type, int index);
	std::string randString(int length);
	int CustomData_get_layer_index__notypemap(int type);
	void customData_update_offsets();
	void CustomData_update_typemap(CustomData *data);
	CustomDataLayer* customData_add_layer__internal(int type, void *layerdata, int totelem, const char *name);
	bool customdata_typemap_is_valid();
	void CustomData_bmesh_alloc_block(void **block);
	void CustomData_bmesh_set_default_n(void **block, int n);
private:
	friend class BMesh;
	std::vector<CustomDataLayer> layers;      /* CustomDataLayers, ordered by type */
	int typemap[CD_NUMTYPES];              /* runtime only! - maps types to indices of first layer of that type,
								  * MUST be >= CD_NUMTYPES, but we cant use a define here.
								  * Correct size is ensured in CustomData_update_typemap assert() */
	int totsize;                  /* in editmode, total size of all data layers */
	boost::pool<> *pool;     /* (BMesh Only): Memory pool for allocation of blocks */
};
VM_END_NAMESPACE

#endif