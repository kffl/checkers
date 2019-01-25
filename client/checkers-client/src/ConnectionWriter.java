import java.io.OutputStream;
import java.io.PrintWriter;


public class ConnectionWriter {
	
	private PrintWriter writer;
	
	public ConnectionWriter(OutputStream os) {
		writer = new PrintWriter(os, true);
		
	}
	
	public void sendMove(int field1, int field2) {
		send("move;" + String.valueOf(field1) + ";" + String.valueOf(field2) + ";");
	}
	
	public void sendQuit() {
		send("quit;");
	}
	
	private void send(String msg) {
		System.out.println("Sending: " + msg);
		writer.println(msg);
	}
}
