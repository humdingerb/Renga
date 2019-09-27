/*
 * media.h
 * Copyright (C) 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>
 *
 * Distributed under terms of the MIT license.
 */

#pragma once


#include <gloox/stanzaextension.h>


class Media: public gloox::StanzaExtension
{
	public:
		Media(const gloox::Tag* tag = 0);
		virtual const std::string& filterString() const final;
		virtual gloox::StanzaExtension* newInstance(const gloox::Tag* tag) const final
		{
			return new Media(tag);
		}

		virtual gloox::Tag* tag() const final;
		virtual gloox::StanzaExtension* clone() const final
		{
			// TODO
			return new Media();
		}

		const std::string& uri() const { return fURI; }
		const std::string& type() const { return fType; }
		const std::string& data() const;

	private:
		std::string fType;
		std::string fURI;
};
