#ifndef H_HVIFUTIL
#define H_HVIFUTIL

#include <IconUtils.h>
#include <Resources.h>

static BBitmap* LoadIconFromResource(const char* name)
{
	BResources resources;
	resources.SetToImage(name);

	size_t size = 0;
	const uint8* buffer = (const uint8*)resources.LoadResource('VICN', name, &size);

	if (buffer == NULL)
		return NULL;

	// FIXME scale this bitmap according to font size
	BBitmap* result = new BBitmap(BRect(0, 0, 19, 19), B_RGBA32);
	status_t status = BIconUtils::GetVectorIcon(buffer, size, result);
	if (status != B_OK) {
		delete result;
		return NULL;
	}
	return result;
}

#endif // H_HVIFUTIL
