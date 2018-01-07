class MyMain {
    public static void main(String[] a){
	System.out.println(new MyTest().Run());
    }
}

class MyTest {
    public int Run(){
	int x;
	return x.test(); // 对非类变量使用成员函数
    }
}
