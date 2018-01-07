class MyMain {
    public static void main(String[] a){
	System.out.println(new MyDerived().Run());
    }
}

class MyBase {
    public int Run(){
	return 0;
    }
}

class MyDerived extends MyBase {
    public int Run(int x){ // 与基类的函数签名不一致
	return x;
    }
}
