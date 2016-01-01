import java.io.IOException;

class Exec {
  public static void main(String[] args) {
    // Zdefiniowanie argumentów i uruchomienie procesu.
    String[] arguments = {
      "C:\\Windows\\System32\\notepad.exe",
      "C:\\Windows\\Temp\\this_file_will_be_here_forever.txt"
    };
    Process p;
    try {
      p = Runtime.getRuntime().exec(arguments);
    } catch (IOException ex) {
      System.err.println("Failed to create new process (not Windows?).");
      return;
    }
    System.out.println("Process started. Waiting for it to exit.");

    // Oczekiwanie na zakończenie procesu. 
    try {
      p.waitFor();
    } catch (InterruptedException ex) {
      // Ten wyjątek jest rzucany jesli inny wątek przerwie oczekiwanie
      // niniejszego watku. To sie nie powinno nigdy zdarzyć, ponieważ
      // niniejsza przykładowa aplikacja posiada jedynie jeden watek.
      // Niemniej jednak kompilator Java wymaga aby przerwanie zostało 
      // złapane.
    }
    System.out.println("The child process has exited.");
  }
}

