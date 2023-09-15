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
	if (argc != 7)
	{
		std::cerr
			<< std::endl
			<< "Usage: FM " 
			<< "stream_length domain_size skew bits bitvectors loops.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distributions.\n"
			<< "  bits:          The number of bits per bitvector.\n"
			<< "  bitvectors:    The number of bitvectors per sketch.\n"
			<< "  loops:         The number of individual experiments to run.\n"
			<< std::endl;
		return -1;
	}

	unsigned long N = atol(argv[1]);
	unsigned long domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	unsigned long bits = atol(argv[4]);
	unsigned long bitvectors = atol(argv[5]);
	unsigned long loops = atol(argv[6]);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalErrorU = 0.0, totalErrorS = 0.0;
	double minU = std::numeric_limits<double>::max();
	double minS = std::numeric_limits<double>::max();
	double maxU = 0.0, maxS = 0.0;
	unsigned long total = 0;
	unsigned long seed = time(0);

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		for (unsigned long cLoop = 0; cLoop < loops; cLoop++)
		{
			Tools::Random r(seed + cLoop);
			Tools::PRGZipf zipf(0L, domainSize, skew, &r);

			Sketches::FM fmU(bits, bitvectors, Sketches::HT_UNIVERSAL);
			Sketches::FM fmS(bits, bitvectors, Sketches::HT_SHA1);
			std::set<unsigned long> exact;

			for (unsigned long i = 1; i <= N; i++)
			{
				unsigned long l = zipf.nextLong();
				std::ostringstream ss;
				ss << l << std::flush;

				fmU.insert(l, 2);
				fmS.insert(ss.str(), 2);
				exact.insert(l);
			}

			double errorU = std::abs(static_cast<double>(exact.size() * 2) - static_cast<double>(fmU.getCount())) / static_cast<double>(exact.size() * 2);
			double errorS = std::abs(static_cast<double>(exact.size() * 2) - static_cast<double>(fmS.getCount())) / static_cast<double>(exact.size() * 2);

			minU = std::min(minU, errorU);
			minS = std::min(minS, errorS);
			maxU = std::max(maxU, errorU);
			maxS = std::max(maxS, errorS);
			totalErrorU += errorU;
			totalErrorS += errorS;
			total++;

			std::cerr
				<< "Loop: " << cLoop << ", "
				<< "Exact distinct: " << exact.size() << ", "
				<< "FM distinct (U | S): "
				<< fmU.getCount() << " | " << fmS.getCount() << ", "
				<< "Relative error: " << errorU << " | " << errorS
				<< std::endl;
		}

		std::cerr
			<< "Average relative error (U | S): "
			<< totalErrorU / total << " | " << totalErrorS / total << std::endl
			<< "Minimum error: " << minU << " | " << minS << std::endl
			<< "Maximum error: " << maxU << " | " << maxS << std::endl;

		Tools::Random r;
		Tools::PRGZipf zipf(0L, domainSize, skew, &r);

		Sketches::FM fmU(bits, bitvectors, Sketches::HT_UNIVERSAL);
		Sketches::FM fmS(bits, bitvectors, Sketches::HT_SHA1);

		for (unsigned long i = 1; i <= N; i++)
		{
			unsigned long l = zipf.nextLong();
			std::ostringstream ss;
			ss << l << std::flush;

			fmU.insert(l);
			fmS.insert(ss.str());
		}

		std::cerr << "Testing getData operations." << std::endl;

		unsigned long len, len2;
		byte *data, *data2;

		fmS.getData(&data, len);
		Sketches::FM fmC(data);
		fmC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		fmU.getData(&data, len);
		Sketches::FM fmC2(data);
		fmC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing copy constructor." << std::endl;

		Sketches::FM fmC3(fmU);
		fmU.getData(&data, len);
		fmC3.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The copy constructor does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator<<." << std::endl;

		std::cerr << fmU << std::endl;
		std::cerr << fmC3 << std::endl;

		std::cerr << "Testing the clear operation." << std::endl;
		fmC3.clear();
		std::cerr << fmC3 << std::endl;

		std::cerr << "Testing the merge operation." << std::endl;
		fmC3.merge(fmU);

		fmU.getData(&data, len);
		fmC3.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The merge operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;

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

