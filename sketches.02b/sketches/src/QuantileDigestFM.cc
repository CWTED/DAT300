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

Sketches::QuantileDigestFM::Node::Node(
	const std::string& id,
	long low,
	long high,
	unsigned long rank,
	unsigned long c,
	unsigned long fmSize,
	unsigned long fmBitmaps
) : m_low(low), m_high(high), m_c(fmSize, fmBitmaps, HT_SHA1)
{
	std::ostringstream ss;
	ss << rank << "-" << id << std::flush;
	m_c.insert(id, c);
}

Sketches::QuantileDigestFM::Node::Node(
	long low,
	long high,
	const Sketches::FM& c
) : m_low(low), m_high(high), m_c(c)
{
}

bool Sketches::QuantileDigestFM::Node::PostOrder::operator()(
	const Sketches::QuantileDigestFM::Node* n1,
	const Sketches::QuantileDigestFM::Node* n2
)
{
	if (n1->m_high < n2->m_high) return true;
	else if (n1->m_high == n2->m_high) return n1->m_low > n2->m_low;
	else return false;
}

unsigned long Sketches::QuantileDigestFM::Node::getNodeRank(
	unsigned long low,
	unsigned long high
)
{
	// this computes the rank in the postorder traversal.
	// I can't find a better way to do this.
	unsigned long rank = 0;
	unsigned long i = low - 1;

	while (i > 1)
	{
		unsigned long n = static_cast<unsigned long>(std::pow(2.0, std::floor(std::log10(i) / std::log10(2.0))));
		rank += static_cast<unsigned long>(std::pow(2.0, std::log10(n) / std::log10(2.0) + 1.0)) - 1;
		i -= n;
	}
	rank += i;

	if (low != high)
	{
		rank += static_cast<unsigned long>(std::pow(2.0, std::log10(high - low + 1) / std::log10(2.0) + 1.0)) - 1;
	}

	return rank;
}

Sketches::QuantileDigestFM::QuantileDigestFM(
	const std::string& id,
	const QuantileDigest& in,
	unsigned long fmSize,
	unsigned long fmBitmaps
) : m_id(id),
    m_N(fmSize, 30, HT_SHA1)
{
	initialize(in, fmSize, fmBitmaps);
}

Sketches::QuantileDigestFM::QuantileDigestFM(
	unsigned long id,
	const QuantileDigest& in,
	unsigned long fmSize,
	unsigned long fmBitmaps
) : m_N(fmSize, 30, HT_SHA1)
{
	std::ostringstream ss;
	ss.write(reinterpret_cast<const char*>(&id), sizeof(unsigned long));
	m_id = ss.str();

	initialize(in, fmSize, fmBitmaps);
}

Sketches::QuantileDigestFM::QuantileDigestFM(const QuantileDigestFM& in)
 : m_id(in.m_id), m_min(in.m_min), m_max(in.m_max), m_k(in.m_k), m_N(in.m_N)
{
	std::set<Node*, Node::PostOrder>::iterator it;

	for (unsigned long h = 0; h < in.m_qd.size(); h++)
	{
		m_qd.push_back(std::set<Node*, Node::PostOrder>());

		for (it = in.m_qd[h].begin(); it != in.m_qd[h].end(); it++)
			m_qd[h].insert(new Node((*it)->m_low, (*it)->m_high, (*it)->m_c));
	}
}

Sketches::QuantileDigestFM::~QuantileDigestFM()
{
	for (unsigned long h = 0; h < m_qd.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = m_qd[h].begin(); it != m_qd[h].end(); it++)
			delete *it;
	}
}

Sketches::QuantileDigestFM& Sketches::QuantileDigestFM::operator=(
const QuantileDigestFM& in
)
{
	if (this != &in)
	{
		m_id = in.m_id;
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

void Sketches::QuantileDigestFM::getQuantile(
	double q,
	long& value,
	unsigned long& rank
) const
{
	getQuantile(m_N.getCount(), q, value, rank);
}

void Sketches::QuantileDigestFM::getQuantile(
	unsigned long total,
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
		rank < static_cast<unsigned long>(std::floor(total * q))
	)
	{
		rank += (*it)->m_c.getCount();
		it++;
	}

	it--;
	value = (*it)->m_high;
}

unsigned long Sketches::QuantileDigestFM::getInputLength() const
{
	return m_N.getCount();
}

void Sketches::QuantileDigestFM::merge(const Sketches::QuantileDigestFM& in)
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
			std::set<Node*, Node::PostOrder>::iterator itThis =
				u[h].find(*itIn);

			if (itThis != u[h].end())
				(*itThis)->m_c.merge((*itIn)->m_c);
			else
				tmp.push_back(new Node(
					(*itIn)->m_low,
					(*itIn)->m_high,
					(*itIn)->m_c)
				);
		}

		for (unsigned long i = 0; i < tmp.size(); i++)
			u[h].insert(tmp[i]);
	}

	m_N.merge(in.m_N);

	// I should not delete entries of m_qd here.
	// I will delete them through u afterwards.
	compress(u);

	for (unsigned long h = 0; h < u.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = u[h].begin(); it != u[h].end(); it++)
			delete *it;
	}
}

Sketches::QuantileDigestFM Sketches::QuantileDigestFM::getMerged(
	const Sketches::QuantileDigestFM& in
) const
{
	assert(in.m_qd.size() == m_qd.size());

	QuantileDigestFM ret(*this);
	ret.merge(in);
	return ret;
}

unsigned long Sketches::QuantileDigestFM::getSize() const
{
	unsigned long cNodes = 0;
	unsigned long fmSize = 0;

	for (unsigned long cIndex = 0; cIndex < m_qd.size(); cIndex++)
	{
		cNodes += m_qd[cIndex].size();
		if (fmSize == 0 && m_qd[cIndex].size() > 0)
			fmSize = (*(m_qd[cIndex].begin()))->m_c.getSize();
	}

	return
		2 * sizeof(long) +
		2 * sizeof(unsigned long) +
		m_N.getSize() +
		cNodes * (sizeof(unsigned long) + fmSize);
}

void Sketches::QuantileDigestFM::initialize(
	const QuantileDigest& in,
	unsigned long fmSize,
	unsigned long fmBitmaps
)
{
	m_N.clear();
	m_qd.clear();

	m_min = in.m_min;
	m_max = in.m_max;
	m_k = in.m_k;
	m_N.insert(m_id, in.m_N);

	for (unsigned long h = 0; h < in.m_qd.size(); h++)
	{
		m_qd.push_back(std::set<Node*, Node::PostOrder>());

		std::set<
			QuantileDigest::Node*,
			QuantileDigest::Node::PostOrder
		>::iterator it;

		for (it = in.m_qd[h].begin(); it != in.m_qd[h].end(); it++)
		{
			unsigned long rank = Node::getNodeRank(
				(*it)->m_low - m_min + 1,
				(*it)->m_high - m_min + 1
			);

			m_qd[h].insert(new Node(
				m_id,
				(*it)->m_low,
				(*it)->m_high,
				rank,
				(*it)->m_c, fmSize, fmBitmaps)
			);
		}
	}
}

void Sketches::QuantileDigestFM::compress(
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

	unsigned long l = static_cast<unsigned long>(std::floor(static_cast<double>(m_N.getCount()) / m_k));
	Node *pSibling = 0, *pParent = 0;

	for (unsigned long h = 0; h < qd.size(); h++)
	{
		std::set<Node*, Node::PostOrder>::iterator it;

		for (it = qd[h].begin(); it != qd[h].end(); it++)
		{
			Node* n = *it;

			if (n->m_c.getCount() == 0) continue;

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
				Node tmpNode(m_id, plow, phigh, 0, 1, 1);
				std::set<Node*, Node::PostOrder>::iterator itTmp =
					qd[h + 1].find(&tmpNode);

				if (itTmp != qd[h + 1].end())
				{
					pParent = *itTmp;
					pfound = true;
				}
			}

			Node tmpNode(m_id, slow, shigh, 0, 1, 1);
			std::set<Node*, Node::PostOrder>::iterator itTmp =
				qd[h].find(&tmpNode);

			if (itTmp != qd[h].end())
			{
				pSibling = *itTmp;
				sfound = true;
			}

			Sketches::FM sum(n->m_c);
			if (pfound) sum.merge(pParent->m_c);
			if (sfound) sum.merge(pSibling->m_c);

			if (sum.getCount() < l)
			{
				if (sfound) pSibling->m_c.clear();

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

