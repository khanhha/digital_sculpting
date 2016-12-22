#include "VTextureLoader.h"
#include <QImage>
#include <QOpenGLTexture>
#include <QString>

using namespace  gte;

VK_BEGIN_NAMESPACE

Texture2* VTextureLoader::load(std::string const& filename)
{
	QImage image(QString(filename.c_str()));
	image = image.mirrored();
	QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
	DFType gtformat = DF_R8G8B8A8_UNORM;
	size_t width = glImage.width();
	size_t height = glImage.height();
	// Create the 2D texture and compute the stride and image size.
	std::unique_ptr<Texture2> texture(new Texture2(gtformat, width, height, false));
	size_t const stride = width * texture->GetElementSize();
	size_t const imageSize = stride * height;
	memcpy(texture->Get<unsigned char*>(), glImage.constBits(), imageSize);
	return texture.release();
}

VK_END_NAMESPACE
