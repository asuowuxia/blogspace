#ifndef __QUOTE_H__
#define __QUOTE_H__

#include <iostream>
#include <memory>

class Quote{
 public:
  Quote() = default;
  Quote(const std::string& book, double pri)
    :bookNo(book), price(pri){}
  std::string isbn() const{return bookNo;}
  virtual double net_price(std::size_t n)const{
    return n * price;
  }
  virtual void debug()const{
    std::cout << bookNo << " " << price << std::endl;
  }
  virtual ~Quote() = default;
 private:
  std::string bookNo;
 protected:
  double price = 0.0;
};

class Bulk_quote : public Quote{
 public:
  Bulk_quote() = default;
  Bulk_quote(const std::string&, double, std::size_t,
	     double);
  double net_price(std::size_t n)const override;
  void debug()const override;
 private:
  std::size_t min_qty = 0;//适用于折扣的最低购买数量
  double discount = 0.0;//折扣额
};

class Min_quote : public Quote{
 public:
  Min_quote() = default;
  Min_quote(const std::string&, double, std::size_t,
	     double);
  double net_price(std::size_t n)const override;
  void debug()const override;
 private:
  std::size_t max_qty = 10;//适用于折扣的最高购买数量
  double discount = 0.1;//折扣额
};

#endif
