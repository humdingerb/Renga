//////////////////////////////////////////////////
// Blabber [FileItem.cpp]
//////////////////////////////////////////////////

#ifndef FILE_ITEM_H
	#include "FileItem.h"
#endif

FileItem::FileItem(const char *label, const char *filename, BMessage *msg, char shortcut = 0, uint32 modifiers = 0)
	: BMenuItem(label, msg, shortcut, modifiers) {
	_filename = filename;
}

string FileItem::Filename()	{
	return _filename;
}