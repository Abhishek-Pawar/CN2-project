#include <iostream>
#include <thread>
class A{
  public:
    inline static thread_local long l ;
    int value ;
    A(int a = 2){
       value = a;     
    }
    int foo(){
       A::l = 3;
       return 3;
    }
};
A a;
int main(){
    // A::l = 3;
     a.foo();
    return 0;
}