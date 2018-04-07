public class SimpleNew {
  public static int ca = 15;
  public static int cb = 20;

  public int a, b;

  public SimpleNew(int aa, int bb) {
    a = aa;
    b = bb;
  }

  public static int main(String[] args) {
    SimpleNew a = new SimpleNew(5, 10);
    return a.a + a.b + ca + cb;
  }
}
