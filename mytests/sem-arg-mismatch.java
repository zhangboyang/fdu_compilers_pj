class MyMain {
    public static void main(String[] a){
	System.out.println(new MyTest().Run(100)); // 参数数目不正确
    }
}

class MyTest {
    public int Run(){
	return this.Test(true); // 参数类型不正确
    }
    public int Test(int x){
	return 0;
    }
}
