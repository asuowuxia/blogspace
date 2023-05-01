#include <iostream>
#include <string>
#include <future>

class A{
  int data;
public:
  A(int d = 10) : data(d){}
  int _data() const {return data;}
};
double div1(double a, double b){
  if(b == 0){
    //throw std::string("error");//进入string的catch
    //throw "error";//进入const char*的catch
    //throw 1;//进入int的catch
    throw A(101);//进入A a的catch
    
  }
  return a / b;
}

double div2(std::promise<double>& pro, double a, double b){
    int x;
    std::cin.exceptions (std::ios::failbit);   //如果不加这句，std::cin >> x这里，即使给的不是数字，也不会发生异常。
    try{
      std::cin >> x;
    }catch(std::exception&){
      pro.set_exception(std::current_exception());
    }

}
int main(){
  try{

    //std::asnyc
    std::future<double> f = std::async(div1, 10, 0);
    std::cout << f.get() << std::endl;

    //std::package_task
    std::packaged_task<double(double, double)> t(div1);
    std::future<double> f2 = t.get_future();
    std::thread thread1(std::ref(t), 100, 0);
    thread1.detach();
    f2.get();

    //std::promise
    std::promise<double> pro;
    std::future<double> f3 = pro.get_future();
    std::thread thread2(div2, std::ref(pro), 100, 0);
    thread2.join();
    f3.get();//进入catch(...)部分
    
  }
  catch(A a){
    std::cout << "err:A a" << std::endl;
    std::cout << a._data() << std::endl;
  }
  catch(int a){
    std::cout << "err:int" << std::endl;
    std::cout << a << std::endl;
  }
  catch(const char* s){
    std::cout << "err:char*" << std::endl;
    std::cout << s << std::endl;
  }
  catch(std::string s){
    std::cout << "err:string" << std::endl;
    std::cout << s << std::endl;
  }
  catch(...){
    using namespace std;
    cout << "...." << endl;
    
  }
}
