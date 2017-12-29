class myDerivedClassTest {
    public static void main(String[] a){
        System.out.println(new MyTest().Run());
    }
}

class MyTest {
    MyBase a;
    MyDerived b;
    public int Run() {
        int t;

        a = new MyBase();
        t = a.SetX();
        t = a.SetY();
        t = a.Print();

        a = new MyDerived();
        t = a.SetX();
        t = a.SetY();
        t = a.Print();

        b = new MyDerived();
        t = b.SetX();
        t = b.SetY();
        t = b.SetZ();
        t = b.Print();

        return 0;
    }
}

class MyBase {
    int x;
    int y;
    public int SetX() {
        x = 1;
        return 0;
    }
    public int SetY() {
        y = 2;
        return 0;
    }
    public int Print() {
	System.out.println(x);
	System.out.println(y);
        return 0;
    }
}

class MyDerived extends MyBase {
    int z;
    public int Print() {
	System.out.println(x);
	System.out.println(y);
        System.out.println(z);
        return 0;
    }
    public int SetZ() {
        z = 4;
        return 0;
    }
    public int SetY() {
        y = 3;
        return 0;
    }
}
