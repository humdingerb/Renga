/*
 * Extensions.h
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#pragma once

enum {
	ExtBase = gloox::ExtUser,
	ExtBOB,
	ExtMedia
};

const std::string XMLNS_BOB               = "urn:xmpp:bob";
const std::string XMLNS_MEDIA             = "urn:xmpp:media-element";
