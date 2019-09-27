/*
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#include "media.h"
#include <gloox/tag.h>

#include "Extensions.h"


Media::Media(const gloox::Tag* tag)
	: gloox::StanzaExtension(ExtMedia)
{
    if(!tag || tag->name() != "media" || tag->xmlns() != XMLNS_MEDIA)
      return;

	// Get the URI
	gloox::Tag* uri = tag->findChild("uri");
	fType = uri->findAttribute("type");
	fURI = uri->cdata();
	//gloox::Tag* path = uri->children().front();
	fprintf(stderr, "got Media! %s\n", uri->xml().c_str());
	// Add method to get the data (if the URI is a CID, find the BOB, else,
	// try using BUrlProtocolRoster)
}


const std::string& Media::filterString() const
{
	static const std::string filter = "/iq/query/x/field/media[@xmlns='" + XMLNS_MEDIA + "']";
	return filter;
}


gloox::Tag* Media::tag() const
{
	gloox::Tag* t = new gloox::Tag("media");
	t->setXmlns(XMLNS_MEDIA);
	// TODO fill in
	return t;
}
