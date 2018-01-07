class MyMain {
    public static void main(String[] a){
	System.out.println(new MyTest().Run2()); // 调用未定义的方法
    }
}

class MyTest {
    public int Run(){
	int x;
	Haha z;
	x = z.test(); // 引用未定义的类中的方法
	return y; // 引用未定义的变量
    }
}
