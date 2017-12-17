public class Loop {
  public static int c = 5, a = 2;

  public static int main(String[] args) {
    int i = 0;
    for (; i < c; ++i) {
      i += a;
    }

    return i;
  }
}
