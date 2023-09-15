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

template<class T> Sketches::CountingSample_impl<T>::CountingSample_impl(
	unsigned long sampleSize,
	Tools::Random& r
) : m_N(0), m_sampleSize(sampleSize), m_tau(1.0), m_pRandom(&r)
{
}

template<class T> Sketches::CountingSample_impl<T>::CountingSample_impl(
	const Sketches::CountingSample_impl<T>& in
) : m_N(in.m_N),
    m_sampleSize(in.m_sampleSize),
    m_tau(in.m_tau),
    m_pRandom(in.m_pRandom)
{
	typename std::set<Key*, KeyLess>::iterator it;

	for (it = in.m_reservoir.begin(); it != in.m_reservoir.end(); it++)
	{
		KeyTally* pP = dynamic_cast<KeyTally*>(*it);
		if (pP != 0) m_reservoir.insert(new KeyTally(pP->m_key, pP->m_count));
		else m_reservoir.insert(new Key((*it)->m_key));
	}
}

template<class T> Sketches::CountingSample_impl<T>::CountingSample_impl(
	const byte* data,
	Tools::Random& r
)
{
}

template<class T> Sketches::CountingSample_impl<T>::~CountingSample_impl()
{
	typename std::set<Key*, KeyLess>::iterator it;
	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++) delete *it;
}

template<class T> Sketches::CountingSample_impl<T>&
Sketches::CountingSample_impl<T>::operator=(
	const CountingSample_impl<T>& in
)
{
	if (this != &in)
	{
		m_N = in.m_N;
		m_sampleSize = in.m_sampleSize;
		m_tau = in.m_tau;
		m_pRandom = in.m_pRandom;

		typename std::set<Key*, KeyLess>::iterator it;

		for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
			delete *it;
		m_reservoir.clear();

		for (it = in.m_reservoir.begin(); it != in.m_reservoir.end(); it++)
		{
			KeyTally* pP = dynamic_cast<KeyTally*>(*it);

			if (pP != 0)
				m_reservoir.insert(new KeyTally(pP->m_key, pP->m_count));
			else
				m_reservoir.insert(new Key((*it)->m_key));
		}
	}

	return *this;
}

template<class T> void Sketches::CountingSample_impl<T>::insert(T key)
{
	m_N++;
	Key* e = new Key(key);
	typename std::set<Key*, KeyLess>::iterator it = m_reservoir.find(e);

	if (it != m_reservoir.end())
	{
		// the entry is already in the sample.
		KeyTally* p = dynamic_cast<KeyTally*>((*it));
		if (p == 0)
		{
			// if the entry was inserted only once,
			// create a key/counter pair for it.
			m_reservoir.erase(it);
			p = new KeyTally(key, 2UL);
			m_reservoir.insert(p);
		}
		else
		{
			// else just increase the count.
			p->m_count += 1;
		}
		delete e;
	}
	else if (m_pRandom->nextUniformDouble() >= 1.0 / m_tau)
	{
		delete e;
	}
	else
	{
		// this is the first occurrence of this entry.

		if (m_reservoir.size() >= m_sampleSize) resample();
		m_reservoir.insert(e);
	}
}

template<class T> void Sketches::CountingSample_impl<T>::erase(T key)
{
	m_N--;
	Key* e = new Key(key);
	typename std::set<Key*, KeyLess>::iterator it = m_reservoir.find(e);
	delete e;

	if (it != m_reservoir.end())
	{
		KeyTally* p = dynamic_cast<KeyTally*>(*it);

		if (p == 0)
		{
			// if the entry was inserted only once, delete it.
			m_reservoir.erase(it);
			delete p;
		}
		else
		{
			// else just decrease the count.
			if (p->m_count == 2)
			{
				m_reservoir.erase(it);
				m_reservoir.insert(new Key(key));
				delete p;
			}
			else
			{
				p->m_count--;
				assert(p->m_count > 1);
			}
		}
	}
}

template<class T> void Sketches::CountingSample_impl<T>::resample()
{
	bool bRemoved = false;
	typename std::set<Key*, KeyLess>::iterator it, it2;
	std::vector<Key*> toInsert;

	while (! bRemoved)
	{
		// this is an empirical increase of the threshold, as specified by
		// P. Gibbons and Y. Matias
		// New Sampling-Based Summary Statistics for
		// Improving Approximate Query Answers
		double tauprime = 1.1 * m_tau;

		toInsert.clear();

		for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
		{
			KeyTally* p = dynamic_cast<KeyTally*>(*it);

			if (p != 0)
			{
				if (m_pRandom->nextUniformDouble() >= m_tau / tauprime) p->m_count--;
				while (
					p->m_count >= 1 &&
					m_pRandom->nextUniformDouble() >= 1.0 / tauprime
				) p->m_count--;

				if (p->m_count <= 1)
				{
					it2 = it;
					it--;
					m_reservoir.erase(it2);

					if (p->m_count == 1) toInsert.push_back(new Key(p->m_key));
					else bRemoved = true;

					delete p;
				}
			}
			else
			{
				if (m_pRandom->nextUniformDouble() >= m_tau / tauprime)
				{
					it2 = it;
					it--;
					m_reservoir.erase(it2);
					delete *it2;
					bRemoved = true;
				}
			}
		}

		m_reservoir.insert(toInsert.begin(), toInsert.end());
		m_tau = tauprime;
	}
}

template<class T> void Sketches::CountingSample_impl<T>::clear()
{
	m_tau = 1.0;
	typename std::set<Key*, KeyLess>::iterator it;
	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++) delete *it;
	m_reservoir.clear();
}

template<class T> unsigned long Sketches::CountingSample_impl<T>::getInputLength() const
{
	return m_N;
}

template<class T> double Sketches::CountingSample_impl<T>::getSamplingFactor() const
{
	return 1.0 / m_tau;
}

template<class T> unsigned long
Sketches::CountingSample_impl<T>::getFrequency(T id) const
{
	Key* e = new Key(id);
	typename std::set<Key*, KeyLess>::iterator it = m_reservoir.find(e);
	delete e;

	unsigned long c = 0;

	if (it != m_reservoir.end())
	{
		c = static_cast<unsigned long>(std::max(0.1, std::floor(0.418 * m_tau - 1.0)));

		const KeyTally* p = dynamic_cast<const KeyTally*>(*it);
		if (p != 0) c += p->m_count;
		else c += 1;
	}

	return c;
}

template<class T> std::map<T, unsigned long>
Sketches::CountingSample_impl<T>::getFrequent(unsigned long theta) const
{
	std::map<T, unsigned long> answers;
	typename std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
	{
		unsigned long c = 1;
		const KeyTally* p = dynamic_cast<const KeyTally*>(*it);
		if (p != 0) c = p->m_count;

		c += static_cast<unsigned long>(std::max(0.1, std::floor(0.418 * m_tau - 1.0)));

		if (c >= theta) answers.insert(std::pair<T, unsigned long>((*it)->m_key, c));
	}

	return answers;
}

template<class T> unsigned long
Sketches::CountingSample_impl<T>::getTotalKeyTallies() const
{
	unsigned long s = 0;
	typename std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
	{
		KeyTally* pP = dynamic_cast<KeyTally*>(*it);
		if (pP != 0) s++;
	}

	return s;
}

template<class T> std::vector<std::pair<T, unsigned long> >
Sketches::CountingSample_impl<T>::getSample() const
{
	std::vector<std::pair<T, unsigned long> > ret;
	typename std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
	{
		KeyTally* pP = dynamic_cast<KeyTally*>(*it);
		if (pP != 0) ret.push_back(std::pair<T, unsigned long>(pP->m_key, pP->m_count));
		else ret.push_back(std::pair<T, unsigned long>((*it)->m_key, 1));
	}

	return ret;
}

/**************************************************/
/*           Template specializations             */
/**************************************************/

template<class T> class CountingSample : public Sketches::CountingSample_impl<T>
{
public:
	CountingSample(unsigned long reservoirSize, Tools::Random& r);
	CountingSample(const byte* data, Tools::Random& r);
	CountingSample(const CountingSample<T>& in);
	virtual ~CountingSample();
	virtual unsigned long getSize() const = 0;
	virtual void getData(byte** data, unsigned long& length) const = 0;
};

Sketches::CountingSample<unsigned long>::CountingSample(
	unsigned long reservoirSize,
	Tools::Random& r
) : CountingSample_impl<unsigned long>(reservoirSize, r)
{
}

Sketches::CountingSample<unsigned long>::CountingSample(
	const byte* data,
	Tools::Random& r
) :  CountingSample_impl<unsigned long>(data, r)
{
	memcpy(&m_N, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_sampleSize, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_tau, data, sizeof(double));
	data += sizeof(double);

	unsigned long size, key, count;
	memcpy(&size, data, sizeof(unsigned long));
	data += sizeof(unsigned long);

	for (unsigned long i = 0; i < size; i++)
	{
		memcpy(&key, data, sizeof(unsigned long));
		data += sizeof(unsigned long);
		memcpy(&count, data, sizeof(unsigned long));
		data += sizeof(unsigned long);

		if (count == 1)
			m_reservoir.insert(new Key(key));
		else
			m_reservoir.insert(new KeyTally(key, count));
	}

	m_pRandom = &r;
}

Sketches::CountingSample<unsigned long>::CountingSample(
	const CountingSample<unsigned long>& in
) : Sketches::CountingSample_impl<unsigned long>(in)
{
}

Sketches::CountingSample<unsigned long>::~CountingSample()
{
}

unsigned long Sketches::CountingSample<unsigned long>::getSize() const
{
	return
		2 * sizeof(unsigned long) +
		sizeof(double) +
		sizeof(unsigned long) +
		m_reservoir.size() * 2 * sizeof(unsigned long);
		// this consumes more space than needed,
		// but it avoids alignment problems.
}

void Sketches::CountingSample<unsigned long>::getData(
	byte** data,
	unsigned long& length
) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	memcpy(p, &m_N, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_sampleSize, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_tau, sizeof(double));
	p += sizeof(double);

	unsigned long l = m_reservoir.size();
	memcpy(p, &l, sizeof(unsigned long));
	p += sizeof(unsigned long);

	std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
	{
		l = (*it)->m_key;
		memcpy(p, &l, sizeof(unsigned long));
		p += sizeof(unsigned long);

		KeyTally* pP = dynamic_cast<KeyTally*>(*it);
		if (pP != 0)
		{
			memcpy(p, &(pP->m_count), sizeof(unsigned long));
			p += sizeof(unsigned long);
		}
		else
		{
			l = 1;
			memcpy(p, &l, sizeof(unsigned long));
			p += sizeof(unsigned long);
		}
	}

	assert(p == (*data) + length);
}

Sketches::CountingSample<std::string>::CountingSample(
	unsigned long reservoirSize,
	Tools::Random& r
) : CountingSample_impl<std::string>(reservoirSize, r)
{
}

Sketches::CountingSample<std::string>::CountingSample(
	const byte* data,
	Tools::Random& r
) :  CountingSample_impl<std::string>(data, r)
{
	memcpy(&m_N, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_sampleSize, data, sizeof(unsigned long));
	data += sizeof(unsigned long);
	memcpy(&m_tau, data, sizeof(double));
	data += sizeof(double);

	unsigned long size, count;
	memcpy(&size, data, sizeof(unsigned long));
	data += sizeof(unsigned long);

	for (unsigned long i = 0; i < size; i++)
	{
		std::string s(reinterpret_cast<const char*>(data));
		data += s.size() + 1;

		memcpy(&count, data, sizeof(unsigned long));
		data += sizeof(unsigned long);

		if (count == 1)
			m_reservoir.insert(new Key(s));
		else
			m_reservoir.insert(new KeyTally(s, count));
	}

	m_pRandom = &r;
}

Sketches::CountingSample<std::string>::CountingSample(
	const CountingSample<std::string>& in
) : Sketches::CountingSample_impl<std::string>(in)
{
}

Sketches::CountingSample<std::string>::~CountingSample()
{
}

unsigned long Sketches::CountingSample<std::string>::getSize() const
{
	unsigned long ret = 3 * sizeof(unsigned long) +	sizeof(double);

	std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
		ret += sizeof(unsigned long) + (*it)->m_key.size() + 1;

	return ret;
}

void Sketches::CountingSample<std::string>::getData(
	byte** data,
	unsigned long& length
) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	memcpy(p, &m_N, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_sampleSize, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_tau, sizeof(double));
	p += sizeof(double);

	unsigned long l = m_reservoir.size();
	memcpy(p, &l, sizeof(unsigned long));
	p += sizeof(unsigned long);

	std::set<Key*, KeyLess>::iterator it;

	for (it = m_reservoir.begin(); it != m_reservoir.end(); it++)
	{
		memcpy(p, (*it)->m_key.c_str(), (*it)->m_key.size());
		p += (*it)->m_key.size();
		*p = 0;
		p++;

		KeyTally* pP = dynamic_cast<KeyTally*>(*it);
		if (pP != 0)
		{
			memcpy(p, &(pP->m_count), sizeof(unsigned long));
			p += sizeof(unsigned long);
		}
		else
		{
			l = 1;
			memcpy(p, &l, sizeof(unsigned long));
			p += sizeof(unsigned long);
		}
	}

	assert(p == (*data) + length);
}

