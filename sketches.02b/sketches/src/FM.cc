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

Sketches::FM::FM(
	unsigned long bits,
	unsigned long bitmaps,
	Sketches::HashType t
) : m_type(t)
{
	if (bitmaps < 1 || bitmaps > 256)
		throw Tools::IllegalArgumentException(
			"FM: Number of bitmaps must be in [1, 256]"
		);

	if (m_type == HT_SHA1)
	{
		if (bits < 1 || bits > (Tools::SHA1Hash::HashLength - 1) * CHAR_BIT)
		{
			std::ostringstream ss;
			ss << 
				"FM: Bitmap length must be in [1, " <<
				(Tools::SHA1Hash::HashLength - 1) * CHAR_BIT <<
				"].";
			throw Tools::IllegalArgumentException(ss.str());
		}
	}
	else if (m_type == HT_UNIVERSAL)
	{
		if (bits < 1 || bits > 256)
			throw Tools::IllegalArgumentException(
				"FM: Bitmap length must be in [1, 256]."
			);
	}
	else
	{
		throw Tools::NotSupportedException(
			"FM: This hash type is not supported yet."
		);
	}

	for (unsigned long i = 0; i < bitmaps; i++)
		m_bitmap.push_back(std::vector<bool>(bits, false));
}

Sketches::FM::FM(const byte* data)
{
	reset(data);
}

Sketches::FM::~FM()
{
}

void Sketches::FM::insert(const std::string& id, unsigned long val)
{
	if (m_type == HT_UNIVERSAL)
	{
		unsigned long long l = atoll(id.c_str());
		insert(l);
	}
	else if (m_type == HT_SHA1)
	{
		if (val == 0) val = 1;

		for (unsigned long i = 0; i < val; i++)
		{
			std::ostringstream ss;
			ss << i << "-" << id;

			Tools::SHA1Hash sha;
			unsigned long len;
			byte* data;
			sha.hash(ss.str(), &data, len);

			unsigned long b =
				static_cast<unsigned long>(data[0]) %
				m_bitmap.size();

			unsigned long i = 1, offset = 0;

			/*
			while (data[i] == 0) { i++; offset += CHAR_BIT; }

			if (offset >= m_bitmap[b].size())
			{
				m_bitmap[b][m_bitmap[b].size() - 1] = true;
			}
			else
			{
				byte mask = 1;
				while ((data[i] & mask) == 0) { mask <<= 1; offset++; }
					// this should yield a non-zero bit.
				m_bitmap[b][offset] = true;
			}
			*/

			// optimize for the most common case, where the
			// first byte is not expected to be zero.
			byte mask = 1;

			while ((data[i] & mask) == 0)
			{
				mask <<= 1;
				offset++;

				if (mask == 0)
				{
					if (i == len - 1) break;
						// prevent buffer overrun.
					mask = 1;
					i++;
				}
			}

			if (offset >= m_bitmap[b].size())
				m_bitmap[b][m_bitmap[b].size() - 1] = true;
			else
				m_bitmap[b][offset] = true;
		}
	}
	else
	{
		throw Tools::NotSupportedException(
			"FM: This hash type is not supported yet."
		);
	}
}

void Sketches::FM::insert(
	const Tools::UniversalHash::value_type& id,
	unsigned long val
)
{
	if (m_type == HT_UNIVERSAL)
	{
		if (val == 0) val = 1;

		Tools::Random r(id, Tools::RGT_DRAND48);
		//Tools::Random r(id);
			// The Mersenne generator has huge initialization
			// overhead so, for efficiency, it cannot be used here.
			// On the other hand, drand48 produces low order bits
			// with very small periodicity, so it requires a larger
			// sample of values in order to produce equivalent
			// results.

		for(; val % m_bitmap.size() != 0; --val)
		{
			byte b = r.nextUniformLong(0, m_bitmap.size());
			unsigned long offset = pickOffset(r);
			m_bitmap[b][offset] = true;
		}

		unsigned long bulkAddSize = val / m_bitmap.size();

		if (bulkAddSize > 0)
		{
			unsigned long fl = fillLength(bulkAddSize);

			if (fl > 0)
			{
				for (unsigned long b = 0; b < m_bitmap.size(); b++)
					for (unsigned long j = 0; j < fl; j++)
						m_bitmap[b][j] = true;

				unsigned long remaining =
					pickBinomial(
						bulkAddSize,
						std::pow(0.5, static_cast<double>(fl)),
						r
					);

				for(unsigned long j = 0; j < remaining; j++)
				{
					for (unsigned long b = 0; b < m_bitmap.size(); b++)
					{
						unsigned long offset = pickOffset(r);
						m_bitmap[b][offset] = true;
					}
				}
			}
			else
			{
				for(unsigned long j = 0; j < bulkAddSize; j++)
				{
					unsigned long b = r.nextUniformLong(0L, m_bitmap.size());
					unsigned long offset = pickOffset(r);
					m_bitmap[b][offset] = true;
				}
			}
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
			"FM: This hash type is not supported yet."
		);
	}
}

void Sketches::FM::erase(const std::string& id, unsigned long val)
{
	throw Tools::NotSupportedException(
		"FM: FM sketches do not support deletions."
	);
}

void Sketches::FM::erase(
	const Tools::UniversalHash::value_type& id,
	unsigned long val
)
{
	throw Tools::NotSupportedException(
		"FM: FM sketches do not support deletions."
	);
}

void Sketches::FM::clear()
{
	unsigned long bitmaps = m_bitmap.size();
	unsigned long bits = m_bitmap[0].size();
	m_bitmap.clear();

	for (unsigned long i = 0; i < bitmaps; i++)
		m_bitmap.push_back(std::vector<bool>(bits, false));
}

unsigned long Sketches::FM::getCount() const
{
	unsigned long R, S = 0;

	for (unsigned long b = 0; b < m_bitmap.size(); b++)
	{
		R = 0;
		while (R < m_bitmap[b].size() && m_bitmap[b][R] == true) R++;
		S += R ;
	}

	// we need to scale with m_bitmap.size() here,
	// since during the insertion we only insert each element
	// in one bitmap only (a la Flajolet pseudocode, page 16),
	// instead of in all bitmaps (this is much faster).
	return static_cast<unsigned long>(std::ceil((m_bitmap.size() / PHI) * std::pow(2.0, static_cast<double>(S) / static_cast<double>(m_bitmap.size()))));
}

void Sketches::FM::merge(const Sketches::FM& in)
{
	if (
		m_type != in.m_type ||
		m_bitmap.size() != in.m_bitmap.size() ||
		m_bitmap[0].size() != in.m_bitmap[0].size()
	)
		throw Tools::IllegalArgumentException(
			"FM: FM sketches do not have compatible sizes or types."
		);

	for (unsigned long i = 0; i < m_bitmap.size(); i++)
		for (unsigned long j = 0; j < m_bitmap[i].size(); j++)
			if (in.m_bitmap[i][j]) m_bitmap[i][j] = true;
}

void Sketches::FM::merge(const Sketches::FM& f1, const Sketches::FM& f2)
{
	if (
		f1.m_type != f2.m_type ||
		f1.m_bitmap.size() != f2.m_bitmap.size() ||
		f1.m_bitmap[0].size() != f2.m_bitmap[0].size()
	)
		throw Tools::IllegalArgumentException(
			"FM: FM sketches do not have compatible sizes or types."
		);

	(*this) = f1;
	merge(f2);
}

Sketches::FM Sketches::FM::getMerged(const Sketches::FM& in) const
{
	FM ret(*this);
	ret.merge(in);
	return ret;
}

void Sketches::FM::reset(const byte* data)
{
	byte l;
	// FM type.
	memcpy(&l, data, sizeof(byte));
	data += sizeof(byte);
	m_type = static_cast<HashType>(l);

	// number of bitmaps.
	memcpy(&l, data, sizeof(byte));
	data += sizeof(byte);
	unsigned long bitmaps = static_cast<unsigned long>(l) + 1;

	// number of bits per bitmap.
	memcpy(&l, data, sizeof(byte));
	data += sizeof(byte);
	unsigned long bits = static_cast<unsigned long>(l) + 1;

	if (bits < 1 || bitmaps < 1)
		throw Tools::IllegalArgumentException(
			"FM: Bitmap length and number of "
			"bitmaps must be positive integers."
		);

	for (unsigned long i = 0; i < bitmaps; i++)
		m_bitmap.push_back(std::vector<bool>(bits, false));

	// number of bits in prefix.
	memcpy(&l, data, sizeof(byte));
	data += sizeof(byte);
	unsigned long prefix = static_cast<unsigned long>(l);

	for (unsigned long i = 0; i < bitmaps; i++)
	{
		for (unsigned long j = 0; j < prefix; j++) m_bitmap[i][j] = true;

		// number of significant remaining bytes.
		memcpy(&l, data, sizeof(byte));
		data += sizeof(byte);

		unsigned long cl = prefix;

		for (unsigned long j = 0; j < static_cast<unsigned long>(l); j++)
		{
			byte mask = 1;

			for (unsigned long m = 0; m < CHAR_BIT; m++)
			{
				if ((*data) & mask) m_bitmap[i][cl] = true;
				else m_bitmap[i][cl] = false;
				cl++;
				if (cl >= bits) break;
				mask <<= 1;
			}
			data += sizeof(byte);
		}
		for (unsigned long j = cl; j < bits; j++) m_bitmap[i][j] = false;
	}
	//assert(pC == data + length);
}

bool Sketches::FM::isSubsumedBy(const Sketches::FM& in) const
{
	if (
		m_type != in.m_type ||
		m_bitmap.size() != in.m_bitmap.size() ||
		m_bitmap[0].size() != in.m_bitmap[0].size()
	)
		throw Tools::IllegalArgumentException(
			"FM: FM sketches do not have compatible sizes or types."
		);

	for (unsigned long i = 0; i < m_bitmap.size(); i++)
		for (unsigned long j = 0; j < m_bitmap[i].size(); j++)
			if (m_bitmap[i][j] == true && in.m_bitmap[i][j] == false)
				return false;

	return true;
}

unsigned long Sketches::FM::getUncompressedSize() const
{
	return static_cast<unsigned long>(std::ceil(static_cast<double>(m_bitmap.size() * m_bitmap[0].size()) / CHAR_BIT));
}

unsigned long Sketches::FM::getSize() const
{
	unsigned long l;
	byte* data;
	getData(&data, l);
	delete[] data;
	return l;
}

void Sketches::FM::getData(byte** buffer, unsigned long& length) const
{
	// I will use a stringstream here to store the
	// bytes, since I do not know the final data
	// length to begin with. Notice I can only
	// insert byte values in the stringstream for
	// this to work correctly.

	unsigned long prefix = m_bitmap[0].size();
		// this points to the first zero.

	for (unsigned long i = 0; i < m_bitmap.size(); i++)
	{
		unsigned long j = 0;
		while (j < m_bitmap[i].size() && m_bitmap[i][j] == true) j++;
		if (prefix > j) prefix = j;
	}

	// I will assume here that bitmaps are not
	// larger than 255 bits and that we do not
	// have more than 255 bitmaps.

	assert(m_bitmap.size() <= 256);
	assert(m_bitmap[0].size() <= 256);

	std::ostringstream ret;
	ret 
		<< static_cast<byte>(m_type)
		<< static_cast<byte>(m_bitmap.size() - 1)
		<< static_cast<byte>(m_bitmap[0].size() - 1)
		<< static_cast<byte>(prefix);

	for (unsigned long i = 0; i < m_bitmap.size(); i++)
	{
		std::vector<byte> v;
		byte buf = 0, mask = 1;
		unsigned long postfix = 0;
			// this points to the first of the last zeroes.

		for (unsigned long j = prefix; j < m_bitmap[i].size(); j++)
		{
			if (m_bitmap[i][j] == true)
			{
				buf = buf | mask;
				postfix = j + 1 - prefix;
			}
			mask <<= 1;

			if (mask == 0)
			{
				v.push_back(buf);
				buf = 0;
				mask = 1;
			}
		}
		if (mask != 1) v.push_back(buf);

		unsigned long postfixByte =
			(postfix % CHAR_BIT == 0) ?
				postfix / CHAR_BIT : (postfix / CHAR_BIT) + 1;

		ret << static_cast<byte>(postfixByte);
		for (unsigned long j = 0; j < postfixByte; j++) ret << v[j];
	}

	ret.flush();
	std::string str = ret.str();
	length = str.size();
	*buffer = new byte[length];
	memcpy(*buffer, str.c_str(), length);
}

unsigned long Sketches::FM::getVectorLength() const
{
	return m_bitmap[0].size();
}

unsigned long Sketches::FM::getNumberOfVectors() const
{
	return m_bitmap.size();
}

unsigned long Sketches::FM::pickOffset(Tools::Random& r) const
{
	// this approach needs too many random numbers.
	// Also, it utilizes the low order bits of
	// drand48 which introduces a lot of bias
	// and messes up the accuracy of the filter
	// (see comments below).
	/*
	unsigned long x = 0;
	while(r.flipCoin()) x++;
	if (x >= m_bitmap[0].size()) x = m_bitmap[0].size() - 1;
	return x;
	*/

	// this approach is sexy, but it is not
	// optimized for the most common case,
	// where x is expected not to be zero.
	// FIXME: this approach might introduce bias
	// if the low order bits produced by the PRG
	// are biased, as is the case with drand48.
	// See below on how to fix this
	// (you should try this! It really messes up
	// the results significantly!!)
	/*
	unsigned long offset = 0;
	unsigned long x = static_cast<unsigned long>(r.nextUniformLong());

	while (x == 0)
	{
		offset += sizeof(unsigned long) * CHAR_BIT;
		x = static_cast<unsigned long>(r.nextUniformLong());

		// I could terminate the loop here early
		// if offset becomes larger than needed,
		// by why bother checking since this is
		// expected to happen very rarely.
	}

	if (offset >= m_bitmap[0].size())
	{
		return m_bitmap[0].size() - 1;
	}
	else
	{
		unsigned long mask = 1;
		while ((x & mask) == 0) { mask <<= 1; offset++; }
			// this has to yield a non-zero bit before mask overflows.
		assert(offset < m_bitmap[0].size());
		return offset;
	}
	*/

	// optimize the most common case, where the first
	// random bits are not expected to be all zero.
	// NOTICE: this approach might introduce bias
	// if the low order bits produced by the PRG
	// are biased, as is the case with drand48.
	// So, we sample the PRG twice per iteration
	// and use only the high order bits.
	//assert(sizeof(unsigned long) % 2 == 0);
	unsigned long mask = 1;
	unsigned long offset = 0;
	unsigned long halfLong = (sizeof(unsigned long) / 2) * CHAR_BIT;
	unsigned long hm = (~0UL) << halfLong;
	unsigned long lm = ~hm;

	unsigned long x =
		((static_cast<unsigned long>(r.nextUniformLong()) >> halfLong) & lm) |
		(static_cast<unsigned long>(r.nextUniformLong()) & hm);

	while ((x & mask) == 0)
	{
		mask <<= 1;
		offset++;

		if (mask == 0)
		{
			x = ((static_cast<unsigned long>(r.nextUniformLong()) >> halfLong) & lm) | (static_cast<unsigned long>(r.nextUniformLong()) & hm);
			mask = 1;
		}
	}

	if (offset >= m_bitmap[0].size())
		return m_bitmap[0].size() - 1;
	else
		return offset;
}

unsigned long Sketches::FM::fillLength(unsigned long val) const
{
	if (val <= 10) return 0;

	double value = static_cast<double>(val);
	double logValue = std::log(value);

	return static_cast<unsigned long>(std::floor(std::log(value / (logValue * logValue)) / std::log(2.0)));
}

long Sketches::FM::pickBinomial(
	long nIn, double pIn, Tools::Random& rand
) const
{
	double q = 1.0 - pIn;
	double s = pIn / q;
	double a = static_cast<double>(nIn + 1L) * s;
	double r = std::pow(q, static_cast<double>(nIn));
	double u = rand.nextUniformDouble();
	long x = 0;

	while(u > r)
	{
		u -= r;
		++x;
		r *= (a / x) - s;
	}

	return x;
}

std::ostream& Sketches::operator<<(std::ostream& os, const Sketches::FM& s)
{
	os
		<< s.m_type << " " << s.m_bitmap.size() << " "
		<< s.m_bitmap[0].size() << " ";

	for (unsigned long i = 0; i < s.m_bitmap.size(); i++)
	{
		for (unsigned long j = 0; j < s.m_bitmap[i].size(); j++)
			os << s.m_bitmap[i][j];

		os << " ";
	}

	return os;
}

