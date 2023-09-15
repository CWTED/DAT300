// Sketches Library
//
// Copyright (C) 2005 Marios Hadjieleftheriou
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include <Sketches.h>

Sketches::BloomFilter::BloomFilter(
	unsigned long bits,
	unsigned long hashes,
	HashType t
) : m_type(t), m_bits(bits), m_hashes(hashes), m_filter(bits, false)
{
	if (m_hashes == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: number of hashes must be larger than zero."
		);

	if (m_bits == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: vector size must be larger than zero."
		);

	if (m_type == HT_UNIVERSAL)
	{
		Tools::Random r;
		for (unsigned long i = 0; i < m_hashes; i++)
			m_hash.push_back(Tools::UniversalHash(r));
	}
	else if (m_type == HT_SHA1)
	{
		if (m_bits >= std::pow(2.0, 16.0))
			throw Tools::IllegalArgumentException(
				"BloomFilter: to use the SHA1 hash, vector size must be in [1, 2^16 - 1]."
			);

		if (m_hashes > 10)
			throw Tools::IllegalArgumentException(
				"BloomFilter: to use the SHA1 hash, number of hashes must be in [1, 10]."
			);
	}
	else
	{
		throw Tools::NotSupportedException(
			"BloomFilter: This hash type is not supported yet."
		);
	}
}

Sketches::BloomFilter::BloomFilter(
	unsigned long bits,
	unsigned long hashes,
	Tools::Random& r
) : m_type(HT_UNIVERSAL), m_bits(bits), m_hashes(hashes), m_filter(bits, false)
{
	if (m_hashes == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: number of hashes must be larger than zero."
		);

	if (m_bits == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: vector size must be larger than zero."
		);

	for (unsigned long i = 0; i < hashes; i++)
		m_hash.push_back(Tools::UniversalHash(r));
}

Sketches::BloomFilter::BloomFilter(
	unsigned long bits,
	const std::vector<Tools::UniversalHash>& hashes
) : m_type(HT_UNIVERSAL),
    m_bits(bits),
    m_hashes(hashes.size()),
    m_filter(bits, false),
    m_hash(hashes)
{
	if (m_hashes == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: number of hashes must be larger than zero."
		);

	if (m_bits == 0)
		throw Tools::IllegalArgumentException(
			"BloomFilter: vector size must be larger than zero."
		);
}

Sketches::BloomFilter::~BloomFilter()
{
}

Sketches::BloomFilter::BloomFilter(const byte* data)
{
	memcpy(&m_type, data, sizeof(HashType));
	data += sizeof(HashType);
	memcpy(&m_hashes, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_bits, data, sizeof(unsigned long));
	data += sizeof(unsigned long);

	if (m_type == HT_UNIVERSAL)
	{
		for (unsigned long i = 0; i < m_hashes; i++)
		{
			Tools::UniversalHash h(data);
			m_hash.push_back(h);
			data += h.getSize();
		}
	}

	bool b;

	for (unsigned long i = 0; i < m_bits; i++)
	{
		memcpy(&b, data, sizeof(bool));
		data += sizeof(bool);
		m_filter.push_back(b);
	}
}

void Sketches::BloomFilter::insert(const std::string& id)
{
	if (m_type == HT_UNIVERSAL)
	{
		unsigned long long l = atoll(id.c_str());
		insert(l);
	}
	else if (m_type == HT_SHA1)
	{
		Tools::SHA1Hash sha;
		unsigned long len;
		byte* data;
		sha.hash(id, &data, len);

		for (unsigned long i = 0; i < m_hashes; i++)
		{
			unsigned long h =
				*(reinterpret_cast<uint16_t*>(data + 2 * i)) % m_bits;

			m_filter[h] = true;
		}
		delete[] data;
	}
	else
	{
		throw Tools::NotSupportedException(
			"BloomFilter: This hash type is not supported yet."
		);
	}
}

void Sketches::BloomFilter::insert(const Tools::UniversalHash::value_type& id)
{
	if (m_type == HT_UNIVERSAL)
	{
		for (unsigned long i = 0; i < m_hashes; i++)
		{
			unsigned long h = m_hash[i].hash(id) % m_bits;
			m_filter[h] = true;
		}
	}
	else if (m_type == HT_SHA1)
	{
		std::ostringstream ss;
		ss << id << std::flush;
		insert(ss.str());
	}
	else
	{
		throw Tools::NotSupportedException(
			"BloomFilter: This hash type is not supported yet."
		);
	}
}

void Sketches::BloomFilter::erase(const std::string& id)
{
	throw Tools::NotSupportedException(
		"BloomFilter: Bloom filters do not support deletions."
	);
}

void Sketches::BloomFilter::erase(const Tools::UniversalHash::value_type& id)
{
	throw Tools::NotSupportedException(
		"BloomFilter: Bloom filters do not support deletions."
	);
}

void Sketches::BloomFilter::clear()
{
	m_filter.assign(m_filter.size(), false);
}

bool Sketches::BloomFilter::contains(const std::string& id) const
{
	if (m_type == HT_UNIVERSAL)
	{
		unsigned long long l = atoll(id.c_str());
		return contains(l);
	}
	else if (m_type == HT_SHA1)
	{
		Tools::SHA1Hash sha;
		unsigned long len;
		byte* data;
		sha.hash(id, &data, len);

		for (unsigned long i = 0; i < m_hashes; i++)
		{
			unsigned long h =
				*(reinterpret_cast<uint16_t*>(data + 2 * i)) % m_bits;

			if (m_filter[h] == false)
			{
				delete[] data;
				return false;
			}
		}

		delete[] data;
		return true;
	}
	else
	{
		throw Tools::NotSupportedException(
			"BloomFilter: This hash type is not supported yet."
		);
	}
}

bool Sketches::BloomFilter::contains(
	const Tools::UniversalHash::value_type& id
) const
{
	if (m_type == HT_UNIVERSAL)
	{
		for (unsigned long i = 0; i < m_hashes; i++)
		{
			unsigned long h = m_hash[i].hash(id) % m_bits;
			if (m_filter[h] == false) return false;
		}
		return true;
	}
	else if (m_type == HT_SHA1)
	{
		std::ostringstream ss;
		ss << id << std::flush;
		return contains(ss.str());
	}
	else
	{
		throw Tools::NotSupportedException(
			"BloomFilter: This hash type is not supported yet."
		);
	}
}

void Sketches::BloomFilter::merge(const Sketches::BloomFilter& in)
{
	// FIXME: I am not checking if the hashes are exactly the same here.
	if (m_type != in.m_type || m_bits != in.m_bits || m_hashes != in.m_hashes)
		throw Tools::IllegalArgumentException(
			"BloomFilter: Bloom filters do not have equal number of "
			"bits or hash functions, or are not of the same hash type."
		);

	for (unsigned long i = 0; i < m_filter.size(); i++)
		m_filter[i] = m_filter[i] | in.m_filter[i];
}

Sketches::BloomFilter Sketches::BloomFilter::getMerged(
	const Sketches::BloomFilter& in
) const
{
	// FIXME: I am not checking if the hashes are exactly the same here.
	if (m_type != in.m_type || m_bits != in.m_bits || m_hashes != in.m_hashes)
		throw Tools::IllegalArgumentException(
			"BloomFilter: Bloom filters do not have equal number of "
			"bits or hash functions, or are not of the same hash type."
		);

	BloomFilter ret(*this);
	ret.merge(in);
	return ret;
}

void Sketches::BloomFilter::intersect(const Sketches::BloomFilter& in)
{
	// FIXME: I am not checking if the hashes are exactly the same here.
	if (m_type != in.m_type || m_bits != in.m_bits || m_hashes != in.m_hashes)
		throw Tools::IllegalArgumentException(
			"BloomFilter: Bloom filters do not have equal number of "
			"bits or hash functions, or are not of the same hash type."
		);

	for (unsigned long i = 0; i < m_filter.size(); i++)
		m_filter[i] = m_filter[i] & in.m_filter[i];
}

Sketches::BloomFilter Sketches::BloomFilter::getIntersection(
	const Sketches::BloomFilter& in
) const
{
	// FIXME: I am not checking if the hashes are exactly the same here.
	if (m_type != in.m_type || m_bits != in.m_bits || m_hashes != in.m_hashes)
		throw Tools::IllegalArgumentException(
			"BloomFilter: Bloom filters do not have equal number of "
			"bits or hash functions, or are not of the same hash type."
		);

	BloomFilter ret(*this);
	ret.intersect(in);
	return ret;
}

unsigned long Sketches::BloomFilter::getVectorLength() const
{
	return m_bits;
}

unsigned long Sketches::BloomFilter::getNumberOfHashes() const
{
	return m_hashes;
}

unsigned long Sketches::BloomFilter::getNumberOfBitsSet() const
{
	unsigned long ret = 0;

	for (unsigned long i = 0; i < m_filter.size(); i++)
		if (m_filter[i] == true) ret++;

	return ret;
}

unsigned long Sketches::BloomFilter::getSize() const
{
	unsigned long ret =
		sizeof(HashType) +
		2 * sizeof(unsigned long) +
		m_filter.size() * sizeof(bool);

	if (m_type == HT_UNIVERSAL)
		for (unsigned long i = 0; i < m_hash.size(); i++)
			ret += m_hash[i].getSize();

	return ret;
}

void Sketches::BloomFilter::getData(
	byte** data,
	unsigned long& length
) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	memcpy(p, &m_type, sizeof(HashType));
	p += sizeof(HashType);
	memcpy(p, &m_hashes, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_bits, sizeof(unsigned long));
	p += sizeof(unsigned long);

	unsigned long l;
	byte* buf;
	for (unsigned long i = 0; i < m_hash.size(); i++)
	{
		m_hash[i].getData(&buf, l);
		memcpy(p, buf, l);
		p += l;
		delete[] buf;
	}

	bool b;
	for (unsigned long i = 0; i < m_filter.size(); i++)
	{
		b = m_filter[i];
		memcpy(p, &b, sizeof(bool));
		p += sizeof(bool);
	}

	assert(p == (*data) + length);
}

std::ostream& Sketches::operator<<(
	std::ostream& os,
	const Sketches::BloomFilter& s
)
{
	os << s.m_type << " " << s.m_hashes << " " << s.m_bits << " ";

	for (unsigned long i = 0; i < s.m_hash.size(); i++)
		os << s.m_hash[i] << " ";

	for (unsigned long i = 0; i < s.m_bits; i++)
		os << s.m_filter[i];

	return os;
}

