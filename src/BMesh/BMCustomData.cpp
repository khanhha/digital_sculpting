#include "BMCustomData.h"
#include <cstring>

VM_BEGIN_NAMESPACE

static void layerCopy_propFloat(const void *source, void *dest, int count)
{
	memcpy(dest, source, sizeof(float) * count);
}

static void layerCopy_propInt(const void *source, void *dest, int count)
{
	memcpy(dest, source, sizeof(int) * count);
}

static void layerCopy_propPtr(const void *source, void *dest, int count)
{
	memcpy(dest, source, sizeof(uintptr_t) * count);
}
static void layerCopyValue_normal(const void *source, void *dest, const int mixmode, const float mixfactor)
{
	const float *no_src = (float*)source;
	float *no_dst = (float*)dest;
	no_dst[0] = no_src[0];	no_dst[1] = no_src[1]; no_dst[2] = no_src[2];
}

static const LayerTypeInfo LAYERTYPEINFO[CD_NUMTYPES] = {
	/* 3 floats per normal vector */
	{ sizeof(float) * 3, "vec3f", 1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, layerCopyValue_normal },
	/* 10: CD_PROP_FLT */
	{ sizeof(float), "MFloatProperty", 1, "Float", layerCopy_propFloat, NULL, NULL, NULL },
	/* 11: CD_PROP_INT */
	{ sizeof(int), "MIntProperty", 1, "Int", layerCopy_propInt, NULL, NULL, NULL },
	{ sizeof(uintptr_t), "MPtrProperty", 1, "Ptr", layerCopy_propPtr, NULL, NULL, NULL },
};

static const char *LAYERTYPENAMES[CD_NUMTYPES] = {
	"vec3f", "MFloatProperty", "MIntProperty",
};

static const LayerTypeInfo *layerType_getInfo(int type)
{
	if (type < 0 || type >= CD_NUMTYPES) return NULL;

	return &LAYERTYPEINFO[type];
}

static const char *layerType_getName(int type)
{
	if (type < 0 || type >= CD_NUMTYPES) return NULL;

	return LAYERTYPENAMES[type];
}
CustomData::CustomData()
	:
	totsize(0),
	pool(nullptr)
{
	for (size_t i = 0; i < sizeof(typemap) / sizeof(typemap[i]); ++i){
		typemap[i] = -1;
	}
}

CustomData::CustomData(const std::vector<CustomDataLayer> &layers_, boost::pool<> *pool_)
	:
	layers(layers_),
	pool(pool_)
{
}


CustomData::CustomData(const CustomData &other)
	:
	layers(other.layers),
	pool(nullptr)
{
	customData_update_offsets();
	init_pool();
}

CustomData::~CustomData()
{
	if (pool)
		delete pool;
	totsize = 0;
}

void CustomData::CustomData_set_layer_unique_name(int index)
{
	CustomDataLayer *nlayer = &layers[index];
	const LayerTypeInfo *typeInfo = layerType_getInfo(nlayer->type);

	if (!typeInfo->defaultname)
		return;

	while (true){
		std::string randstr = randString(2);
		std::string finalStr = std::string(typeInfo->defaultname).append(randstr);
		if (!cd_layer_find_dupe(finalStr.c_str(), nlayer->type, index)){
			strcpy_s(nlayer->name, finalStr.size(), finalStr.c_str());
			break;
		}
	}

	/*BLI_uniquename_cb(customdata_unique_check, &data_arg, DATA_(typeInfo->defaultname), '.', nlayer->name,
		sizeof(nlayer->name));*/
}

/* to use when we're in the middle of modifying layers */
int CustomData::CustomData_get_layer_index__notypemap(int type)
{
	for (size_t i = 0; i < layers.size(); ++i)
		if (layers[i].type == type)
			return i;

	return -1;
}

void CustomData::customData_update_offsets()
{
	const LayerTypeInfo *typeInfo;
	size_t i, offset = 0;

	for (i = 0; i < layers.size(); ++i) {
		typeInfo = layerType_getInfo(layers[i].type);

		layers[i].offset = offset;
		offset += typeInfo->size;
	}

	totsize = offset;
	CustomData_update_typemap(this);
}

CustomDataLayer* CustomData::customData_add_layer__internal(int type, void *layerdata, int totelem, const char *name)
{
	const LayerTypeInfo *typeInfo = layerType_getInfo(type);
	const int size = totelem * typeInfo->size;
	int flag = 0, index = layers.size();
	void *newlayerdata = NULL;

	layers.push_back(CustomDataLayer());

	/* keep layers ordered by type */
	for (; index > 0 && layers[index - 1].type > type; --index)
		layers[index] = layers[index - 1];

	layers[index].type = type;
	layers[index].flag = flag;

	if (name || (name = typeInfo->defaultname)) {
		strncpy_s(layers[index].name, name, sizeof(layers[index].name));
		//CustomData_set_layer_unique_name(index);
	}
	else
		layers[index].name[0] = '\0';

	if (index > 0 && layers[index - 1].type == type) {
		layers[index].active = layers[index - 1].active;
	}
	else {
		layers[index].active = 0;
	}

	customData_update_offsets();

	return &layers[index];
}

bool CustomData::CustomData_has_layer(int type)
{
	return (CustomData_get_layer_index(type) != -1);
}


void CustomData::CustomData_update_typemap(CustomData *data)
{
	int i, lasttype = -1;

	for (i = 0; i < CD_NUMTYPES; i++) {
		data->typemap[i] = -1;
	}

	for (size_t i = 0; i < data->layers.size(); i++) {
		const int type = data->layers[i].type;
		if (type != lasttype) {
			data->typemap[type] = i;
			lasttype = type;
		}
	}
}

/* currently only used in BLI_assert */
#ifndef NDEBUG
bool CustomData::customdata_typemap_is_valid()
{
	CustomData data_copy(layers,  nullptr);
	CustomData_update_typemap(&data_copy);
	return (memcmp(typemap, data_copy.typemap, sizeof(typemap)) == 0);
}
#endif

/* -------------------------------------------------------------------- */
/* index values to access the layers (offset from the layer start) */
int CustomData::CustomData_get_layer_index(int type)
{
	BLI_assert(customdata_typemap_is_valid());
	return typemap[type];
}

int CustomData::CustomData_get_layer_index_n(int type, int n)
{
	int i = CustomData_get_layer_index(type);

	if (i != -1) {
		BLI_assert(i + n < (int)layers.size());
		i = (layers[i + n].type == type) ? (i + n) : (-1);
	}

	return i;
}

int CustomData::CustomData_get_named_layer_index(int type, const char *name)
{
	for (size_t i = 0; i < layers.size(); ++i)
		if (layers[i].type == type)
			if (STREQ(layers[i].name, name))
				return i;

	return -1;
}

int CustomData::CustomData_get_active_layer_index(int type)
{
	const int layer_index = typemap[type];
	BLI_assert(customdata_typemap_is_valid());
	return (layer_index != -1) ? layer_index + layers[layer_index].active : -1;
}

int CustomData::CustomData_get_offset(int type)
{
	/* get the layer index of the active layer of type */
	int layer_index = CustomData_get_active_layer_index(type);
	if (layer_index == -1) return -1;

	return layers[layer_index].offset;
}

int CustomData::CustomData_get_n_offset(int type, int n)
{
	/* get the layer index of the active layer of type */
	int layer_index = CustomData_get_layer_index_n(type, n);
	if (layer_index == -1) return -1;

	return layers[layer_index].offset;
}

void CustomData::CustomData_bmesh_alloc_block(void **block)
{

	if (*block)
		CustomData_bmesh_free_block(block);

	if (totsize > 0)
		*block = pool->malloc();
	else
		*block = NULL;
}

void CustomData::CustomData_bmesh_set_default_n(void **block, int n)
{
	const LayerTypeInfo *typeInfo;
	int offset = layers[n].offset;

	typeInfo = layerType_getInfo(layers[n].type);

	if (typeInfo->set_default) {
		typeInfo->set_default(POINTER_OFFSET(*block, offset), 1);
	}
	else {
		memset(POINTER_OFFSET(*block, offset), 0, typeInfo->size);
	}
}

void CustomData::CustomData_bmesh_set_default(void **block)
{
	if (*block == NULL)
		CustomData_bmesh_alloc_block(block);

	for (size_t i = 0; i < layers.size(); ++i) {
		CustomData_bmesh_set_default_n(block, i);
	}
}

void CustomData::CustomData_bmesh_free_block(void **block)
{
	const LayerTypeInfo *typeInfo;

	if (*block == NULL)
		return;

	for (size_t i = 0; i < layers.size(); ++i) {
		if (!(layers[i].flag & CD_FLAG_NOFREE)) {
			typeInfo = layerType_getInfo(layers[i].type);

			if (typeInfo->free) {
				int offset = layers[i].offset;
				typeInfo->free(POINTER_OFFSET(*block, offset), 1, typeInfo->size);
			}
		}
	}

	if (totsize)
		pool->free(*block);

	*block = NULL;
}

/**
* Same as #CustomData_bmesh_free_block but zero the memory rather then freeing.
*/
void CustomData::CustomData_bmesh_free_block_data(void *block)
{
	const LayerTypeInfo *typeInfo;

	if (block == NULL)
		return;

	for (size_t i = 0; i < layers.size(); ++i) {
		if (!(layers[i].flag & CD_FLAG_NOFREE)) {
			typeInfo = layerType_getInfo(layers[i].type);

			if (typeInfo->free) {
				int offset = layers[i].offset;
				typeInfo->free(POINTER_OFFSET(block, offset), 1, typeInfo->size);
			}
		}
	}

	if (totsize)
		memset(block, 0, totsize);
}


/*static method for copy data between different custom data*/
void CustomData::CustomData_bmesh_copy_data(const CustomData *source, CustomData *dest,
	void *src_block, void **dest_block)
{
	const LayerTypeInfo *typeInfo;
	size_t dest_i, src_i;

	if (*dest_block == NULL) {
		dest->CustomData_bmesh_alloc_block(dest_block);
		if (*dest_block)
			memset(*dest_block, 0, dest->totsize);
	}

	if (source != dest){
		/* copies a layer at a time */
		dest_i = 0;
		for (src_i = 0; src_i < source->layers.size(); ++src_i) {

			/* find the first dest layer with type >= the source type
			* (this should work because layers are ordered by type)
			*/
			while (dest_i < dest->layers.size() && dest->layers[dest_i].type < source->layers[src_i].type) {
				dest_i++;
			}

			/* if there are no more dest layers, we're done */
			if (dest_i >= dest->layers.size()) return;

			/* if we found a matching layer, copy the data */
			if (dest->layers[dest_i].type == source->layers[src_i].type &&
				STREQ(dest->layers[dest_i].name, source->layers[src_i].name))
			{
				const void *src_data = POINTER_OFFSET(src_block, source->layers[src_i].offset);
				void *dest_data = POINTER_OFFSET(*dest_block, dest->layers[dest_i].offset);

				typeInfo = layerType_getInfo(source->layers[src_i].type);

				if (typeInfo->copy)
					typeInfo->copy(src_data, dest_data, 1);
				else
					memcpy(dest_data, src_data, typeInfo->size);

				/* if there are multiple source & dest layers of the same type,
				* we don't want to copy all source layers to the same dest, so
				* increment dest_i
				*/
				dest_i++;
			}
		}
	}
	else{
		/* copies a layer at a time */
		for (size_t lid = 0; lid < source->layers.size(); ++lid) {

			const void *src_data = POINTER_OFFSET(src_block, source->layers[lid].offset);
			void *dest_data = POINTER_OFFSET(*dest_block, source->layers[lid].offset);

			typeInfo = layerType_getInfo(source->layers[lid].type);

			if (typeInfo->copy)
				typeInfo->copy(src_data, dest_data, 1);
			else
				memcpy(dest_data, src_data, typeInfo->size);
		}
	}
}

CustomDataLayer* CustomData::CustomData_add_layer(int type, void *layerdata, int totelem)
{
	CustomDataLayer *layer;
	const LayerTypeInfo *typeInfo = layerType_getInfo(type);

	layer = customData_add_layer__internal(type, layerdata, totelem, typeInfo->defaultname);
	CustomData_update_typemap(this);

	return layer;
}

CustomDataLayer* CustomData::CustomData_add_layer_named(int type, void *layerdata, int totelem, const char *name)
{
	CustomDataLayer *layer;

	layer = customData_add_layer__internal(type, layerdata, totelem, name);

	CustomData_update_typemap(this);

	return layer;
}

bool CustomData::CustomData_free_layer(int type, int totelem, int index)
{
	const int n = index - CustomData_get_layer_index(type);
	int i;

	if (index < 0)
		return false;

	for (i = index + 1; i < (int)layers.size(); ++i)
		layers[i - 1] = layers[i];

	layers.pop_back();

	/* if layer was last of type in array, set new active layer */
	i = CustomData_get_layer_index__notypemap(type);

	if (i != -1) {
		/* don't decrement zero index */
		const int index_nonzero = n ? n : 1;
		CustomDataLayer *layer;

		for (layer = &layers[i]; i < (int)layers.size() && layer->type == type; i++, layer++) {
			if (layer->active >= index_nonzero) layer->active--;
		}
	}

	customData_update_offsets();

	return true;
}

bool CustomData::CustomData_free_layer_active(int type, int totelem)
{
	int index = 0;
	index = CustomData_get_active_layer_index(type);
	if (index == -1)
		return false;
	return CustomData_free_layer(type, totelem, index);
}


void CustomData::CustomData_free_layers(int type, int totelem)
{
	while (CustomData_has_layer(type))
		CustomData_free_layer_active(type, totelem);
}

/*Bmesh Custom Data Functions. Should replace editmesh ones with these as well, due to more effecient memory alloc*/
void* CustomData::CustomData_bmesh_get(void *block, int type)
{
	int layer_index;

	/* get the layer index of the first layer of type */
	layer_index = CustomData_get_active_layer_index(type);
	if (layer_index == -1) return NULL;

	return POINTER_OFFSET(block, layers[layer_index].offset);
}

void* CustomData::CustomData_bmesh_get_n(void *block, int type, int n)
{
	int layer_index;

	/* get the layer index of the first layer of type */
	layer_index = CustomData_get_layer_index(type);
	if (layer_index == -1) return NULL;

	return POINTER_OFFSET(block, layers[layer_index + n].offset);
}

void CustomData::CustomData_bmesh_set(void *block, int type, const void *source)
{
	void *dest = CustomData_bmesh_get(block, type);
	const LayerTypeInfo *typeInfo = layerType_getInfo(type);

	if (!dest) return;

	if (typeInfo->copy)
		typeInfo->copy(source, dest, 1);
	else
		memcpy(dest, source, typeInfo->size);
}

void CustomData::CustomData_bmesh_set_n(void *block, int type, int n, const void *source)
{
	void *dest = CustomData_bmesh_get_n(block, type, n);
	const LayerTypeInfo *typeInfo = layerType_getInfo(type);

	if (!dest) return;

	if (typeInfo->copy)
		typeInfo->copy(source, dest, 1);
	else
		memcpy(dest, source, typeInfo->size);
}

std::string CustomData::randString(int length)
{
	char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
	size_t stringLen = 26 * 2 + 10 + 7;
	std::string randomString;
	randomString.resize(length + 1);

	unsigned int key = 0;

	for (int n = 0; n < length; n++) {
		key = rand() % stringLen;
		randomString[n] = string[key];
	}

	randomString[length] = '\0';

	return randomString;
}

bool CustomData::cd_layer_find_dupe(const char *name, int type, int index)
{
	/* see if there is a duplicate */
	for (size_t i = 0; i < layers.size(); i++) {
		if (i != index) {
			CustomDataLayer *layer = &layers[i];

			if (CustomData_is_property_layer(type)) {
				if (CustomData_is_property_layer(layer->type) && STREQ(layer->name, name)) {
					return true;
				}
			}
			else {
				if (i != index && layer->type == type && STREQ(layer->name, name)) {
					return true;
				}
			}
		}
	}

	return false;
}

bool CustomData::CustomData_is_property_layer(int type)
{
	if ((type == CD_PROP_FLT) || (type == CD_PROP_INT))
		return true;
	return false;
}

void CustomData::init_pool()
{
	BLI_assert(pool == NULL);

	if (totsize) {
#ifdef LIMIT_MEM_POOL
		pool = new boost::pool<>(totsize, 32, 256);
#else
		pool = new boost::pool<>(totsize);
#endif
	}
}

void CustomData::CustomData_bmesh_interp(const void **src_blocks, const float *weights, const float *sub_weights, int count, void *dst_block)
{
	void *source_buf[SOURCE_BUF_SIZE];
	const void **sources = (const void **)source_buf;

	/* slow fallback in case we're interpolating a ridiculous number of
	* elements
	*/
	if (count > SOURCE_BUF_SIZE)
		sources = (const void**)malloc(sizeof(*sources) * count);

	/* interpolates a layer at a time */
	for (size_t i = 0; i < layers.size(); ++i) {
		CustomDataLayer *layer = &layers[i];
		const LayerTypeInfo *typeInfo = layerType_getInfo(layer->type);
		if (typeInfo->interp) {
			for (int j = 0; j < count; ++j) {
				sources[j] = POINTER_OFFSET(src_blocks[j], layer->offset);
			}
			CustomData_bmesh_interp_n(
				sources,
				weights, sub_weights, count,
				POINTER_OFFSET(dst_block, layer->offset), i);
		}
	}

	if (count > SOURCE_BUF_SIZE) free((void *)sources);
}


/**
* \note src_blocks_ofs & dst_block_ofs
* must be pointers to the data, offset by layer->offset already.
*/
void CustomData::CustomData_bmesh_interp_n(const void **src_blocks_ofs, const float *weights, const float *sub_weights, int count, void *dst_block_ofs, int n)
{
	CustomDataLayer *layer = &layers[n];
	const LayerTypeInfo *typeInfo = layerType_getInfo(layer->type);

	typeInfo->interp(src_blocks_ofs, weights, sub_weights, count, dst_block_ofs);
}

VM_END_NAMESPACE