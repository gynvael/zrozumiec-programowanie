import java.util.*;

public class StressMapFixed {
  public static TreeMap<String, Integer> dict;
  public static final String[] KEYS = {
    "k1", "k2", "k3", "k4", "k5", "k6"    
  };

  public static void main(String args[]) {
    dict = new TreeMap<String, Integer>();

    // Korzystając z możliwości stworzenia klasy dziedziczącej "w miejscu"
    // definiuje klasę-wątek Purger oraz Adder.
    Thread purger = new Thread() {
      public void run() {
        int poor_mans_random = 647;
        for (;;) {
          synchronized(dict) {  // Zajęcie muteksu.
            dict.remove(KEYS[poor_mans_random % 6]);
          }  // Zwolnienie muteksu.
          poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
        }
      }
    };

    Thread adder = new Thread() {
      public void run() {
        int poor_mans_random = 499;
        for (;;) {
          synchronized(dict) {  // Zajęcie muteksu.
            dict.put(KEYS[poor_mans_random % 6], poor_mans_random);
          }  // Zwolnienie muteksu.
          poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
        }
      }
    };

    // Uruchomienie obu wątków.
    purger.start();
    adder.start();
    int result = 0;
    for (;;) {
      synchronized(dict) {  // Zajęcie muteksu.
        Integer value = dict.get("k1");
        if (value != null) {
          result += value;
        }
      }  // Zwolnienie muteksu.
    }
  }
}

