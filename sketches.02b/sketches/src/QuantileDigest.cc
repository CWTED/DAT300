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

Sketches::QuantileDigest::Node::Node(long low, long high, unsigned long c)
 : m_low(low), m_high(high), m_c(c)
{
}

bool Sketches::QuantileDigest::Node::PostOrder::operator()(
	const Sketches::QuantileDigest::Node* n1,
	const Sketches::QuantileDigest::Node* n2
)
{
	if (n1->m_high < n2->m_high) return true;
	else if (n1->m_high == n2->m_high) return n1->m_low > n2->m_low;
	else return false;
}

Sketches::QuantileDigest::QuantileDigest(
	unsigned long k,
	long min,
	long max,
	const std::vector<long>& data
) : m_min(min), m_max(max), m_k(k), m_N(0)
{
	std::map<long, unsigned long> hist;

	for (unsigned long i = 0; i < data.size(); i++)
	{
		std::map<long, unsigned long>::iterator it = hist.find(data[i]);
		if (it == hist.end())
			hist.insert(std::pair<long, unsigned long>(data[i], 1));
		else
			(*it).second = (*it).second + 1;
	}

	initialize(hist);
}

Sketches::QuantileDigest::QuantileDigest(
	unsigned long k,
	long min,
	long max,
	const std::map<long, unsigned long>& data
) : m_min(min), m_max(max), m_k(k), m_N(0)
{
	initialize(data);
}

Sketches::QuantileDigest::QuantileDigest(const byte* data)
{
	const byte* p = data;
	memcpy(&m_min, p, sizeof(long));
	p += sizeof(long);
	memcpy(&m_max, p, sizeof(long));
	p += sizeof(long);
	memcpy(&m_k, p, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(&m_N, p, sizeof(unsigned long));
	p += sizeof(unsigned long);

	unsigned long height;
	memcpy(&height, p, sizeof(unsigned long));
	p += sizeof(unsigned long);

	for (unsigned long h = 0; h < height; h++)
	{
		m_qd.push_back(std::set<Node*, Node::PostOrder>());

		unsigned long width, c;
		long m, M;

		memcpy(&width, p, sizeof(unsigned long));
		p += sizeof(unsigned long);

		for (unsigned long i = 0; i < width; i++)
		{
			memcpy(&m, p, sizeof(long));
			p += sizeof(long);
			memcpy(&M, p, sizeof(long));
			p += sizeof(long);
			memcpy(&c, p, sizeof(unsigned long));
			p += sizeof(unsigned long);

			m_qd[m_qd.size() - 1].insert(new Node(m, M, c));
		}
	}
}

Sketches::QuantileDigest::QuantileDigest(const QuantileDigest& in)
 : m_min(in.m_min), m_max(in.m_max), m_k(in.m_k), m_N(in.m_N)
{
	std::set<Node*, Node::PostOrder>::iterator it;

	for (unsigned long h = 0; h < in.m_qd.size(); h++)
	{
		m_qd.push_back(std::set<Node*, Node::PostOrder>());

		for (it = in.m_qd[h].begin(); it != in.m_qd[h].end(); it++)
			m_qd[h].insert(new Node((*it)->m_low, (*it)->m_high, (*it)->m_c));
	}
}

Sketches::QuantileDigest::~QuantileDigest()
{
	for (unsigned long h = 0; h < m_qd.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = m_qd[h].begin(); it != m_qd[h].end(); it++)
			delete *it;
	}
}

Sketches::QuantileDigest& Sketches::QuantileDigest::operator=(
	const QuantileDigest& in
)
{
	if (this != &in)
	{
		m_min = in.m_min;
		m_max = in.m_max;
		m_k = in.m_k;
		m_N = in.m_N;

		std::set<Node*, Node::PostOrder>::iterator it;

		for (unsigned long h = 0; h < m_qd.size(); h++)
			for (it = m_qd[h].begin(); it != m_qd[h].end(); it++)
				delete *it;

		m_qd.clear();

		for (unsigned long h = 0; h < in.m_qd.size(); h++)
		{
			m_qd.push_back(std::set<Node*, Node::PostOrder>());

			for (it = in.m_qd[h].begin(); it != in.m_qd[h].end(); it++)
				m_qd[h].insert(new Node(
					(*it)->m_low,
					(*it)->m_high,
					(*it)->m_c)
				);
		}
	}

	return *this;
}

void Sketches::QuantileDigest::initialize(
	const std::map<long, unsigned long>& data
) throw(Tools::IllegalArgumentException)
{
	unsigned long height = static_cast<unsigned long>(std::ceil(log10(std::abs(m_max - m_min) + 1.0) / log10(2.0) + 1.0));

	std::vector<std::set<Node*, Node::PostOrder> > qd;

	for (unsigned long h = 0; h < height; h++)
		qd.push_back(std::set<Node*, Node::PostOrder>());

	m_N = 0;
	std::map<long, unsigned long>::const_iterator it;

	for (it = data.begin(); it != data.end(); it++)
	{
		//assert((*it).first >= m_min && (*it).first <= m_max);
		if ((*it).first < m_min || (*it).first > m_max)
			throw Tools::IllegalArgumentException(
				"QuantileDigest::QuantileDigest: Data is out of bounds."
			);

		m_N += (*it).second;
		qd[0].insert(new Node((*it).first, (*it).first, (*it).second));
	}

	compress(qd);

	for (unsigned long h = 0; h < qd.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it2;

		for (it2 = qd[h].begin(); it2 != qd[h].end(); it2++)
			delete *it2;
	}
}

void Sketches::QuantileDigest::getQuantile(
	double q,
	long& value,
	unsigned long& rank
) const
{
	std::set<Node*, Node::PostOrder> s(m_qd[0]);

	for (unsigned long h = 1; h < m_qd.size(); h++)
		s.insert(m_qd[h].begin(), m_qd[h].end());

	std::set<Node*, Node::PostOrder>::iterator it = s.begin();
	rank = 0;

	while (
		it != s.end() &&
		rank < static_cast<unsigned long>(std::floor(m_N * q))
	)
	{
		rank += (*it)->m_c;
		it++;
	}

	it--;
	value = (*it)->m_high;
}

unsigned long Sketches::QuantileDigest::getInputLength() const
{
	return m_N;
}

void Sketches::QuantileDigest::merge(const Sketches::QuantileDigest& in)
{
	assert(in.m_qd.size() == m_qd.size());

	std::vector<std::set<Node*, Node::PostOrder> > u;

	for (unsigned long h = 0; h < m_qd.size(); h++)
	{
		u.push_back(m_qd[h]);
		std::vector<Node*> tmp;

		std::set<Node*, Node::PostOrder>::iterator itIn;
		for (itIn = in.m_qd[h].begin(); itIn != in.m_qd[h].end(); itIn++)
		{
			std::set<Node*, Node::PostOrder>::iterator itThis = u[h].find(*itIn);
			if (itThis != u[h].end())
				(*itThis)->m_c += (*itIn)->m_c;
			else
				tmp.push_back(new Node(
					(*itIn)->m_low, (*itIn)->m_high, (*itIn)->m_c)
				);
		}

		for (unsigned long i = 0; i < tmp.size(); i++)
			u[h].insert(tmp[i]);
	}

	m_N += in.m_N;

	compress(u);

	for (unsigned long h = 0; h < u.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = u[h].begin(); it != u[h].end(); it++)
			delete *it;
	}
}

Sketches::QuantileDigest Sketches::QuantileDigest::getMerged(
	const Sketches::QuantileDigest& in
) const
{
	assert(in.m_qd.size() == m_qd.size());

	QuantileDigest ret(*this);
	ret.merge(in);
	return ret;
}

unsigned long Sketches::QuantileDigest::getSize() const
{
	// FIXME: I should use the post order node
	// identifier here for the node range in order to
	// save an extra 4 bytes per node entry.
	unsigned long ret = 2 * sizeof(long) + 3 * sizeof(unsigned long);

	for (unsigned long h = 0; h < m_qd.size(); h++)
		ret +=
			sizeof(unsigned long) +
			2 * m_qd[h].size() *  sizeof(long) +
			m_qd[h].size() * sizeof(unsigned long);

	return ret;
}


void Sketches::QuantileDigest::getData(
	byte** data,
	unsigned long& length
) const
{
	length = getSize();
	*data = new byte[length];
	byte* p = *data;

	memcpy(p, &m_min, sizeof(long));
	p += sizeof(long);
	memcpy(p, &m_max, sizeof(long));
	p += sizeof(long);
	memcpy(p, &m_k, sizeof(unsigned long));
	p += sizeof(unsigned long);
	memcpy(p, &m_N, sizeof(unsigned long));
	p += sizeof(unsigned long);

	unsigned long l = m_qd.size();
	memcpy(p, &l, sizeof(unsigned long));
	p += sizeof(unsigned long);

	std::set<Node*, Node::PostOrder>::iterator it;

	for (unsigned long h = 0; h < m_qd.size(); h++)
	{
		l = m_qd[h].size();
		memcpy(p, &l, sizeof(unsigned long));
		p += sizeof(unsigned long);

		for (it = m_qd[h].begin(); it != m_qd[h].end(); it++)
		{
			l = (*it)->m_low;
			memcpy(p, &l, sizeof(long));
			p += sizeof(long);			
			l = (*it)->m_high;
			memcpy(p, &l, sizeof(long));
			p += sizeof(long);			
			l = (*it)->m_c;
			memcpy(p, &l, sizeof(unsigned long));
			p += sizeof(unsigned long);			
		}
	}

	assert(static_cast<unsigned long>(p - *data) == length);
}

void Sketches::QuantileDigest::compress(
	std::vector<std::set<Node*, Node::PostOrder> >& qd
)
{
	// We need to be careful here. Normally, I should
	// delete all Nodes before clearing the vector.
	// Nevertheless, I will let the caller take care of that.
	// So be careful when you call compress.
	m_qd.clear();
	for (unsigned long h = 0; h < qd.size(); h++)
		m_qd.push_back(std::set<Node*, Node::PostOrder>());

	unsigned long l =
		static_cast<unsigned long>(std::floor(static_cast<double>(m_N) / m_k));

	Node *pSibling = 0, *pParent = 0;

	for (unsigned long h = 0; h < qd.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = qd[h].begin(); it != qd[h].end(); it++)
		{
			Node* n = *it;

			if (n->m_c == 0) continue;

			// find the parent
			unsigned long children = n->m_high - n->m_low + 1;
			unsigned long pchildren = 2 * children;
			bool leftChild =  (n->m_low % pchildren == 0) ? true : false;
			unsigned long plow = (leftChild) ? n->m_low : n->m_low - children;
			unsigned long phigh = (leftChild) ? n->m_high + children : n->m_high;
			unsigned long slow = (leftChild) ? n->m_high + 1 : n->m_low - children;
			unsigned long shigh = (leftChild) ? n->m_high + children : n->m_low - 1;

			bool pfound = false;
			bool sfound = false;

			if (h < qd.size() - 1)
			{
				Node tmpNode(plow, phigh, 0);
				std::set<Node*, Node::PostOrder>::iterator itTmp =
					qd[h + 1].find(&tmpNode);

				if (itTmp != qd[h + 1].end())
				{
					pParent = *itTmp;
					pfound = true;
				}
			}

			Node tmpNode(slow, shigh, 0);
			std::set<Node*, Node::PostOrder>::iterator itTmp =
				qd[h].find(&tmpNode);

			if (itTmp != qd[h].end())
			{
				pSibling = *itTmp;
				sfound = true;
			}

			unsigned long sum = n->m_c;
			if (pfound) sum += pParent->m_c;
			if (sfound) sum += pSibling->m_c;

			if (sum < l)
			{
				if (sfound) pSibling->m_c = 0;
				
				if (pfound)
				{
					pParent->m_c = sum;
				}
				else
				{
					if (h < qd.size() - 1)
						qd[h + 1].insert(new Node(plow, phigh, sum));
					else
						m_qd[h].insert(new Node(n->m_low, n->m_high, sum));
				}
			}
			else
			{
				m_qd[h].insert(new Node(n->m_low, n->m_high, n->m_c));
			}
		}
	}
}

std::ostream& Sketches::operator<<(std::ostream& os, const Sketches::QuantileDigest& s)
{
	for (unsigned long h = 0; h < s.m_qd.size(); h++)
	{
		os << h << ":";
		std::set<Sketches::QuantileDigest::Node*, Sketches::QuantileDigest::Node::PostOrder>::iterator it;

		for (it = s.m_qd[h].begin(); it != s.m_qd[h].end(); it++)
		{
			os
				<< "<" << (*it)->m_low << ", " << (*it)->m_high
				<< ", " << (*it)->m_c << "> ";
		}
		os << std::endl;
	}
	
	return os;
}

