class Factorial{
    public static void main(String[] a){
	System.out.println(new Fac().ComputeFac(10));
    }
}

class Fac {
    int x;
    int x; // 重复的类成员变量

    public int ComputeFac(int num){
	int y;
	int y; // 重复的局部变量
	int num; // 局部变量与参数重复
	int num_aux ;
	if (num < 1)
	    num_aux = 1 ;
	else 
	    num_aux = num * (this.ComputeFac(num-1)) ;
	return num_aux ;
    }

    public int ComputeFac(int num){ // 重复的类方法
	return 0;
    }

}

class Fac { // 重复的类

}
