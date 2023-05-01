// this file has no copyright notice. It was not developed at Aubeta
// $Header: /iQueCenter/servers/src/IWebCommon/arithmetic_types.cpp 2     5/27/05 4:20p Kurtg $

# include <afx.h>				//	timing functions in windows
# define DIVIDE_POLICY_MOD		//	define to use C++ operator% to get remainder
//# define NO_ROUNDING			//	define to use branching code for rounding
# define TEST_GRANULE 100		//	count interations totaling at least this many mS
# include "arithmetic_types.h"	//	arithmetric_types.h uses these #defines

/******************************************************************************
ARITHMETIC TYPES LIBRARY METHODS
******************************************************************************/

/******************************************************************************
ARITHMETIC TYPES TEST HARNESS
******************************************************************************/

//	display numerator, demonimator, quotient, remainder, and rounding
static int ModulusTest(int n, int d, int& q, int& r, std::ostream& os)
{
	q = n / d;
	r = n % d;
	int r1, r2;
	r1 = neg(abs(d) - (abs(r)<<1) - 1);	// -1 or  0
	r2 = sign(n^d);						//	1 or -1
	os << "    " << n << '/' << d << " = " << q << " R " << r << " round " << (r1 & r2) << std::endl;
	return r1&r2;	//	return rounding
}// end ModulusTest

/******************************************************************************
CLASS numeric --  wrapped numeric class
ARGUMENTS
	T	--	underlying numerical representation type
IMPLEMENTATION
	numeric is a wrapper around a built-in type to make it possible to run a
	test written as a template. This is for comparing float or double to my
	fixed point types
******************************************************************************/
template <typename T=int> class numeric : public arithmetic_operator_helper<numeric<T> >
{
public:
	typedef T rep_t;

	numeric(int  i=0) : rep_((T)(i))	{}
	numeric(double f) : rep_((T)(f))	{}

//	arithmetical operators
	numeric& operator+=(numeric const& that)	{ this->rep_ += that.rep_; return *this; }
	numeric& operator-=(numeric const& that)	{ this->rep_ -= that.rep_; return *this; }
	numeric& operator*=(numeric const& that)	{ this->rep_ *= that.rep_; return *this; }
	numeric& operator/=(numeric const& that)	{ this->rep_ /= that.rep_; return *this; }

//	comparison operators
	bool operator==(numeric const& rhs) const	{ return this->rep_ == rhs.rep_; }
	bool operator< (numeric const& rhs) const	{ return this->rep_ <  rhs.rep_; }

	friend std::ostream& operator<<(std::ostream& o, numeric n)
	{ o << n.rep(); return o; }

//	conversions
	//	get representation
	rep_t rep()									{ return rep_; }
	numeric& operator=(int i)					{ rep_ = i; return *this; }
private:
	rep_t rep_;
};// end class numeric

// template <typename T> std::ostream& operator<<(std::ostream& o, numeric<T> n)
// { o << n.rep(); return o; }

//	perform "typical" computation involving addition, multiply, divide; log results
template <typename T> void Computation1a(char* p, std::ostream& os)
{
	T res, g(9.8), v0(5), d0(50);
	os << p << ": typical computation 1: a=" << g << " m/sec^2, v0=" << v0 << " m/sec, d0=" << d0 << "m" << std::endl;

	for (T t = 0; t < 10; t = t + 1)
	{
		res = g * t * t / 2 + v0 * t + d0;
		os	<< "    time " << t << ", distance " << res << ", rep " << res.rep() << std::endl;
	}
}// end Computation1a

//	perform "typical" computation involving addition, multiply, divide; measure rate over many iterations
template <typename T> DWORD Computation1b(char* p, int iterations, std::ostream& os)
{
	T res, g(9.8), v0(5), d0(50);
	DWORD tick = ::GetTickCount();
	for (int j = 1; j < iterations; ++j)
		for (T t = 0; t < 10; t = t + 1)
		{
			res = g * t * t / 2 + v0 * t + d0;
		}
	tick = ::GetTickCount() - tick;
	if (tick >= TEST_GRANULE)
		os << p << iterations << " iterations in " << tick << "mS" << std::endl;
	return tick;
}// end Computation1b

//	 test rounding algorithm by performing typical rounding computation
template <typename T> void RoundingTest(char* p, std::ostream& os)
{
	os << p << ": rounding test" << std::endl;
	os << "      5/3 rounds to " << T(5)/3   << std::endl;
	os << "     -5/3 rounds to " << T(-5)/3  << std::endl;
	os << "     5/-3 rounds to " << T(5)/-3  << std::endl;
	os << "    -5/-3 rounds to " << T(-5)/-3 << std::endl;
}// end RoundingTest

//	show usable magnitude of representation by hunting for overflow
template <typename T> void OverflowTest(char* p, std::ostream& os)
{
	os << p << " overflow test" << std::endl;
	for (int i=1; i>0; i=(i<<1)+1)
	{
		T t(234.567);
		T res = t * i;
		os << "    " << t << " * " << i << " = " << res << std::endl;
		if (sign(res.rep()) == -1) 
			break;
	}
}// end OverflowTest

//	test driver
int arithmetic_types_module_test(int nTest, std::ostream& os)
{
	bool rc = true;
	static DWORD fptick = 0, fpiterations = 10000000, tick;
	char* p;
	switch(nTest)
	{
	case 1:	//	test support functions used in the branch-free rounding algorithm
		{
			os << "\ntest " << nTest << ": testing support functions" << std::endl;
			for (int i = -3; i <= 3; i+=1)
				os	<<     "neg (" << i << ") = " << neg(i) << ", " 
					<<     "abs (" << i << ") = " << absval(i) << ", " 
					<<     "sign(" << i << ") = " << sign(i) << std::endl;
		}
		break;

	case 2:	/*	integer modulus sign test
				I have heard that the integer division instruction does not
				compute the sign of the remainder the same on every processor.
				This test passes if the remainder's sign is computed the same 
				way as on a PC. If this test fails, the implementation must be
				adjusted for the non-PC processor you're testing on.

				This case also tests the rounding computation.
			*/
		{
			os << "\ntest " << nTest << ": testing integer modulus operator" << std::endl;
			int n = 5, d = 3, q, r;
			ModulusTest( n, d, q, r, os);
			rc &= ( q >= 0 && r >= 0);
			ModulusTest(-n, d, q, r, os);
			rc &= ( q <  0 && r <  0);
			ModulusTest( n,-d, q, r, os);
			rc &= ( q <  0 && r >= 0);
			ModulusTest(-n,-d, q, r, os);
			rc &= ( q >= 0 && r <  0);
			if (rc)
				 os << "modulus has same sign as on PC. implementation ok" << std::endl;
			else os << "modulus has different sign. implementation not ok" << std::endl;
		}
		break;

	case 13:
		{
			int q, r;
			os << "\ntest " << nTest << ": testing rounding computation" << std::endl;
			for (int i=-20; i < 20; i++)
			{
				if ((i >= -19 && i <= -15) ||
					(i >=  -9 && i <=  -5) ||
					(i >=   5 && i <=   9) ||
					(i >=  15 && i <=  19))
					 rc &= (ModulusTest(i, 10, q, r, os) != 0);
				else rc &= (ModulusTest(i, 10, q, r, os) == 0);
			}
		}
		break;

	case 3:	//	typical computation using double, for comparison
		{	
			p = "numeric<double> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <numeric<double> >(p, os);
			Computation1a<numeric<double> >(p, os);
		//	search for number of fp iterations that takes at least TEST_GRANULE mS
			for (fpiterations = 10000; 
				 (fptick = Computation1b<numeric<double> >(p, fpiterations, os)) <= TEST_GRANULE; 
				 fpiterations*=10)
				/* empty */;
		}
		break;

	case 4:	//	typical computation using float
		{	
			p = "numeric<float> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <numeric<float> >(p, os);
			Computation1a<numeric<float> >(p, os);
			Computation1b<numeric<float> >(p, fpiterations, os);
		}
		break;

	case 5:	//	typical computation using int
		{
			p = "numeric<int> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <numeric<int> >(p, os);
			Computation1a<numeric<int> >(p, os);
			Computation1b<numeric<int> >(p, fpiterations, os);
		}
		break;

	case 6:	//	typical computation using short
			//	turns out the same as int because values are promoted to int in computation
		{
			p = "numeric<short> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <numeric<short> >(p, os);
			Computation1a<numeric<short> >(p, os);
			Computation1b<numeric<short> >(p, fpiterations, os);
		}
		break;

	case 7:	//	typical computation using rounded integer
		{
			p = "rounding<int> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <rounding<int> >(p, os);
			Computation1a<rounding<int> >(p, os);
			Computation1b<rounding<int> >(p, fpiterations, os);
			OverflowTest <rounding<int> >(p, os);
		}
		break;

	case 8:	//	typical computation using fixed binary
		{
			static const int bits = 10;
			p = "fixed_binary<10> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <fixed_binary<bits> >(p, os);
			Computation1a<fixed_binary<bits> >(p, os);
			Computation1b<fixed_binary<bits> >(p, fpiterations, os);
			OverflowTest <fixed_binary<bits> >(p, os);
		}
		break;

	case 12:	//	typical computation using fixed binary with a different policy
		{
			static const int bits = 10;
			p = "fixed_binary<10, long, policy_restrict_range> ";
			typedef fixed_binary<bits, long, policy_restrict_range> fixb;
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <fixb>(p, os);
			Computation1a<fixb>(p, os);
			Computation1b<fixb>(p, fpiterations, os);
			OverflowTest <fixb>(p, os);
		}
		break;

	case 15://	typical computation using  unsigned fixed binary
		{
			static const int bits = 10;
			p = "unsigned_fixed_binary<10> ";
			typedef fixed_binary<bits,unsigned long> fixb;
			os << "\ntest " << nTest << ": " << p << std::endl;
		//	RoundingTest <fixb>(p, os);
			os << p << ": rounding test" << std::endl;
			os << "      5/3 rounds to " << fixb(5)/3   << std::endl;
			Computation1a<fixb>(p, os);
			Computation1b<fixb>(p, fpiterations, os);
			OverflowTest <fixb>(p, os);
		}
		break;

	case 9:	//	typical computation using fixed decimal
		{
			p = "fixed_decimal<3> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <fixed_decimal<3> >(p, os);
			Computation1a<fixed_decimal<3> >(p, os);
			Computation1b<fixed_decimal<3> >(p, fpiterations, os);
			OverflowTest <fixed_decimal<3> >(p, os);
		}
		break;

	case 11:	//	typical computation using fixed decimal with a different policy
		{
			typedef fixed_decimal<3, long, policy_restrict_range> fixd;
			p = "fixed_decimal<3, long, policy_restrict_range> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
			RoundingTest <fixd>(p, os);
			Computation1a<fixd>(p, os);
			Computation1b<fixd>(p, fpiterations, os);
			OverflowTest <fixd>(p, os);
		}
		break;

	case 14://	typical computation using  unsigned fixed decimal
		{
			typedef fixed_decimal<3, unsigned long> fixd;
			p = "fixed_decimal<3, unsigned long> ";
			os << "\ntest " << nTest << ": " << p << std::endl;
		//	RoundingTest <fixd>(p, os);
			os << p << ": rounding test" << std::endl;
			os << "      5/3 rounds to " << fixd(5)/3   << std::endl;
			Computation1a<fixd>(p, os);
			Computation1b<fixd>(p, fpiterations, os);
			OverflowTest <fixd>(p, os);
		}
		break;

	case 10://	type conversion. This really tests by compiling
		{
			typedef fixed_binary<3,long> T;
			T res(1.2);
			res = res + ((short)1);
			rc &= (res == T(2.2));
			res = res + ((int)  2);
			rc &= (res == T(4.2));
			res = res + ((long) 3);
			rc &= (res == T(7.2));
			res = res + ((unsigned short)4);
			rc &= (res == T(11.2));
			res = res + ((unsigned int)  5);
			rc &= (res == T(16.2));
			res = res + ((unsigned long) 6);
			rc &= (res == T(22.2));
			res = res + ((double)      7.8);
			rc &= (res == T(30));

			res = 1.2;
			res = ((short)1)          + res;
			rc &= (res == T(2.2));
			res = ((int)  2)          + res;
			rc &= (res == T(4.2));
			res = ((long) 3)          + res;
			rc &= (res == T(7.2));
			res = ((unsigned short)4) + res;
			rc &= (res == T(11.2));
			res = ((unsigned int)  5) + res;
			rc &= (res == T(16.2));
			res = ((unsigned long) 6) + res;
			rc &= (res == T(22.2));
			res = ((double)      7.8) + res;	//	converts from double to int
			rc &= (res == T(30));
		}
		break;

	case 100://	run a convenient bunch of tests
		{
			os << CTime::GetCurrentTime().Format("test arithmetic types on %c") << std::endl;
		//	rc &= (arithmetic_types_module_test(1, os) == 1);	//	support function test
		//	rc &= (arithmetic_types_module_test(2, os) == 1);	//	integer modulus test
		//	rc &= (arithmetic_types_module_test(13,os) == 1);	//	rounding test
			rc &= (arithmetic_types_module_test(3, os) == 1);	//	double test
		//	rc &= (arithmetic_types_module_test(4, os) == 1);	//	float takes 10x longer than double
			rc &= (arithmetic_types_module_test(5, os) == 1);	//	int test
		//	rc &= (arithmetic_types_module_test(6, os) == 1);	//	short takes as long as int
			rc &= (arithmetic_types_module_test(7, os) == 1);	//	rounding integer
			rc &= (arithmetic_types_module_test(8, os) == 1);	//	fixed binary
			rc &= (arithmetic_types_module_test(12,os) == 1);	//	fixed binary restricted range
			rc &= (arithmetic_types_module_test(15,os) == 1);	//	fixed binary unsigned
			rc &= (arithmetic_types_module_test(9, os) == 1);	//	fixed decimal
			rc &= (arithmetic_types_module_test(11,os) == 1);	//	fixed decimal restricted range
			rc &= (arithmetic_types_module_test(14,os) == 1);	//	fixed decimal unsigned
		//	rc &= (arithmetic_types_module_test(10,os) == 1);	//	data type conversion test
		}
		break;

	case 0: return 13;
	default: return -1;
	}
	return rc ? 1 : 0;
}// end arithmetic_types_module_test

/*	Things I learned that require documentation
	 o	There is no performance improvement by making the base type short int.
		This is almostly certainly because arithmetic is widened to int.
	 o	On pentium 4 processors, double is far faster than float.
	 o	On pentium 4 processors, there is no performance degradation by using
		wide intermediate results. This implies the ALU is 64 bits wide.

	Things to add
	 o	Can I templatize rounded_division() and move it to a common base of
		the policy class? I could use the intermediate type to cast the args
		to select the right template.
*/