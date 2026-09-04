// Auto-generated JNA bindings for MyLib

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Callback;
import java.util.Arrays;
import java.util.List;

public class MyLib {
    public static class NativeException extends RuntimeException {
        public NativeException(String message) { super(message); }
    }

    public static class CddFfiError extends Structure {
        public int code;
        public String message;
        public CddFfiError() { super(); }
        protected List<String> getFieldOrder() {
            return Arrays.asList("code", "message");
        }
        public static class ByReference extends CddFfiError implements Structure.ByReference {}
    }

    private interface MyLibLib extends Library {
        MyLibLib INSTANCE = (MyLibLib) Native.load("MyLib", MyLibLib.class);

    }

}
