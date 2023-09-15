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

Sketches::LossyCounting::LossyCounting(double epsilon)
 : m_epsilon(epsilon), m_w(static_cast<unsigned long>(1.0 / epsilon)), m_L(0), m_bucket(1)
{
}

Sketches::LossyCounting::~LossyCounting()
{
}

Sketches::LossyCounting::Entry::Entry(unsigned long c, unsigned long d)
 : m_count(c), m_delta(d)
{
}

void Sketches::LossyCounting::insert(const std::string& id, unsigned long val)
{
	std::map<std::string, LossyCounting::Entry>::iterator it;

	if (val == 0) val = 1;

	for (unsigned long cVal = 0; cVal < val; cVal++)
	{
		if (m_L > 0 && (m_L % m_w) == 0)
		{
			std::stack<std::string> del;

			for (it = m_sketch.begin(); it != m_sketch.end(); it++)
				if ((*it).second.m_count + (*it).second.m_delta <= m_bucket)
					del.push((*it).first);

			while (! del.empty())
			{
				m_sketch.erase(del.top()); del.pop();
			}

			m_bucket++;
		}

		m_L++;

		it = m_sketch.find(id);
			// search for it every time, since it might
			// have been deleted at the bucket boundary.

		if (it != m_sketch.end())
			(*it).second.m_count++;
		else
			m_sketch.insert(
				std::pair<std::string, LossyCounting::Entry>(
				id,	LossyCounting::Entry(1, m_bucket - 1))
			);
	}
}

void Sketches::LossyCounting::erase(const std::string& id, unsigned long val)
{
	throw Tools::NotSupportedException(
		"LossyCounting: Lossy counting does not support deletions."
	);

	// Lossy counting does not support deletions in general.
	// Under various adversarial inputs this
	// deletion algorithm will produce false dismissals.
	/*
	std::map<std::string, LossyCounting::Entry>::iterator it = m_sketch.find(id);

	if (it != m_sketch.end())
	{
		if ((*it).second.m_count == 1) m_sketch.erase(it);
		else (*it).second.m_count--;
	}

	// we need to decrease the length of the stream but
	// keep the bucket number untouched.
	m_L--;
	*/
}

void Sketches::LossyCounting::insert(unsigned long id, unsigned long val)
{
	std::ostringstream ss;
	ss << id << std::flush;
	insert(ss.str(), val);
}

void Sketches::LossyCounting::erase(unsigned long id, unsigned long val)
{
	throw Tools::NotSupportedException(
		"LossyCounting: Lossy counting does not support deletions."
	);
}

void Sketches::LossyCounting::clear()
{
	m_L = 0;
	m_bucket = 1;
	m_sketch.clear();
}

unsigned long Sketches::LossyCounting::getNumberOfEntries() const
{
	return m_sketch.size();
}

unsigned long Sketches::LossyCounting::getMaxFrequency(
	const std::string& id
) const
{
	std::map<std::string, LossyCounting::Entry>::const_iterator it = m_sketch.find(id);

	if (it != m_sketch.end())
		return (*it).second.m_count + (*it).second.m_delta;

	return m_bucket - 1;
}

unsigned long Sketches::LossyCounting::getMinFrequency(const std::string& id) const
{
	std::map<std::string, LossyCounting::Entry>::const_iterator it = m_sketch.find(id);

	if (it != m_sketch.end())
		return (*it).second.m_count;

	return 0;
}

std::map<std::string, std::pair<unsigned long, unsigned long> >
Sketches::LossyCounting::getFrequent(
	unsigned long theta
) const
{
	std::map<std::string, std::pair<unsigned long, unsigned long> > ret;
	
	std::map<std::string, LossyCounting::Entry>::const_iterator it;

	for (it = m_sketch.begin(); it != m_sketch.end(); it++)
	{
		if ((*it).second.m_count + (*it).second.m_delta >= theta)
			ret.insert(
				std::pair<
					std::string,
					std::pair<unsigned long, unsigned long> >(
						(*it).first,
						std::pair<unsigned long, unsigned long>(
							(*it).second.m_count,
							(*it).second.m_delta))
			);
	}

	return ret;
}

unsigned long Sketches::LossyCounting::getInputLength() const
{
	return m_L;
}

double Sketches::LossyCounting::getEpsilon() const
{
	return m_epsilon;
}

unsigned long Sketches::LossyCounting::getSize() const
{
	unsigned long ret = sizeof(double) + 3 * sizeof(unsigned long) + 2 * m_sketch.size() * sizeof(unsigned long);

	std::map<std::string, LossyCounting::Entry>::const_iterator it;

	for (it = m_sketch.begin(); it != m_sketch.end(); it++)
		ret += (*it).first.size();

	return ret;
}

