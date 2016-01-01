import java.io.*;

class stdin {
  public static void main(String[] args) {
    InputStreamReader cin = new InputStreamReader(System.in);
    BufferedReader br = new BufferedReader(cin);
    try {
      System.out.println(br.readLine());
    } catch(IOException x) {
      // Obsługa błędów.
    }
  }
}
