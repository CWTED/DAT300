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

Sketches::AMS::AMS(double e, double d)
 : m_seed(time(0)),
   m_s1(static_cast<unsigned long>(std::ceil(16.0 / (e*e)))),
   m_s2(static_cast<unsigned long>(std::ceil(2 * std::log10(1.0 / d) / std::log10(2.0))))
{
	m_pAtomicSketch = new long[m_s1 * m_s2];
	bzero(m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < m_s1 * m_s2; i++)
		m_hash.push_back(Tools::UniversalHash(r, 4));
}

Sketches::AMS::AMS(unsigned long s1, unsigned long s2)
 : m_seed(time(0)), m_s1(s1), m_s2(s2)
{
	m_pAtomicSketch = new long[m_s1 * m_s2];
	bzero(m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < m_s1 * m_s2; i++)
		m_hash.push_back(Tools::UniversalHash(r, 4));
}

Sketches::AMS::AMS(double e, double d, unsigned long seed)
 : m_seed(seed),
   m_s1(static_cast<unsigned long>(std::ceil(16.0 / (e*e)))),
   m_s2(static_cast<unsigned long>(std::ceil(2 * std::log10(1.0 / d) / std::log10(2.0))))
{
	m_pAtomicSketch = new long[m_s1 * m_s2];
	bzero(m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < m_s1 * m_s2; i++)
		m_hash.push_back(Tools::UniversalHash(r, 4));
}

Sketches::AMS::AMS(unsigned long s1, unsigned long s2, unsigned long seed)
 : m_seed(seed), m_s1(s1), m_s2(s2)
{
	m_pAtomicSketch = new long[m_s1 * m_s2];
	bzero(m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < m_s1 * m_s2; i++)
		m_hash.push_back(Tools::UniversalHash(r, 4));
}

Sketches::AMS::AMS(const Sketches::AMS& in)
 : m_seed(in.m_seed), m_s1(in.m_s1), m_s2(in.m_s2), m_hash(in.m_hash)
{
	m_pAtomicSketch = new long[m_s1 * m_s2];
	memcpy(m_pAtomicSketch, in.m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));
}

Sketches::AMS::AMS(const byte* data)
{
	memcpy(&m_s1, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_s2, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_seed, data, sizeof(unsigned long));
	data += sizeof(unsigned long);

	Tools::Random r(m_seed);
	for (unsigned long i = 0; i < m_s1 * m_s2; i++)
		m_hash.push_back(Tools::UniversalHash(r, 4));

	m_pAtomicSketch = new long[m_s1 * m_s2];
	memcpy(m_pAtomicSketch, data, m_s1 * m_s2 * sizeof(long));
}

Sketches::AMS::~AMS()
{
	delete[] m_pAtomicSketch;
}

Sketches::AMS& Sketches::AMS::operator=(const AMS& in)
{
	if (this != &in)
	{
		if (m_hash.size() != in.m_hash.size())
		{
			delete[] m_pAtomicSketch;
			m_pAtomicSketch = new long[in.m_s1 * in.m_s2];
		}

		m_hash = in.m_hash;
		m_s1 = in.m_s1;
		m_s2 = in.m_s2;
		m_seed = in.m_seed;
		memcpy(m_pAtomicSketch, in.m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));
	}

	return *this;
}

void Sketches::AMS::insert(const std::string& id, long val)
{
	unsigned long long l = atoll(id.c_str());
	insert(l, val);
}

void Sketches::AMS::insert(
	const Tools::UniversalHash::value_type& id,
	long val
)
{
	for (unsigned long i = 0; i < m_s2; i++)
	{
		for (unsigned long j = 0; j < m_s1; j++)
		{
			unsigned long m = m_hash[j + i * m_s1].hash(id);
			if ((m & 1) == 1) m_pAtomicSketch[j + i * m_s1] += val;
			else m_pAtomicSketch[j + i * m_s1] -= val;
		}
	}
}

void Sketches::AMS::erase(const std::string& id, long val)
{
	unsigned long long l = atoll(id.c_str());
	erase(l);
}

void Sketches::AMS::erase(
	const Tools::UniversalHash::value_type& id,
	long val
)
{
	for (unsigned long i = 0; i < m_s2; i++)
	{
		for (unsigned long j = 0; j < m_s1; j++)
		{
			unsigned long m = m_hash[j + i * m_s1].hash(id);
			if ((m & 1) == 1) m_pAtomicSketch[j + i * m_s1] -= val;
			else m_pAtomicSketch[j + i * m_s1] += val;
		}
	}
}

void Sketches::AMS::clear()
{
	bzero(m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));
}

double Sketches::AMS::getF2Norm() const
{
	std::multiset<double> answer;

	for (unsigned long i = 0; i < m_s2; i++)
	{
		double d = 0;

		for (unsigned long j = 0; j < m_s1; j++)
			d += std::pow(m_pAtomicSketch[j + i * m_s1], 2.0);

		answer.insert(d / static_cast<double>(m_s1));
	}

	return Sketches::getMedian<double>(answer);
}

long Sketches::AMS::getFrequency(const std::string& id) const
{
	unsigned long long l = atoll(id.c_str());
	return getFrequency(l);
}

long Sketches::AMS::getFrequency(
	const Tools::UniversalHash::value_type& id
) const
{
	double normA = std::pow(getF2Norm(), 0.5);
	std::multiset<double> answerD;

	for (unsigned long i = 0; i < m_s2; i++)
	{
		double D = 0;

		for (unsigned long j = 0; j < m_s1; j++)
		{
			unsigned long m = m_hash[j + i * m_s1].hash(id);
			if ((m & 1) == 1)
				D += std::pow(m_pAtomicSketch[j + i * m_s1] / normA - 1.0, 2.0);
			else
				D += std::pow(m_pAtomicSketch[j + i * m_s1] / normA + 1.0, 2.0);
		}

		answerD.insert(D / static_cast<double>(m_s1));
	}

	double dist2 = Sketches::getMedian<double>(answerD);
	return static_cast<long>((1.0 - (dist2 / 2.0)) * normA);
}

unsigned long Sketches::AMS::getVectorLength() const
{
	return m_s1;
}

unsigned long Sketches::AMS::getNumberOfVectors() const
{
	return m_s2;
}

unsigned long Sketches::AMS::getSize() const
{
	return	
		3 * sizeof(unsigned long) +
		m_s1 * m_s2 * sizeof(long);
}

void Sketches::AMS::getData(byte** data, unsigned long& length) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	memcpy(p, &m_s1, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_s2, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_seed, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, m_pAtomicSketch, m_s1 * m_s2 * sizeof(long));
	p += m_s1 * m_s2 * sizeof(unsigned long);

	assert(p == (*data) + length);
}

std::ostream& Sketches::operator<<(std::ostream& os, const Sketches::AMS& s)
{
	os << s.m_s1 << " " << s.m_s2 << " ";

	for (unsigned long i = 0; i < s.m_hash.size(); i++)
		os << s.m_hash[i];

	for (unsigned long i = 0; i < s.m_s2; i++)
	{
		for (unsigned long j = 0; j < s.m_s1; j++)
			os << " " << s.m_pAtomicSketch[i * s.m_s1 + j];
	}

	return os;
}

