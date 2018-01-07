class MyMain {
    public static void main(String[] a){
	System.out.println(new MyTest().Run());
    }
}

class MyTest {
    public int Run(){
	int x;
	x = x % 100;
	return x;
    }
}
