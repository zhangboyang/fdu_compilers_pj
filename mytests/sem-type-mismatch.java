class MyMain {
    public static void main(String[] a){
	System.out.println(new MyTest().Run());
    }
}

class MyTest {
    public int Run(){
	int x;
	x = true; // 将布尔值赋给整型变量
	return x;
    }
}
