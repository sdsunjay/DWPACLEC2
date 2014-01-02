public class FatalException extends Exception {
   public FatalException() { super(); }
   public FatalException(String message) { super(message); }
   public FatalException(String message, Throwable cause) { super(message, cause); }
   public FatalException(Throwable cause) { super(cause); }
   public String toString()
   {
      return ("slave did not return cmdChar 'a' or 'r', so it may have died, so we quit");
   }
}
