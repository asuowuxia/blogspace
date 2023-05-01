// this file has no copyright notice. It was not developed at Aubeta
// $Header: /iQueCenter/servers/src/IWebCommon/arithmetic_types.h 2     5/27/05 4:20p Kurtg $

# if !defined ARITHMETIC_TYPES_H
#	define ARITHMETIC_TYPES_H

/******************************************************************************
	FIXED-POINT ARITHMETIC TEMPLATE LIBRARY					   VERSION 2.1
******************************************************************************/

/******************************************************************************
HELPER FUNCTIONS
	Branch-free arithmetic helper templates used in the remainder computation
	from the book Hacker's Delight, by Henry S. Warren Jr. with thanks.
******************************************************************************/

template <typename R> inline R neg(R r)			//	returns -1 if r < 0 else 0
{ return r < 0 ? -1 : 0; }
template <> inline long neg(long r)				{ return (r >> 31); }
template <> inline long long neg(long long r)	{ return (r >> 63); }
template <> inline unsigned long neg(unsigned long) { return 0; }

template <typename R> inline R sign(R r)		//	return 1 if r >= 0 else -1
{ return neg(r) | 1; }							//{ return r < 0 ? -1 : 1; }

template <typename R> inline R absval(R r)		//	return absolute value of r
{ R t = neg(r); return (r ^ t) - t; }			//{ return r < 0 ? -r : r; }

/******************************************************************************
TEMPLATE CLASS policy_restrict_range
PARAMETERS
	R	--	an integral representation type
******************************************************************************/
template <typename R> struct policy_restrict_range // : public arithmetic_policy_helper
{
	typedef typename R rep_t;
	typedef typename R intermed_t;

	/******************************************************************************
	METHOD scaled_multiplication() -- multiply and scale result
	ARGUMENTS
		f1	--	factor 1
		f2	--	factor 2
		s	--	scale factor
	RETURNS	--	scaled product
	IMPLEMENTATION
		a and b are integer representations of some fixed-point type. The product
		a*b has twice as many significant digits of fraction as the arguments.
		Dividing with rounding by the scale factor yields a result that has the
		same number of fractional digits. Because this method is inlined, the 
		compiler has an opportunity to note that the scale factor may be a power
		of 2, and substitute an appropriately more efficient shoft operator.
	******************************************************************************/
	static rep_t scaled_multiplication(intermed_t f1, intermed_t f2, int s)
	{
		rep_t p = rounded_division(f1 * f2, s);
		return p;
	}
	/******************************************************************************
	METHOD rounded_division() -- divide and round result
	ARGUMENTS
		n	--	numerator
		d	--	denominatorr
	RETURNS	--	rounded quotient
	IMPLEMENTATION
		n and d are integer representations of some fixed-point type. n has already
		been scaled to have twice it's normal precision, because division will
		halve the precision of the result.
	******************************************************************************/
	static rep_t rounded_division(intermed_t n, intermed_t d)
	{
		rep_t q = n / d;
		rep_t r = n % d;
		q += sign(n^d) & neg(absval(d) - (absval(r)<<1) - 1);
		return q;
	}
};// end policy_restrict_range

//	specialization for unsigned long
template <> struct policy_restrict_range<unsigned long>
{
	typedef unsigned long rep_t;
	typedef unsigned long intermed_t;

	static rep_t scaled_multiplication(intermed_t f1, intermed_t f2, int s)
	{
		rep_t p = rounded_division(f1 * f2, s);
		return p;
	}
	static rep_t rounded_division(intermed_t n, intermed_t d)
	{
		rep_t q = n / d;
		rep_t r = n % d;
		q += 1 + neg((long)((r<<1) - d));
		return q;
	}
};// end policy_restrict_range<unsigned long>

/******************************************************************************
CLASS policy_long_intermediate_result
******************************************************************************/
template <typename R> struct policy_long_intermediate_result
{
	typedef typename R rep_t;
	typedef typename long long intermed_t;

	static rep_t scaled_multiplication(intermed_t f1, intermed_t f2, int s)
	{
		rep_t p = rounded_division(f1 * f2, s);
		return p;
	}
	static rep_t rounded_division(intermed_t n, intermed_t d)
	{
		rep_t q = (rep_t)(n / d);
		rep_t r = (rep_t)(n % d);
		q += (rep_t)(sign(n^d) & neg(absval(d) - (absval(r)<<1) - 1));
		return q;
	}
};// end policy_long_intermediate_result

//	specialization for unsigned long
template <> struct policy_long_intermediate_result<unsigned long>
{
	typedef unsigned long rep_t;
	typedef unsigned long long intermed_t;

	static rep_t scaled_multiplication(intermed_t f1, intermed_t f2, int s)
	{
		rep_t p = rounded_division(f1 * f2, s);
		return p;
	}
	static rep_t rounded_division(intermed_t n, intermed_t d)
	{
		rep_t q = rep_t(n / d);
		rep_t r = rep_t(n % d);
		q += 1 + neg((long)((r<<1) - d));
		return q;
	}
};// end policy_long_intermediate_result<unsigned long>

/******************************************************************************
CLASS arithmetic_operator_helper
PARAMETERS
	T	--	arithmetic type like rounded, fixed_decimal, or fixed_binary below
IMPLEMENTATION
	for class T representing a numeric type (a type that acts like a number, 
	having commutative + and *, non-commutative - and /, and comparison 
	relations), and	which implement a basic set of methods 
		operator+=, -=, *=, /=, ==, and <
	arithmetic_operator_helper<T> adds additional operator methods
		operator+, -, *, /, unary-, !=, <=, >, >=
	with the usual meanings
******************************************************************************/
template <typename T> struct arithmetic_operator_helper
{
//	arithmetical operators
	friend T operator-(T const& lhs)
	{ T res;  res-=lhs; return res; }
	friend T operator+(T const& lhs, T const& rhs) 
	{ T res(lhs); res+=rhs; return res; }
	friend T operator-(T const& lhs, T const& rhs) 
	{ T res(lhs); res-=rhs; return res; }
	friend T operator*(T const& lhs, T const& rhs) 
	{ T res(lhs); res*=rhs; return res; }
	friend T operator/(T const& lhs, T const& rhs) 
	{ T res(lhs); res/=rhs; return res; }
	//	friend T operator+(T lhs, T const& rhs)
	//	{ return lhs+=rhs; } //	!named return value optimization version

//	arithmetic with other integral types
	template <typename R> friend T operator+(R lhs, T const& rhs)
	{ T res(lhs); res+=rhs; return res; }
	template <typename R> friend T operator-(R lhs, T const& rhs)
	{ T res(lhs); res-=rhs; return res; }
	template <typename R> friend T operator*(R lhs, T const& rhs)
	{ T res(lhs); res*=rhs; return res; }
	template <typename R> friend T operator/(R lhs, T const& rhs)
	{ T res(lhs); res/=rhs; return res; }

//	comparison operators
	friend bool operator!=(T const& lhs, T const& rhs)
	{ return !(lhs == rhs); }
	friend bool operator>=(T const& lhs, T const& rhs)
	{ return !(lhs <  rhs); }
	friend bool operator<=(T const& lhs, T const& rhs)
	{ return !(rhs <  lhs); }
	friend bool operator> (T const& lhs, T const& rhs)
	{ return  (rhs <  lhs); }
};// end class arithmetic_operator_helper

/******************************************************************************
CLASS exp10 -- template helper for computing powers of 10
ARGUMENTS
	N	--	exponent
IMPLEMENTATION
	exp10 computes powers of 10 recursively using a specialization for 0 to 
	end recursion
******************************************************************************/
template <int N> struct exp10    { enum { value = 10 * exp10<N-1>::value }; };
template <>      struct exp10<0> { enum { value = 1 }; };

# include <iostream>	//	needed for the operator<<() ostream inserters

/******************************************************************************
CLASS rounding --  rounding integer template class
ARGUMENTS
	T	--	underlying numerical representation type
******************************************************************************/
template <typename T=long> class rounding : public arithmetic_operator_helper<rounding<T> >
{
	typedef typename policy_restrict_range<T> P;
	typedef typename policy_restrict_range<T>::rep_t rep_t;
public:
	enum	//	some attributes
	{
		radix_  = 2,
		scale_  = 1,
		fraction_digits_ = 0,
	};

	rounding(int i=0)          : rep_((rep_t)(i))		{}
	rounding(unsigned int u)   : rep_((rep_t)(u))		{} 
	rounding(unsigned long ul) : rep_((rep_t)(ul))		{}
	rounding(double d)         : rep_((rep_t)(d + (d >= 0 ? 0.5 : -0.5)))	{}

//	arithmetical operators
	rounding& operator-()						{ this->rep_ = -this->rep_;	}
	rounding& operator+=(rounding const& that)	{ this->rep_ += that.rep_; return *this; }
	rounding& operator-=(rounding const& that)	{ this->rep_ -= that.rep_; return *this; }
	rounding& operator*=(rounding const& that)	{ this->rep_ *= that.rep_; return *this; }
	rounding& operator/=(rounding const& that)
	{ this->rep_ = P::rounded_division(this->rep_, that.rep_); return *this; }

//	comparison operators
	bool operator==(rounding const& rhs) const	{ return this->rep_ == rhs.rep_; }
	bool operator< (rounding const& rhs) const	{ return this->rep_ <  rhs.rep_; }

//	conversions
	//	get representation
	rep_t characteristic() const			{ return rep_; }
	rep_t fraction() const					{ return 0; }
	rep_t get() const						{ return P::rounded_division(rep_, scale_); }
	rep_t rep() const						{ return rep_; }
	int radix() const						{ return radix_; }
	int scale() const						{ return scale_; }
	int fraction_digits() const				{ return fraction_digits_; }
	rounding& operator=(int i)				{ rep_ =  i; return *this; }
	rounding& operator=(unsigned int ui)	{ rep_ = ui; return *this; }
	rounding& operator=(long l)				{ rep_ =  l; return *this; }
	rounding& operator=(unsigned long ul)	{ rep_ = ul; return *this; }
	rounding& operator=(double d)			{ rep_ = (rep_t)(d + (d >= 0 ? 0.5 : -0.5)); return *this; }

	friend std::ostream& operator<<(std::ostream& o, rounding n)
	{ return o << n.get(); }
private:
	rep_t rep_;
};// end class rounding

/******************************************************************************
CLASS fixed_decimal --  exact decimal arithmetic type
	This fixed decimal representation provides a subset of rational numbers 
	evenly spaced between a maximum and minimum value which facilitates exact 
	decimal computations. 
ARGUMENTS
	N	--	number of decimal places
	T	--	underlying integral representation type
TEMPORARY Things to add to fixed_decimal
		o	accept other sizes of fixed decimal arguments
******************************************************************************/
template <int N=2, typename T=long, template <class> class Policy=policy_long_intermediate_result> 
	struct fixed_decimal : public arithmetic_operator_helper<fixed_decimal<N,T,Policy> >
{
	typedef typename Policy<T> P;
	typedef typename Policy<T>::rep_t rep_t;
	enum	//	some attributes
	{
		radix_  = 10,
		scale_ = exp10<N>::value,
		fraction_digits_ = N,
	};

	fixed_decimal(int m=0, int n=0) : rep_((rep_t)(m*scale() + n%scale())){}
	fixed_decimal(unsigned int ui)  : rep_((rep_t)(ui*scale()))			{} 
	fixed_decimal(unsigned long ul) : rep_((rep_t)(ul*scale()))			{}
	fixed_decimal(double d)         : rep_((rep_t)( d*scale() + (d >= 0 ? 0.5 : -0.5)))	{}

//	arithmetical operators
	fixed_decimal& operator-()								{ this->rep_ = -this->rep_;	}
	fixed_decimal& operator+=(fixed_decimal const& that)	{ this->rep_ += that.rep_; return *this; }
	fixed_decimal& operator-=(fixed_decimal const& that)	{ this->rep_ -= that.rep_; return *this; }
	fixed_decimal& operator*=(fixed_decimal const& that)
	{ this->rep_ = P::scaled_multiplication(this->rep_, that.rep_, scale_); return *this; }
	fixed_decimal& operator/=(fixed_decimal const& that)
	{ this->rep_ = P::rounded_division(this->rep_ * scale_, that.rep_); return *this; }

//	comparison operators
	bool operator==(fixed_decimal const& rhs) const			{ return this->rep_ == rhs.rep_; }
	bool operator< (fixed_decimal const& rhs) const			{ return this->rep_ <  rhs.rep_; }

//	conversions
	rep_t characteristic() const				{ return rep_ / scale(); }
	rep_t fraction() const						{ return rep_ % scale(); }
	rep_t get() const							{ return P::rounded_division(rep_, scale()); }
	rep_t rep() const							{ return rep_; }
	int radix() const							{ return radix_; }
	int scale() const							{ return scale_; }
	int fraction_digits() const					{ return fraction_digits_; }
	fixed_decimal& operator=(int i)				{ rep_ = (rep_t)( i*scale()); return *this; }
	fixed_decimal& operator=(unsigned int ui)	{ rep_ = (rep_t)(ui*scale()); return *this; }
	fixed_decimal& operator=(long l)			{ rep_ = (rep_t)( l*scale()); return *this; }
	fixed_decimal& operator=(unsigned long ul)	{ rep_ = (rep_t)(ul*scale()); return *this; }
	fixed_decimal& operator=(double d)			{ rep_ = (rep_t)(d*scale() + 0.5); return *this; }
private:
	rep_t rep_;
};// end template class fixed_decimal

template <int N, typename T, typename P> std::ostream& operator<<(std::ostream& o, fixed_decimal<N,T,P> n)
{ 
	o << n.characteristic() << '.';
	T f = absval(n.fraction()); 
	for (int i = n.scale()/10; i > 0; f %= i, i /= 10)
		o << (char)('0' + f / i);
	return o; 
}//	end operator<<

/******************************************************************************
CLASS fixed_binary --  fixed point arithmetic type
	The fixed point representation provides a subset of rational numbers evenly
	spaced between a maximum and minimum value. By contrast, floating or rational
	representation values cluster tightly around zero, growing increasingly
	sparse as you approach their limits. Fixed point representation thus preserves
	accuracy throughout its range.
ARGUMENTS
	N	--	number of binary bits
	T	--	underlying integral representation type
	
TEMPORARY some options to consider for fixed point
		o	whether to use a long intermediate result or perform additional 
			multiplications to use short intermediate results in multiplication
		o	whether to saturate, throw, or wrap on overflow
		o	conversions from other instantiations of fixed_binary 
		o	range checking
******************************************************************************/
template <int N=7, typename T=long, template <class> class Policy=policy_long_intermediate_result> 
	struct fixed_binary : public arithmetic_operator_helper<fixed_binary<N, T, Policy> >
{
	typedef typename Policy<T> P;
	typedef typename Policy<T>::rep_t rep_t;
	enum	//	some attributes
	{
		radix_  = 2,
		scale_  = 1<<N,
		fraction_digits_ = N
	};
	fixed_binary(int m=0, int n=0) : rep_((rep_t)( m << fraction_digits())+(n & (scale()-1))){}
	fixed_binary(unsigned int  ui) : rep_((rep_t)(ui << fraction_digits()))					{} 
	fixed_binary(unsigned long ul) : rep_((rep_t)(ul << fraction_digits()))					{}
	fixed_binary(double d)         : rep_((rep_t)(d*scale_ + (d >= 0 ? 0.5 : -0.5)))		{}

//	arithmetical operators
	fixed_binary& operator-()							{ this->rep_ = -this->rep_;	}
	fixed_binary& operator+=(fixed_binary const& that)	{ this->rep_ += that.rep_; return *this; }
	fixed_binary& operator-=(fixed_binary const& that)	{ this->rep_ -= that.rep_; return *this; }
	fixed_binary& operator*=(fixed_binary const& that)
	{ this->rep_ = P::scaled_multiplication(this->rep_, that.rep_, scale_); return *this; }
	fixed_binary& operator/=(fixed_binary const& that)
	{ this->rep_ = P::rounded_division(static_cast<P::intermed_t>(this->rep_) << N, that.rep_); return *this; }

//	comparison operators
	bool operator==(fixed_binary const& rhs) const { return this->rep_ == rhs.rep_; }
	bool operator< (fixed_binary const& rhs) const { return this->rep_ <  rhs.rep_; }

//	conversions
	//	get representation
	rep_t characteristic() const			{ return rep_ >> fraction_digits(); }
	rep_t fraction() const					{ return rep_ & (scale()-1); }
	rep_t get() const						{ return P::rounded_division(rep_, scale()); }
	rep_t rep() const						{ return rep_; }
	int radix() const						{ return radix_; }
	int scale() const						{ return scale_; }
	int fraction_digits() const				{ return fraction_digits_; }
	fixed_binary& operator=(int i)			{ rep_ = (rep_t)( i << fraction_digits()); return *this; }
	fixed_binary& operator=(unsigned int ui){ rep_ = (rep_t)(ui << fraction_digits()); return *this; }
	fixed_binary& operator=(long l)			{ rep_ = (rep_t)( l << fraction_digits()); return *this; }
	fixed_binary& operator=(unsigned long ul){rep_ = (rep_t)(ul << fraction_digits()); return *this; }
	fixed_binary& operator=(double d)		{ rep_ = (rep_t)(d*scale() + (d >= 0 ? 0.5 : -0.5)); return *this; }
private:
	rep_t rep_;
};// end template class fixed_binary

template <int N, typename T, typename P> std::ostream& operator<<(std::ostream& o, fixed_binary<N,T,P> n)
{ 
	int scale = exp10<(N > 3 ? 3 : N)>::value;
	T f = fixed_binary<N,T,P>::P::rounded_division(absval(n.fraction()) * scale, n.scale());
	o << n.characteristic() << '.';
	for (int i = scale/10; i > 0; f %= i, i /= 10)
		o << (char)('0' + f / i);
	return o; 
}//	end operator<<

int arithmetic_types_module_test(int test_no, std::ostream& os);

# endif // !defined ARITHMETIC_TYPES_H
