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

int main(int argc, char** argv)
{
	if (argc != 6)
	{
		std::cerr
			<< "Usage: QuantileDigest stream_length domain_size skew k loops.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distributions.\n"
			<< "  k:             Compression parameter.\n"
			<< "  loops:         The number of individual experiments to run.\n"
			<< std::endl;
		return -1;

		return -1;
	}

	unsigned long N = atol(argv[1]);
	unsigned long domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	unsigned long k = atol(argv[4]);
	unsigned long loops = atol(argv[5]);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalerror = 0.0;
	double totalerror2 = 0.0;
	unsigned long seed = time(0);
	std::map<long, unsigned long> exact;

	try
	{
		for (unsigned long cLoop = 0; cLoop < loops; cLoop++)
		{
			std::cerr << "Loop " << cLoop << ": ";

			Tools::Random r(seed + cLoop);
			Tools::PRGZipf zipf(0L, domainSize, skew, &r);

			exact.clear();
			for (unsigned long i = 0; i < N; i++)
			{
				unsigned long l = zipf.nextLong();

				if (exact.find(l) == exact.end())
					exact.insert(std::pair<long, unsigned long>(l, 1));
				else
					exact[l]++;
			}

			Sketches::QuantileDigest qd(k, 0, domainSize, exact);

			long value;
			unsigned long rank;
			qd.getQuantile(0.5, value, rank);

			double e = static_cast<double>(std::abs(static_cast<long>(rank) - static_cast<long>(N * 0.5))) / static_cast<double>(N * 0.5);
			totalerror += e;
			totalerror2 += std::pow(e, 2.0);
			std::cerr << e << std::endl;
		}

		std::cerr
			<< "Average relative error: "
			<< totalerror / loops << std::endl
			<< "Standard deviation: "
			<<	std::sqrt((loops * totalerror2 - std::pow(totalerror, 2.0)) /
				(loops * (loops - 1.0)))
			<< std::endl;

		Sketches::QuantileDigest qd(k, 0, domainSize, exact);

		std::cerr << "Testing getData operations." << std::endl;

		unsigned long len, len2;
		byte *data, *data2;

		qd.getData(&data, len);
		Sketches::QuantileDigest qdC(data);
		qdC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing copy constructor." << std::endl;

		Sketches::QuantileDigest qdC2(qd);
		qd.getData(&data, len);
		qdC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The copy constructor does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator =." << std::endl;

		qdC2 = qd;
		qd.getData(&data, len);
		qdC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The = operator does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cerr << "Error: Unknown exception caught." << std::endl;
		return -1;
	}

	return 0;
}

