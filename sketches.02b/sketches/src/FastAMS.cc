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

Sketches::FastAMS::FastAMS(unsigned long counters, unsigned long hashes)
 : m_seed(time(0)), m_counters(counters)
{
	m_pFilter = new long[m_counters * hashes];
	bzero(m_pFilter, m_counters * hashes * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < hashes; i++)
	{
		m_hash.push_back(Tools::UniversalHash(r));
		m_fourwiseHash.push_back(Tools::UniversalHash(r, 4));
	}
}

Sketches::FastAMS::FastAMS(
	unsigned long counters,
	unsigned long hashes,
	unsigned long seed
) : m_seed(seed), m_counters(counters)
{
	m_pFilter = new long[m_counters * hashes];
	bzero(m_pFilter, m_counters * hashes * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < hashes; i++)
	{
		m_hash.push_back(Tools::UniversalHash(r));
		m_fourwiseHash.push_back(Tools::UniversalHash(r, 4));
	}
}

Sketches::FastAMS::FastAMS(
	const FastAMS& in
) : m_seed(in.m_seed),
    m_counters(in.m_counters),
	m_hash(in.m_hash),
    m_fourwiseHash(in.m_fourwiseHash)
{
	m_pFilter = new long[m_counters * m_hash.size()];
	memcpy(m_pFilter, in.m_pFilter, m_counters * m_hash.size() * sizeof(long));
}

Sketches::FastAMS::FastAMS(const byte* data)
{
	unsigned long hashes;
	memcpy(&hashes, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_counters, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_seed, data, sizeof(unsigned long));
	data += sizeof(unsigned long);

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < hashes; i++)
	{
		m_hash.push_back(Tools::UniversalHash(r));
		m_fourwiseHash.push_back(Tools::UniversalHash(r, 4));
	}

	m_pFilter = new long[m_counters * hashes];
	memcpy(m_pFilter, data, m_counters * hashes * sizeof(long));
}

Sketches::FastAMS::~FastAMS()
{
	delete[] m_pFilter;
}

Sketches::FastAMS& Sketches::FastAMS::operator=(const FastAMS& in)
{
	if (this != &in)
	{
		if (m_counters != in.m_counters || m_hash.size() != in.m_hash.size())
		{
			delete[] m_pFilter;
			m_pFilter = new long[in.m_counters * in.m_hash.size()];
		}

		m_counters = in.m_counters;
		m_hash = in.m_hash;
		m_fourwiseHash = in.m_fourwiseHash;
		m_seed = in.m_seed;
		memcpy(m_pFilter, in.m_pFilter, m_counters * m_hash.size() * sizeof(long));
	}

	return *this;
}

void Sketches::FastAMS::insert(const std::string& id, long val)
{
	unsigned long long l = atoll(id.c_str());
	insert(l, val);
}

void Sketches::FastAMS::insert(
	const Tools::UniversalHash::value_type& id,
	long val
)
{
	for (unsigned long i = 0; i < m_hash.size(); i++)
	{
		unsigned long h = m_hash[i].hash(id) % m_counters;
		unsigned long m = m_fourwiseHash[i].hash(id);

		if ((m & 1) == 1)
			m_pFilter[i * m_counters + h] += val;
		else
			m_pFilter[i * m_counters + h] -= val;
	}
}

void Sketches::FastAMS::erase(const std::string& id, long val)
{
	unsigned long long l = atoll(id.c_str());
	erase(l, val);
}

void Sketches::FastAMS::erase(
	const Tools::UniversalHash::value_type& id,
	long val
)
{
	for (unsigned long i = 0; i < m_hash.size(); i++)
	{
		unsigned long h = m_hash[i].hash(id) % m_counters;
		unsigned long m = m_fourwiseHash[i].hash(id);

		if ((m & 1) == 1)
			m_pFilter[i * m_counters + h] -= val;
		else
			m_pFilter[i * m_counters + h] += val;
	}
}

void Sketches::FastAMS::clear()
{
	bzero(m_pFilter, m_counters * m_hash.size() * sizeof(long));
}

long Sketches::FastAMS::getFrequency(const std::string& id) const
{
	unsigned long long l = atoll(id.c_str());
	return getFrequency(l);
}

long Sketches::FastAMS::getFrequency(
	const Tools::UniversalHash::value_type& id
) const
{
	std::multiset<long> answer;

	for (unsigned long i = 0; i < m_hash.size(); i++)
	{
		unsigned long h = m_hash[i].hash(id) % m_counters;
		unsigned long m = m_fourwiseHash[i].hash(id);

		if ((m & 1) == 1)
			answer.insert(m_pFilter[i * m_counters + h]);
		else
			answer.insert(-m_pFilter[i * m_counters + h]);
	}

	return Sketches::getMedian<long>(answer);
}

unsigned long Sketches::FastAMS::getVectorLength() const
{
	return m_counters;
}

unsigned long Sketches::FastAMS::getNumberOfHashes() const
{
	return m_hash.size();
}

unsigned long Sketches::FastAMS::getSize() const
{
	return
		3 * sizeof(unsigned long) +
		m_counters * m_hash.size() * sizeof(long);
}

void Sketches::FastAMS::getData(byte** data, unsigned long& length) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	unsigned long l = m_hash.size();
	memcpy(p, &l, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_counters, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_seed, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, m_pFilter, m_counters * m_hash.size() * sizeof(long));
	p += m_counters * m_hash.size() * sizeof(long);

	assert(p == (*data) + length);
}

std::ostream& Sketches::operator<<(
	std::ostream& os,
	const Sketches::FastAMS& s
)
{
	os << s.m_hash.size() << " " << s.m_counters;

	for (unsigned long i = 0; i < s.m_hash.size(); i++)
		os << " " << s.m_hash[i];

	for (unsigned long i = 0; i < s.m_fourwiseHash.size(); i++)
		os << " " << s.m_fourwiseHash[i];

	for (unsigned long i = 0; i < s.m_counters * s.m_hash.size(); i++)
		os << " " << s.m_pFilter[i];

	return os;
}

